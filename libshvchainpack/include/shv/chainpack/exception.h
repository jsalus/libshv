#pragma once

#include <shv/chainpack/shvchainpackglobal.h>

#include <shv/chainpack/rpcvalue.h>

#include <stdexcept>
#include <string>

namespace shv::chainpack {

class SHVCHAINPACK_DECL_EXPORT Exception : public std::runtime_error
{
	using Super = std::runtime_error;
public:
	static constexpr bool Throw = true;
public:
	Exception(const std::string& _msg, const std::string& _where = std::string());
	Exception(const std::string& _msg, const std::string& _where, const char *_log_topic);
	Exception(const std::string& _msg, const shv::chainpack::RpcValue& _data, const std::string& _where, const char *_log_topic);
	~Exception() override = default;
public:
	std::string message() const;
	std::string where() const;
	shv::chainpack::RpcValue data() const;
	static void setAbortOnException(bool on);
	static bool isAbortOnException();
protected:
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
	static bool s_abortOnException;
	std::string m_msg;
	shv::chainpack::RpcValue m_data;
	std::string m_where;
};

}

#define SHVCHP_EXCEPTION(e) throw shv::chainpack::Exception(e, std::string(__FILE__) + ":" + std::to_string(__LINE__))
#define SHVCHP_EXCEPTION_V(msg, topic) throw shv::core::Exception(msg, std::string(__FILE__) + ":" + std::to_string(__LINE__), topic)
