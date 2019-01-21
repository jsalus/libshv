#pragma once

#include "../shviotqtglobal.h"

//#include <shv/chainpack/rpc.h>
#include <shv/chainpack/rpcvalue.h>
#include <shv/core/stringview.h>

#include <QObject>
#include <QMetaProperty>

namespace shv { namespace chainpack { class MetaMethod; class RpcValue; class RpcMessage; class RpcRequest; }}
//namespace shv { namespace core { class StringView; }}

namespace shv {
namespace iotqt {
namespace node {

class ShvRootNode;

class SHVIOTQT_DECL_EXPORT ShvNode : public QObject
{
	Q_OBJECT
public:
	using String = std::string;
	using StringList = std::vector<String>;
	using StringView = shv::core::StringView;
	using StringViewList = shv::core::StringViewList;
public:
	explicit ShvNode(ShvNode *parent = nullptr);
	explicit ShvNode(const std::string &node_id, ShvNode *parent = nullptr);
	~ShvNode() override;

	//size_t childNodeCount() const {return propertyNames().size();}
	ShvNode* parentNode() const;
	QList<ShvNode*> ownChildren() const;
	virtual ShvNode* childNode(const String &name, bool throw_exc = true) const;
	//ShvNode* childNode(const core::StringView &name) const;
	virtual void setParentNode(ShvNode *parent);
	virtual String nodeId() const {return m_nodeId;}
	void setNodeId(String &&n);
	void setNodeId(const String &n);

	String shvPath() const;
	static StringViewList splitShvPath(const std::string &shv_path) { return StringView{shv_path}.split('/', '"'); }

	ShvRootNode* rootNode();
	virtual void emitSendRpcMesage(const shv::chainpack::RpcMessage &msg);

	void setSortedChildren(bool b) {m_isSortedChildren = b;}

	void deleteIfEmptyWithParents();

	virtual bool isRootNode() const {return false;}

	virtual void handleRawRpcRequest(chainpack::RpcValue::MetaData &&meta, std::string &&data);
	virtual void handleRpcRequest(const chainpack::RpcRequest &rq);
	virtual chainpack::RpcValue processRpcRequest(const shv::chainpack::RpcRequest &rq);

	virtual shv::chainpack::RpcValue dir(const StringViewList &shv_path, const shv::chainpack::RpcValue &methods_params);
	//virtual StringList methodNames(const StringViewList &shv_path);

	virtual shv::chainpack::RpcValue ls(const StringViewList &shv_path, const shv::chainpack::RpcValue &methods_params);
	// returns null if does not know
	virtual chainpack::RpcValue hasChildren(const StringViewList &shv_path);
	//chainpack::RpcValue hasChildren() {return hasChildren(StringViewList());}
	virtual shv::chainpack::RpcValue lsAttributes(const StringViewList &shv_path, unsigned attributes);

	virtual int grantToAccessLevel(const char *grant_name) const;
public:
	virtual size_t methodCount(const StringViewList &shv_path);
	virtual const shv::chainpack::MetaMethod* metaMethod(const StringViewList &shv_path, size_t ix);
	virtual const shv::chainpack::MetaMethod* metaMethod(const StringViewList &shv_path, const std::string &name);

	virtual StringList childNames(const StringViewList &shv_path);
	StringList childNames() {return childNames(StringViewList());}

	virtual shv::chainpack::RpcValue callMethod(const chainpack::RpcRequest &rq);
	virtual shv::chainpack::RpcValue callMethod(const StringViewList &shv_path, const std::string &method, const shv::chainpack::RpcValue &params);
private:
	String m_nodeId;
	bool m_isSortedChildren = true;
};

class SHVIOTQT_DECL_EXPORT ShvRootNode : public ShvNode
{
	Q_OBJECT
	using Super = ShvNode;
public:
	explicit ShvRootNode(QObject *parent) : Super() {setParent(parent);}

	bool isRootNode() const override {return true;}

	Q_SIGNAL void sendRpcMesage(const shv::chainpack::RpcMessage &msg);
	void emitSendRpcMesage(const shv::chainpack::RpcMessage &msg) override;
};

class SHVIOTQT_DECL_EXPORT MethodsTableNode : public shv::iotqt::node::ShvNode
{
	using Super = shv::iotqt::node::ShvNode;
public:
	//explicit MethodsTableNode(const std::string &node_id, shv::iotqt::node::ShvNode *parent = nullptr)
	//	: Super(node_id, parent) {}
	explicit MethodsTableNode(const std::string &node_id, const std::vector<shv::chainpack::MetaMethod> &methods, shv::iotqt::node::ShvNode *parent = nullptr)
		: Super(node_id, parent), m_methods(methods) {}

