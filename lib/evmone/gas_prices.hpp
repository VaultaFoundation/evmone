#pragma once
#include <cstdint>
#include <algorithm>

namespace evmone { namespace eosevm {

struct gas_prices {
  uint64_t overhead_price{0};
  uint64_t storage_price{0};

  uint64_t get_base_price()const;
  bool is_zero()const;
  
  friend bool operator==(const gas_prices&, const gas_prices&);
};


} }
