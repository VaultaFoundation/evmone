#include "gas_parameters.hpp"
#include <iostream>

namespace evmone {

void revert_speculative_gas(evmc::Result& res) {
    std::cout << "revert_speculative_gas1A: " << res.gas_left << std::endl;
    res.gas_left += res.storage_gas_consumed;
    res.gas_left += res.speculative_cpu_gas_consumed;
    res.gas_refund = 0;
    res.storage_gas_consumed = 0;
    res.storage_gas_refund = 0;
    res.speculative_cpu_gas_consumed = 0;
    std::cout << "revert_speculative_gas1B: " << res.gas_left << std::endl;
}

void revert_speculative_gas(evmc_result& res) {
    std::cout << "revert_speculative_gas2A: " << res.gas_left << std::endl;
    res.gas_left += res.storage_gas_consumed;
    res.gas_left += res.speculative_cpu_gas_consumed;
    res.gas_refund = 0;
    res.storage_gas_consumed = 0;
    res.storage_gas_refund = 0;
    res.speculative_cpu_gas_consumed = 0;
    std::cout << "revert_speculative_gas2B: " << res.gas_left << std::endl;
}

} //namespace evmone
