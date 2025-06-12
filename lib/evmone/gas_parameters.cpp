#include "gas_parameters.hpp"
#include <iostream>

namespace evmone {

void revert_speculative_gas(evmc::Result& res) {
    res.gas_left += res.storage_gas_consumed;
    res.gas_left += res.speculative_cpu_gas_consumed;
    res.gas_refund = 0;
    res.storage_gas_consumed = 0;
    res.storage_gas_refund = 0;
    res.speculative_cpu_gas_consumed = 0;
}

void revert_speculative_gas(evmc_result& res) {
    res.gas_left += res.storage_gas_consumed;
    res.gas_left += res.speculative_cpu_gas_consumed;
    res.gas_refund = 0;
    res.storage_gas_consumed = 0;
    res.storage_gas_refund = 0;
    res.speculative_cpu_gas_consumed = 0;
}

const gas_parameters::storage_cost_t& gas_parameters::get_storage_cost(uint64_t version) {
    if(!storage_cost.has_value()) {
        storage_cost = generate_storage_cost_table(version);
    }
    return *storage_cost;
}

gas_parameters::gas_parameters() {
    values_.G_txnewaccount = 0;
    values_.G_newaccount = 25000;
    values_.G_txcreate = 32000;
    values_.G_codedeposit = 200;
    values_.G_sset = 20000;
}

gas_parameters::gas_parameters(uint64_t txnewaccount, uint64_t newaccount, uint64_t txcreate, uint64_t codedeposit, uint64_t sset) {
    values_.G_txnewaccount = txnewaccount;
    values_.G_newaccount = newaccount;
    values_.G_txcreate = txcreate;
    values_.G_codedeposit = codedeposit;
    values_.G_sset = sset;
}

gas_parameters::gas_parameters(const evmc_gas_parameters& values) {
    values_ = values;
}

gas_parameters::storage_cost_t gas_parameters::generate_storage_cost_table(uint64_t version) {
    const int64_t warm_access = instr::warm_storage_read_cost;
    const int64_t set         = static_cast<int64_t>(values_.G_sset);
    const int64_t reset       = 5000 - instr::cold_sload_cost;
    const int64_t clear       = 4800;

    gas_parameters::storage_cost_t st;
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

} //namespace evmone
