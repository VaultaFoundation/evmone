// evmone: Fast Ethereum Virtual Machine implementation
// Copyright 2022 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <variant>
#include <intx/intx.hpp>
#include <evmc/evmc.hpp>
#include <evmone/execution_state.hpp>
#include <evmone/gas_prices.hpp>

namespace evmone { namespace eosevm {

// EIP-3529: Reduction in refunds
inline constexpr uint64_t kMaxRefundQuotientFrontier{2};
inline constexpr uint64_t kMaxRefundQuotientLondon{5};

inline constexpr uint64_t kGTransaction{21'000};

struct refund_result_v0 {
    uint64_t gas_left{0};
    uint64_t gas_used{0};
    uint64_t gas_refund{0};
};

struct refund_result_v3 {
    uint64_t gas_left{0};
    uint64_t gas_used{0};
    uint64_t gas_refund{0};
    uint64_t discounted_storage_gas_consumed{0};
    uint64_t cpu_gas_consumed{0};
    intx::uint256 overhead_fee;  //approx. => cpu_gas_consumed * overhead_price
    intx::uint256 inclusion_fee; //approx. => cpu_gas_consumed * inclusion_price
    intx::uint256 storage_fee;   //exactly => discounted_storage_gas_consumed * effective_price
    intx::uint256 final_fee;
};

using refund_result = std::variant<refund_result_v0, refund_result_v3>;

refund_result refund(const evmc_revision rev, const uint64_t version,
    const evmc::Result& vm_res, const bool is_contract_creation,
    const uint64_t gas_limit, const evmone::gas_parameters& scaled_gas_params,
    const intx::uint256& price, const evmone::eosevm::gas_prices& gas_prices,
    const intx::uint256& inclusion_price);

} }
