#pragma once
#include <cstdint>
#include <intx/intx.hpp>

namespace evmone { namespace eosevm {

struct execution_result {
  uint64_t discounted_storage_gas_consumed{0};
  uint64_t cpu_gas_consumed{0};
  intx::uint256 overhead_fee;  //approx. => cpu_gas_consumed * overhead_price
  intx::uint256 inclusion_fee; //approx. => cpu_gas_consumed * inclusion_price
  intx::uint256 storage_fee;   //exactly => discounted_storage_gas_consumed * effective_price
  evmc_status_code status{EVMC_SUCCESS};
  evmc::bytes data;
};

} }
