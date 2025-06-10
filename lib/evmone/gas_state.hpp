#pragma once
#include <cstdint>
#include <cassert>

#include <evmc/evmc.hpp>

namespace evmone {

struct gas_state_t {
    explicit gas_state_t() : eos_evm_version_{0}, cpu_gas_refund_{0}, storage_gas_consumed_{0}, storage_gas_refund_{0}, speculative_cpu_gas_consumed_{0} {}

    explicit gas_state_t(uint64_t eos_evm_version, int64_t cpu_gas_refund, int64_t storage_gas_consumed, int64_t storage_gas_refund, int64_t speculative_cpu_gas_consumed) {
        reset(eos_evm_version, cpu_gas_refund, storage_gas_consumed, storage_gas_refund, speculative_cpu_gas_consumed);
    }

    void reset(uint64_t eos_evm_version, int64_t cpu_gas_refund, int64_t storage_gas_consumed, int64_t storage_gas_refund, int64_t speculative_cpu_gas_consumed) {
        eos_evm_version_ = eos_evm_version;
        cpu_gas_refund_ = cpu_gas_refund;
        storage_gas_consumed_ = storage_gas_consumed;
        storage_gas_refund_ = storage_gas_refund;
        speculative_cpu_gas_consumed_ = speculative_cpu_gas_consumed;

    }

    static gas_state_t from_result(uint64_t eos_evm_version, const evmc::Result& result) {
        gas_state_t gas_state;
        gas_state.reset(eos_evm_version, result.gas_refund, result.storage_gas_consumed, result.storage_gas_refund, result.speculative_cpu_gas_consumed);
        return gas_state;
    }

    int64_t storage_gas_consumed()const {
        return storage_gas_consumed_;
    }

    int64_t storage_gas_refund()const {
        return storage_gas_refund_;
    }

    int64_t apply_storage_gas_delta(int64_t storage_gas_delta){
        if (eos_evm_version_ >= 3) {
            int64_t d = storage_gas_delta - storage_gas_refund_;
            storage_gas_refund_ = std::max(-d, int64_t{0});
            const auto gas_consumed = std::max(d, int64_t{0});
            storage_gas_consumed_ += gas_consumed;
            return gas_consumed;
        }
        return storage_gas_delta;
    }

    int64_t apply_speculative_cpu_gas_delta(int64_t cpu_gas_delta) {
        if (eos_evm_version_ >= 3) {
            int64_t d = cpu_gas_delta - cpu_gas_refund_;
            cpu_gas_refund_ = std::max(-d, int64_t{0});
            const auto gas_consumed = std::max(d, int64_t{0});
            speculative_cpu_gas_consumed_ += gas_consumed;
            return gas_consumed;
        }
        return cpu_gas_delta;
    }

    int64_t cpu_gas_refund()const {
        return cpu_gas_refund_;
    }

    int64_t integrate(int64_t child_total_gas_to_consume, const gas_state_t& child_gas_state) {
        assert(child_gas_state.eos_evm_version() == eos_evm_version_);
        add_cpu_gas_refund(child_gas_state.cpu_gas_refund());
        storage_gas_refund_ += child_gas_state.storage_gas_refund();

        const auto child_storage_gas_to_consume = child_gas_state.storage_gas_consumed();
        const auto child_speculative_cpu_gas_to_consume = child_gas_state.speculative_cpu_gas_consumed();
        const auto child_real_cpu_gas_consumed = child_total_gas_to_consume - child_storage_gas_to_consume - child_speculative_cpu_gas_to_consume;

        const auto child_total_gas_consumed = apply_storage_gas_delta(child_storage_gas_to_consume) + child_real_cpu_gas_consumed + apply_speculative_cpu_gas_delta(child_speculative_cpu_gas_to_consume);
        return child_total_gas_consumed;
    }

    int64_t collapse() {
        const auto s =  std::min(storage_gas_consumed_, storage_gas_refund_);
        const auto x = storage_gas_consumed_ - storage_gas_refund_;
        storage_gas_consumed_ = std::max(x, int64_t{0});
        storage_gas_refund_ = std::max(-x, int64_t{0});

        const auto c = std::min(speculative_cpu_gas_consumed_, cpu_gas_refund_);
        const auto y = speculative_cpu_gas_consumed_ - cpu_gas_refund_;
        speculative_cpu_gas_consumed_ = std::max(y, int64_t{0});
        cpu_gas_refund_ = std::max(-y, int64_t{0});

        return s + c;
    }

    void add_cpu_gas_refund(int64_t cpu_refund) {
        cpu_gas_refund_ += cpu_refund;
    }

    uint64_t eos_evm_version()const {
        return eos_evm_version_;
    }

    int64_t speculative_cpu_gas_consumed() const {
        return speculative_cpu_gas_consumed_;
    }

private:
    uint64_t eos_evm_version_ = 0;
    int64_t cpu_gas_refund_ = 0;
    int64_t storage_gas_consumed_ = 0;
    int64_t storage_gas_refund_ = 0;
    int64_t speculative_cpu_gas_consumed_ = 0;
};

} //namespace evmone