	size_t methodCount(const StringViewList &shv_path) override;
	const shv::chainpack::MetaMethod* metaMethod(const StringViewList &shv_path, size_t ix) override;
protected:
	const std::vector<shv::chainpack::MetaMethod> &m_methods;
};


class SHVIOTQT_DECL_EXPORT RpcValueMapNode : public shv::iotqt::node::ShvNode
{
	using Super = shv::iotqt::node::ShvNode;
public:
	static const char *M_LOAD;
	static const char *M_SAVE;
	static const char *M_COMMIT;
public:
	RpcValueMapNode(const std::string &node_id, shv::iotqt::node::ShvNode *parent = nullptr);
	RpcValueMapNode(const std::string &node_id, const shv::chainpack::RpcValue &values, shv::iotqt::node::ShvNode *parent = nullptr);
	//~RpcValueMapNode() override;

	size_t methodCount(const StringViewList &shv_path) override;
	const shv::chainpack::MetaMethod* metaMethod(const StringViewList &shv_path, size_t ix) override;

	StringList childNames(const ShvNode::StringViewList &shv_path) override;
	shv::chainpack::RpcValue hasChildren(const StringViewList &shv_path) override;

	shv::chainpack::RpcValue callMethod(const StringViewList &shv_path, const std::string &method, const shv::chainpack::RpcValue &params) override;

	void clearValuesCache() {m_valuesLoaded = false;}
protected:
	virtual shv::chainpack::RpcValue loadValues();
	virtual bool saveValues(const shv::chainpack::RpcValue &vals);
	const shv::chainpack::RpcValue &values();
	virtual shv::chainpack::RpcValue valueOnPath(const StringViewList &shv_path);
	void setValueOnPath(const StringViewList &shv_path, const shv::chainpack::RpcValue &val);
	bool isDir(const StringViewList &shv_path);
protected:
	bool m_valuesLoaded = false;
	shv::chainpack::RpcValue m_values;
};

/// Deprecated
class SHVIOTQT_DECL_EXPORT ObjectPropertyProxyShvNode : public shv::iotqt::node::ShvNode
{
	using Super = shv::iotqt::node::ShvNode;
public:
	//explicit MethodsTableNode(const std::string &node_id, shv::iotqt::node::ShvNode *parent = nullptr)
	//	: Super(node_id, parent) {}
	explicit ObjectPropertyProxyShvNode(const char *property_name, QObject *property_obj, shv::iotqt::node::ShvNode *parent = nullptr);

	size_t methodCount(const StringViewList &shv_path) override;
	const shv::chainpack::MetaMethod* metaMethod(const StringViewList &shv_path, size_t ix) override;

	shv::chainpack::RpcValue callMethod(const shv::chainpack::RpcRequest &rq) override {return  Super::callMethod(rq);}
	shv::chainpack::RpcValue callMethod(const StringViewList &shv_path, const std::string &method, const shv::chainpack::RpcValue &params) override;
protected:
	QMetaProperty m_metaProperty;
	QObject *m_propertyObj = nullptr;
};

class SHVIOTQT_DECL_EXPORT ValueProxyShvNode : public shv::iotqt::node::ShvNode
{
	using Super = shv::iotqt::node::ShvNode;
public:
	enum class Type {
		Invalid = 0,
		Read = 1,
		Write = 2,
		ReadWrite = 3,
		Signal = 4,
		ReadSignal = 5,
		WriteSignal = 6,
		ReadWriteSignal = 7,
	};
	class SHVIOTQT_DECL_EXPORT Handle
	{
	public:
		Handle() {}
		virtual ~Handle();

		virtual shv::chainpack::RpcValue shvValue(int value_id) = 0;
		virtual bool setShvValue(int value_id, const shv::chainpack::RpcValue &val) = 0;
		//void callShvValueChanged(const char *name, const shv::chainpack::RpcValue &val);
	private:
		//PropertyShvNode *m_propertyNode;
	};
public:
	explicit ValueProxyShvNode(const std::string &node_id, int value_id, Type type, Handle *handled_obj, shv::iotqt::node::ShvNode *parent = nullptr);

	void addMetaMethod(shv::chainpack::MetaMethod &&mm);

	size_t methodCount(const StringViewList &shv_path) override;
	const shv::chainpack::MetaMethod* metaMethod(const StringViewList &shv_path, size_t ix) override;

	shv::chainpack::RpcValue callMethod(const shv::chainpack::RpcRequest &rq) override {return  Super::callMethod(rq);}
	shv::chainpack::RpcValue callMethod(const StringViewList &shv_path, const std::string &method, const shv::chainpack::RpcValue &params) override;
protected:
	bool isWriteable() {return static_cast<int>(m_type) & static_cast<int>(Type::Write);}
	bool isReadable() {return static_cast<int>(m_type) & static_cast<int>(Type::Read);}
	bool isSignal() {return static_cast<int>(m_type) & static_cast<int>(Type::Signal);}

	void onShvValueChanged(int value_id, shv::chainpack::RpcValue val);
protected:
	int m_valueId;
	Type m_type;
	Handle *m_handledObject = nullptr;
	std::vector<shv::chainpack::MetaMethod> m_extraMetaMethods;
};

}}}
