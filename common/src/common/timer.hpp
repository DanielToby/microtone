#pragma once

#include <chrono>

namespace common {

template <typename Fn>
[[nodiscard]] auto timedInvoke(Fn&& fn) {
    auto start = std::chrono::high_resolution_clock::now();

    if constexpr (std::is_void_v<std::invoke_result_t<Fn>>) {
        std::invoke(std::forward<Fn>(fn));
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    } else {
        auto result = std::invoke(std::forward<Fn>(fn));
        auto end = std::chrono::high_resolution_clock::now();
        return std::make_pair(result, std::chrono::duration_cast<std::chrono::microseconds>(end - start));
    }
}

}
