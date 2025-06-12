// evmone: Fast Ethereum Virtual Machine implementation
// Copyright 2022 The evmone Authors.
// SPDX-License-Identifier: Apache-2.0
#include "refund.hpp"

namespace evmone { namespace eosevm {

refund_result refund(const evmc_revision rev, const uint64_t version,
    const evmc::Result& vm_res, const bool is_contract_creation,
    const uint64_t gas_limit, const evmone::gas_parameters& scaled_gas_params, const intx::uint256& price,
    const evmone::eosevm::gas_prices& gas_prices, const intx::uint256& inclusion_price) {

    refund_result res;
    uint64_t gas_left = static_cast<uint64_t>(vm_res.gas_left);
    if (version > 2 ) { // version 3+
        uint64_t storage_gas_consumed{static_cast<uint64_t>(vm_res.storage_gas_consumed)};
        if(is_contract_creation) {
            if( vm_res.status_code == EVMC_SUCCESS ) {
                storage_gas_consumed += scaled_gas_params.values_.G_txcreate; //correct storage gas consumed to account for initial G_txcreate storage gas
            } else {
                gas_left += scaled_gas_params.values_.G_txcreate;
            }
        }

        evmone::gas_state_t vm_res_gas_state(version, static_cast<int64_t>(vm_res.gas_refund),
                static_cast<int64_t>(storage_gas_consumed), static_cast<int64_t>(vm_res.storage_gas_refund), static_cast<int64_t>(vm_res.speculative_cpu_gas_consumed));

        gas_left += static_cast<uint64_t>(vm_res_gas_state.collapse());
        auto gas_used = gas_limit - gas_left;
        assert(vm_res_gas_state.cpu_gas_refund() == 0);
        const auto total_storage_gas_consumed = static_cast<uint64_t>(vm_res_gas_state.storage_gas_consumed());
        assert(gas_used > total_storage_gas_consumed);
        const auto total_cpu_gas_consumed = gas_used - total_storage_gas_consumed;

        refund_result_v3 res_v3;
        res_v3.cpu_gas_consumed = total_cpu_gas_consumed;
        res_v3.discounted_storage_gas_consumed = static_cast<uint64_t>(total_storage_gas_consumed);
        res_v3.inclusion_fee = intx::uint256(total_cpu_gas_consumed)*inclusion_price;
        res_v3.storage_fee = intx::uint256(total_storage_gas_consumed)*price;

        auto final_fee = price * gas_used;
        if(gas_prices.storage_price >= gas_prices.overhead_price) {
            intx::uint256 gas_refund = intx::uint256(total_cpu_gas_consumed);
            gas_refund *= intx::uint256(gas_prices.storage_price-gas_prices.overhead_price);
            if(price > 0) {
                gas_refund /= price;
            } else {
                gas_refund = 0;
            }

            //TODO: SILKWORM_ASSERT(gas_refund <= gas_used);
            assert(gas_refund <= gas_used);
            res_v3.gas_refund = static_cast<uint64_t>(gas_refund);
            gas_left += res_v3.gas_refund;
            assert(gas_limit >= gas_left);
            gas_used = gas_limit - gas_left;
            //TODO: SILKWORM_ASSERT(gas_used >= total_storage_gas_consumed);
            assert(gas_used >= total_storage_gas_consumed);
            final_fee = price * gas_used;

            assert(final_fee >= res_v3.storage_fee);
            const auto overhead_and_inclusion_fee = final_fee - res_v3.storage_fee;
            if( overhead_and_inclusion_fee >= res_v3.inclusion_fee ) {
                res_v3.overhead_fee = overhead_and_inclusion_fee - res_v3.inclusion_fee;
            } else {
                res_v3.inclusion_fee = overhead_and_inclusion_fee;
                res_v3.overhead_fee = 0;
            }
        } else {
            res_v3.overhead_fee = final_fee - res_v3.inclusion_fee - res_v3.storage_fee;
        }

        assert(final_fee == res_v3.inclusion_fee + res_v3.storage_fee + res_v3.overhead_fee);
        res_v3.final_fee = final_fee;
        res_v3.gas_used = gas_used;
        res_v3.gas_left = gas_left;
        res = res_v3;

    } else if (version > 1) { // version 2
        gas_left += vm_res.gas_refund;
        if( gas_left > gas_limit - kGTransaction ) {
            gas_left = gas_limit - kGTransaction;
        }
        res = refund_result_v0{ .gas_left = gas_left, .gas_used = gas_limit - gas_left, .gas_refund = static_cast<uint64_t>(vm_res.gas_refund)};
    } else { //version 0 and 1
        const uint64_t max_refund_quotient{rev >= EVMC_LONDON ? kMaxRefundQuotientLondon
                                                              : kMaxRefundQuotientFrontier};
        const uint64_t max_refund{(gas_limit - gas_left) / max_refund_quotient};
        uint64_t refund = std::min(static_cast<uint64_t>(vm_res.gas_refund), max_refund);
        gas_left += refund;
        res = refund_result_v0{ .gas_left = gas_left, .gas_used = gas_limit - gas_left, .gas_refund = refund };
    }

    return res;
}

} }
