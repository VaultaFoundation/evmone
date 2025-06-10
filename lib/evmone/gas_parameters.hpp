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

struct gas_parameters : evmc_gas_parameters {

    using storage_cost_t = std::array<StorageStoreCost, EVMC_STORAGE_MODIFIED_RESTORED + 1>;

    gas_parameters() : evmc_gas_parameters{
        .G_txnewaccount = 0,
        .G_newaccount = 25000,
        .G_txcreate = 32000,
        .G_codedeposit = 200,
        .G_sset = 20000
    } {}

    gas_parameters(const evmc_gas_parameters& params) : evmc_gas_parameters{
        .G_txnewaccount = params.G_txnewaccount,
        .G_newaccount = params.G_newaccount,
        .G_txcreate = params.G_txcreate,
        .G_codedeposit = params.G_codedeposit,
        .G_sset = params.G_sset
    } {}

    gas_parameters(uint64_t txnewaccount, uint64_t newaccount, uint64_t txcreate, uint64_t codedeposit, uint64_t sset) : evmc_gas_parameters{
        .G_txnewaccount = txnewaccount,
        .G_newaccount = newaccount,
        .G_txcreate = txcreate,
        .G_codedeposit = codedeposit,
        .G_sset = sset
    } {}

    static gas_parameters apply_discount_factor(const intx::uint256& factor_num, const intx::uint256& factor_den, const evmone::gas_parameters& g) {
        std::cout << "apply_discount_factor:"
                  << intx::to_string(factor_num) << "/" 
                  << intx::to_string(factor_den) << ","
                  << "G_txcreate: " << g.G_txcreate << std::endl;

        gas_parameters out;
        out.G_txnewaccount = static_cast<uint64_t>((factor_num*g.G_txnewaccount)/factor_den);
        out.G_newaccount   = static_cast<uint64_t>((factor_num*g.G_newaccount)/factor_den);
        out.G_txcreate     = static_cast<uint64_t>((factor_num*g.G_txcreate)/factor_den);
        out.G_codedeposit  = static_cast<uint64_t>((factor_num*g.G_codedeposit)/factor_den);
        out.G_sset         = static_cast<uint64_t>((factor_num*g.G_sset)/factor_den);
        return out;
    }

    const storage_cost_t& get_storage_cost(uint64_t version) {
        if(!storage_cost.has_value()) {
            storage_cost = generate_storage_cost_table(version);
        }
        return *storage_cost;
    }

private:
    storage_cost_t generate_storage_cost_table(uint64_t version) {
        const int64_t warm_access = instr::warm_storage_read_cost;
        const int64_t set         = static_cast<int64_t>(G_sset);
        const int64_t reset       = 5000 - instr::cold_sload_cost;
        const int64_t clear       = 4800;

        storage_cost_t st;
        if( version >= 3) {
            int64_t cpu_gas_to_change_slot  = reset - warm_access; //cpu cost of adding or removing or mutating a slot in the db
            int64_t storage_gas_to_add_slot = set; //storage cost of adding a new slot into the db

            st[EVMC_STORAGE_ASSIGNED]          = {                      0,                        0};
            st[EVMC_STORAGE_ADDED]             = { cpu_gas_to_change_slot,  storage_gas_to_add_slot};
            st[EVMC_STORAGE_DELETED]           = { cpu_gas_to_change_slot, -storage_gas_to_add_slot};
            st[EVMC_STORAGE_MODIFIED]          = { cpu_gas_to_change_slot,                        0};
            st[EVMC_STORAGE_DELETED_ADDED]     = {                      0,  storage_gas_to_add_slot};
            st[EVMC_STORAGE_MODIFIED_DELETED]  = {                      0, -storage_gas_to_add_slot};
            st[EVMC_STORAGE_DELETED_RESTORED]  = {-cpu_gas_to_change_slot,  storage_gas_to_add_slot};
            st[EVMC_STORAGE_ADDED_DELETED]     = {-cpu_gas_to_change_slot, -storage_gas_to_add_slot};
            st[EVMC_STORAGE_MODIFIED_RESTORED] = {-cpu_gas_to_change_slot,                        0};
        } else {
            st[EVMC_STORAGE_ASSIGNED]          = {warm_access, 0};
            st[EVMC_STORAGE_ADDED]             = {set, 0};
            st[EVMC_STORAGE_DELETED]           = {reset, clear};
            st[EVMC_STORAGE_MODIFIED]          = {reset, 0};
            st[EVMC_STORAGE_DELETED_ADDED]     = {warm_access,-clear};
            st[EVMC_STORAGE_MODIFIED_DELETED]  = {warm_access, clear};
            st[EVMC_STORAGE_DELETED_RESTORED]  = {warm_access, reset - warm_access - clear};
            st[EVMC_STORAGE_ADDED_DELETED]     = {warm_access, set - warm_access};
            st[EVMC_STORAGE_MODIFIED_RESTORED] = {warm_access, reset - warm_access};
        }
        return st;
    }

    std::optional<storage_cost_t> storage_cost;
};

} //namespace evmone