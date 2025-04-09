#include <cstdint>
#include <algorithm>

#include "gas_prices.hpp"

namespace evmone { namespace eosevm {

uint64_t gas_prices::get_base_price()const {
    return std::max(overhead_price, storage_price);
}

bool gas_prices::is_zero()const {
    return overhead_price == 0 && storage_price == 0;
}

bool operator==(const gas_prices& lhs, const gas_prices& rhs) {
    return lhs.overhead_price == rhs.overhead_price && lhs.storage_price == rhs.storage_price;
}

} }
