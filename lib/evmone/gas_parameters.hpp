#pragma once
#include <array>
#include <optional>
#include <evmc/evmc.hpp>
#include <intx/intx.hpp>
#include <iostream>
#include <evmone/instructions_traits.hpp>
#include "storage_cost.hpp"

namespace evmone {

void revert_speculative_gas(evmc::Result& res);
void revert_speculative_gas(evmc_result& res);

struct gas_parameters {

    using storage_cost_t = std::array<StorageStoreCost, EVMC_STORAGE_MODIFIED_RESTORED + 1>;

    gas_parameters();
    gas_parameters(uint64_t txnewaccount, uint64_t newaccount, uint64_t txcreate, uint64_t codedeposit, uint64_t sset);
    gas_parameters(const evmc_gas_parameters& params);

    static gas_parameters apply_discount_factor(const intx::uint256& factor_num, const intx::uint256& factor_den, const evmone::gas_parameters& g) {
        gas_parameters out;
        out.values_.G_txnewaccount = static_cast<uint64_t>((factor_num*g.values_.G_txnewaccount)/factor_den);
        out.values_.G_newaccount   = static_cast<uint64_t>((factor_num*g.values_.G_newaccount)/factor_den);
        out.values_.G_txcreate     = static_cast<uint64_t>((factor_num*g.values_.G_txcreate)/factor_den);
        out.values_.G_codedeposit  = static_cast<uint64_t>((factor_num*g.values_.G_codedeposit)/factor_den);
        out.values_.G_sset         = static_cast<uint64_t>((factor_num*g.values_.G_sset)/factor_den);
        return out;
    }
    
    const storage_cost_t& get_storage_cost(uint64_t version);
    evmc_gas_parameters values_;
private:
    storage_cost_t generate_storage_cost_table(uint64_t version);
    std::optional<storage_cost_t> storage_cost;
};

} //namespace evmone
