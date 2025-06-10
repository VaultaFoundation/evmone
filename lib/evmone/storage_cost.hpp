#pragma once
#include <cstdint>

namespace evmone {

struct StorageStoreCost
{
    int64_t gas_cost;
    int64_t gas_refund;
};

} //namespace evmone