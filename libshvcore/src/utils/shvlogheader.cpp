#include "shvlogheader.h"

#include "../stringview.h"
#include "../exception.h"

#include <algorithm>

namespace shv {
namespace core {
namespace utils {

const std::string ShvLogHeader::EMPTY_PREFIX_KEY = "";

const char *ShvLogHeader::Column::name(ShvLogHeader::Column::Enum e)
{
	switch (e) {
	case Column::Enum::Timestamp: return "timestamp";
	//case Column::Enum::UpTime: return "upTime";
	case Column::Enum::Path: return "path";
	case Column::Enum::Value: return "value";
	case Column::Enum::ShortTime: return "shortTime";
	case Column::Enum::Domain: return "domain";
	case Column::Enum::SampleType: return "sampleType";
	}
	return "invalid";
}

ShvLogHeader ShvLogHeader::fromMetaData(const chainpack::RpcValue::MetaData &md)
{
	ShvLogHeader ret;
	const chainpack::RpcValue::Map &device = md.value("device").toMap();
	ret.setDeviceId(device.value("id").toString());
	ret.setDeviceType(device.value("type").toString());
	ret.setLogVersion(md.value("logVersion").toInt());
	ShvGetLogParams params = ShvGetLogParams::fromRpcValue(md.value("logParams"));
	ret.setLogParams(std::move(params));
	ret.setRecordCount(md.value("recordCount").toInt());
	ret.setRecordCountLimit(md.value("recordCountLimit").toInt());
	ret.setRecordCountLimitHit(md.value("recordCountLimitHit").toBool());
	ret.setWithSnapShot(md.value(ShvGetLogParams::KEY_WITH_SNAPSHOT).toBool());
	ret.setWithPathsDict(md.value(ShvGetLogParams::KEY_WITH_PATHS_DICT, true).toBool());
	ret.setFields(md.value("fields").toList());
	ret.setPathDict(md.value("pathsDict").toIMap());
	{
		const chainpack::RpcValue::Map &m = md.value("sources").toMap();
		for(const auto &kv : m) {
			ret.m_sources[kv.first] = ShvLogTypeInfo::fromRpcValue(kv.second);
		}
		chainpack::RpcValue ti = md.value("typeInfo");
		if(ti.isMap())
			ret.m_sources[EMPTY_PREFIX_KEY] = ShvLogTypeInfo::fromRpcValue(ti);
	}
	ret.setDateTime(md.value("dateTime"));
	ret.setSince(md.value("since"));
	ret.setUntil(md.value("until"));
	return ret;
}

chainpack::RpcValue::MetaData ShvLogHeader::toMetaData() const
{
	chainpack::RpcValue::MetaData md;
	chainpack::RpcValue::Map device;
	if(!deviceId().empty())
		device["id"] = deviceId();
	if(!deviceType().empty())
		device["type"] = deviceType();
	if(!device.empty())
		md.setValue("device", device);
	md.setValue("logVersion", logVersion());
	md.setValue("logParams", logParams().toRpcValue());
	md.setValue("recordCount", recordCount());
	md.setValue("recordCountLimit", recordCountLimit());
	md.setValue("recordCountLimitHit", recordCountLimitHit());
	md.setValue(ShvGetLogParams::KEY_WITH_SNAPSHOT, withSnapShot());
	md.setValue(ShvGetLogParams::KEY_WITH_PATHS_DICT, withPathsDict());
	if(!fields().empty())
		md.setValue("fields", fields());
	if(!pathDict().empty())
		md.setValue("pathsDict", pathDict());
	chainpack::RpcValue::Map srcs;
	for(const auto &kv : m_sources) {
		const std::string &path_prefix = kv.first;
		if(path_prefix.empty())
			md.setValue("typeInfo", kv.second.toRpcValue());
		else
			srcs[path_prefix] = kv.second.toRpcValue();
	}
	if(!srcs.empty())
		md.setValue("sources", std::move(srcs));
	md.setValue("dateTime", dateTime());
	md.setValue("since", since());
	md.setValue("until", until());
	return md;
}

const ShvLogTypeInfo &ShvLogHeader::typeInfo(const std::string &path_prefix) const
{
	auto it = m_sources.find(path_prefix);
	if(it == m_sources.end()) {
		static const ShvLogTypeInfo ti;
		return ti;
	}
	return m_sources.at(path_prefix);
}

void ShvLogHeader::setTypeInfo(ShvLogTypeInfo &&ti)
{
	m_sources[EMPTY_PREFIX_KEY] = std::move(ti);
}

void ShvLogHeader::setTypeInfo(const ShvLogTypeInfo &ti)
{
	m_sources[EMPTY_PREFIX_KEY] = ti;
}

void ShvLogHeader::setTypeInfo(const std::string &path_prefix, ShvLogTypeInfo &&ti)
{
	m_sources[path_prefix] = std::move(ti);
}
/*
void ShvLogHeader::clearTypeInfo()
{
	m_sources.clear();
	m_pathsTypeDescrValid = false;
}
*/
/*
std::map<std::string, ShvLogTypeDescr> ShvLogHeader::pathsTypeDescr() const
{
	std::map<std::string, ShvLogTypeDescr> ret;
	for(const auto &kv : m_sources) {
		std::string prefix = kv.first;
		const ShvLogTypeInfo &ti = kv.second;
		for(const auto &kv2 : ti.paths) {
			const std::string &type_name = kv2.second.typeName;
			auto it = ti.types.find(type_name);
			if(it != ti.types.end()) {
				std::string path = kv2.first;
				if(prefix != EMPTY_PREFIX_KEY)
					path = prefix  + '/' + path;
				ret[path] = it->second;
			}
		}
	}
	return ret;
}
 */
#if 0
chainpack::RpcValue ShvLogHeader::valueOnPath(const std::string &path) const
{
	return valueOnPath(StringView(path).split('/'));
}

chainpack::RpcValue ShvLogHeader::valueOnPath(const StringViewList &path) const
{
	chainpack::RpcValue ret;
	if(path.empty())
		return ret;
	ret = value(path.value(0).toString());
	return valueOnPath_helper(ret, path);
}

chainpack::RpcValue ShvLogHeader::valueOnPath_helper(const chainpack::RpcValue &parent, const StringViewList &path) const
{
	chainpack::RpcValue ret = parent;
	for (size_t i = 1; i < path.size(); ++i) {
		const auto &sv = path[i];
		ret = ret.at(sv.toString());
	}
	return ret;
}

void ShvLogHeader::setValueOnPath(const std::string &path, const chainpack::RpcValue &val)
{
	setValueOnPath(StringView(path).split('/'), val);
}

void ShvLogHeader::setValueOnPath(const StringViewList &path, const chainpack::RpcValue &val)
{
	if(path.empty())
		SHV_EXCEPTION("Empty path");
	std::string path0 = path.value(0).toString();
	if(path.size() == 1) {
		setValue(path0, val);
		return;
	}
	shv::chainpack::RpcValue v = value(path0);
	if(!v.isMap()) {
		v = chainpack::RpcValue::Map();
		setValue(path0, v);
	}
	setValueOnPath_helper(v, path, val);
}

void ShvLogHeader::setValueOnPath_helper(const chainpack::RpcValue &parent, const StringViewList &path, const chainpack::RpcValue &val)
{
	chainpack::RpcValue v = parent;
	for (size_t i = 1; i < path.size()-1; ++i) {
		const std::string dir = path.at(i).toString();
		if(!v.isMap())
			SHV_EXCEPTION("Internal error");
		shv::chainpack::RpcValue v1 = v.at(dir);
		if(!v1.isMap()) {
			v1 = chainpack::RpcValue::Map();
			v.set(dir, v1);
		}
		v = v1;
	}
	v.set(path.at(path.size() - 1).toString(), val);
}

std::string ShvLogHeader::deviceId() const
{
	return valueOnPath("device/id").toString();
}

void ShvLogHeader::setDeviceId(const std::string &device_id)
{
	setValueOnPath("device/id", device_id);
}

std::string ShvLogHeader::deviceType() const
{
	return valueOnPath("device/type").toString();
}

void ShvLogHeader::setDeviceType(const std::string &device_type)
{
	setValueOnPath("device/type", device_type);
}
#endif



} // namespace utils
} // namespace core
} // namespace shv
