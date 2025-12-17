#pragma once

#include <chrono>

namespace common {

template <typename Fn>
[[nodiscard]] std::pair<std::invoke_result_t<Fn>, std::chrono::microseconds> timedInvoke(Fn&& fn) {
    auto start = std::chrono::high_resolution_clock::now();
    auto result = std::invoke(std::forward<Fn>(fn));
    auto end = std::chrono::high_resolution_clock::now();

    return {result, std::chrono::duration_cast<std::chrono::microseconds>(end - start)};
}

}
