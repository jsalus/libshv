#pragma once

#include "../shviotqtglobal.h"

#include "shv/chainpack/rpcvalue.h"

#include <string>
#include <vector>
#include <limits>

namespace shv {
namespace iotqt {
namespace acl {

struct SHVIOTQT_DECL_EXPORT AclRole
{
	static constexpr int INVALID_WEIGHT = std::numeric_limits<int>::min();
	int weight = INVALID_WEIGHT;
	std::vector<std::string> roles;
	shv::chainpack::RpcValue profile;

	AclRole() = default;
	AclRole(int w) : weight(w) {}
	AclRole(int w, std::vector<std::string> roles_) : weight(w), roles(std::move(roles_)) {}

	bool isValid() const {return weight != INVALID_WEIGHT;}
	shv::chainpack::RpcValue toRpcValue() const;
	static AclRole fromRpcValue(const shv::chainpack::RpcValue &v);
};

} // namespace acl
} // namespace iotqt
} // namespace shv

