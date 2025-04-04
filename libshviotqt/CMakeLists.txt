qt_add_library(libshviotqt
	src/acl/aclmountdef.cpp
	src/acl/aclpassword.cpp
	src/acl/aclrole.cpp
	src/acl/aclroleaccessrules.cpp
	src/acl/acluser.cpp
	src/node/filenode.cpp
	src/node/localfsnode.cpp
	src/node/propertynode.cpp
	src/node/shvnode.cpp
	src/node/shvnodetree.cpp
	src/rpc/clientappclioptions.cpp
	src/rpc/clientconnection.cpp
	src/rpc/deviceappclioptions.cpp
	src/rpc/deviceconnection.cpp
	src/rpc/rpccall.cpp
	src/rpc/socket.cpp
	src/rpc/localsocket.cpp
	src/rpc/socketrpcconnection.cpp
	src/utils.cpp
	src/utils/network.cpp

	include/shv/iotqt/acl/aclmountdef.h
	include/shv/iotqt/acl/acluser.h
	include/shv/iotqt/acl/aclrole.h
	include/shv/iotqt/acl/aclpassword.h
	include/shv/iotqt/acl/aclroleaccessrules.h
	include/shv/iotqt/node/shvnodetree.h
	include/shv/iotqt/node/shvnode.h
	include/shv/iotqt/node/localfsnode.h
	include/shv/iotqt/node/filenode.h
	include/shv/iotqt/node/propertynode.h
	include/shv/iotqt/utils/network.h
	include/shv/iotqt/rpc/deviceconnection.h
	include/shv/iotqt/rpc/rpccall.h
	include/shv/iotqt/rpc/socket.h
	include/shv/iotqt/rpc/localsocket.h
	include/shv/iotqt/rpc/clientconnection.h
	include/shv/iotqt/rpc/clientappclioptions.h
	include/shv/iotqt/rpc/deviceappclioptions.h
	include/shv/iotqt/rpc/rpccall.h
	include/shv/iotqt/rpc/socketrpcconnection.h
	include/shv/iotqt/utils.h
	)
add_library(libshv::libshviotqt ALIAS libshviotqt)

target_link_libraries(libshviotqt PUBLIC Qt::Network libshvcoreqt)

if(NOT EMSCRIPTEN)
	target_link_libraries(libshviotqt PUBLIC Qt::SerialPort)
	target_sources(libshviotqt PRIVATE
		include/shv/iotqt/rpc/serialportsocket.h
		src/rpc/serialportsocket.cpp
		)
endif()

target_include_directories(libshviotqt PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	)
target_compile_definitions(libshviotqt PRIVATE SHVIOTQT_BUILD_DLL)

if(LIBSHV_WITH_WEBSOCKETS)
	target_sources(libshviotqt PRIVATE
		include/shv/iotqt/rpc/websocket.h
		src/rpc/websocket.cpp
		)
	target_compile_definitions(libshviotqt PUBLIC WITH_SHV_WEBSOCKETS)
	target_link_libraries(libshviotqt PUBLIC Qt::WebSockets)
endif()

if(LIBSHV_WITH_OAUTH2_AZURE)
	target_compile_definitions(libshviotqt PUBLIC WITH_SHV_OAUTH2_AZURE)
	target_link_libraries(libshviotqt PUBLIC Qt::Gui Qt::NetworkAuth)
endif()

function(add_shviotqt_serialportsocket_test test_name)
	add_executable(test_iotqt_${test_name}
		tests/serialportsocket/test_${test_name}.cpp
		tests/serialportsocket/mockserialport.cpp
		tests/serialportsocket/mockrpcconnection.cpp
	)
	#target_compile_definitions(test_iotqt_${test_name} PRIVATE DEF_FILES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/files")
	target_link_libraries(test_iotqt_${test_name} libshviotqt doctest::doctest)
	add_test(NAME test_iotqt_${test_name} COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:test_iotqt_${test_name}>)
endfunction(add_shviotqt_serialportsocket_test)

function(add_shviotqt_test test_name)
	add_executable(
		test_iotqt_${test_name}
		tests/test_${test_name}.cpp
	)
	target_compile_definitions(test_iotqt_${test_name} PRIVATE DEF_FILES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/files")
	target_link_libraries(test_iotqt_${test_name} libshviotqt doctest::doctest)
	add_test(NAME test_iotqt_${test_name} COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:test_iotqt_${test_name}>)
endfunction(add_shviotqt_test)

if(BUILD_TESTING)
	add_shviotqt_test(frame_reader)

	add_shviotqt_serialportsocket_test(serialportsocket)
	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test_serialportsocket")
	target_compile_definitions(test_iotqt_serialportsocket PRIVATE TESTS_DIR="${CMAKE_CURRENT_BINARY_DIR}/test_serialportsocket")
endif()

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/shv" TYPE INCLUDE)

install(TARGETS libshviotqt EXPORT libshvConfig)
export(TARGETS libshviotqt NAMESPACE libshv:: FILE "${CMAKE_CURRENT_BINARY_DIR}/libshvConfig.cmake")
