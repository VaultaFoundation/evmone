#pragma once
#include <cstdint>
#include <algorithm>
#include <functional>
#include <evmc/evmc.hpp>

// namespace {
//     inline bool operator==(const evmc_uint256be& lhs, const evmc_uint256be& rhs) {
//         return std::memcmp(lhs.bytes, rhs.bytes, sizeof(lhs.bytes)) == 0;
//     }
// }

namespace evmone { namespace eosevm {

using filter_function = std::function<bool(const evmc_message&)>;

struct filtered_message {
  evmc::address   sender;
  evmc::address   receiver;
  evmc::bytes32   value;
  evmc::bytes     data;

  bool operator==(const filtered_message& other) const {
    return sender == other.sender && receiver == other.receiver
      && value == other.value && data == other.data;
  }
};

} }
