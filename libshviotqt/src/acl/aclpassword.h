#pragma once

#include "../shviotqtglobal.h"

#include <string>

namespace shv { namespace chainpack { class RpcValue; } }

namespace shv {
namespace iotqt {
namespace acl {

struct SHVIOTQT_DECL_EXPORT AclPassword
{
	enum class Format {Invalid, Plain, Sha1};

	std::string password;
	Format format = Format::Invalid;

	AclPassword() {}
	AclPassword(std::string password_, Format format_)
		: password(std::move(password_))
		, format(format_)
	{}

	bool isValid() const {return format != Format::Invalid;}

	shv::chainpack::RpcValue toRpcValue() const;
	static AclPassword fromRpcValue(const shv::chainpack::RpcValue &v);

	static const char *formatToString(Format f);
	static Format formatFromString(const std::string &s);
};

} // namespace acl
} // namespace iotqt
} // namespace shv

