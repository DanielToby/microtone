#pragma once

#include <array>
#include <functional>


#define M_PI 3.14159265358979323846

namespace microtone {

const std::size_t WAVETABLE_LENGTH = 512;
using WaveTable = std::array<float, WAVETABLE_LENGTH>;

//! Constructs the wave table using fillFn, fetching a value for each index i in WAVETABLE_LENGTH
[[nodiscard]] inline WaveTable buildWaveTable(std::function<float(std::size_t i)>&& fillFn) {
    auto result = WaveTable{};
    for (auto i = 0; i < WAVETABLE_LENGTH; ++i) {
        result[i] = std::invoke(fillFn, i);
    }
    return result;
}

//! Constexpr overload.
template <typename FillFn>
[[nodiscard]] constexpr WaveTable buildWaveTable(FillFn&& fillFn) {
    auto result = WaveTable{};
    for (auto i = 0; i < WAVETABLE_LENGTH; ++i) {
        result[i] = fillFn(i);
    }
    return result;
}

namespace examples {

//! Returns the corresponding sine wave table value at index i.
[[nodiscard]] /* C++26: constexpr */ inline float sineWaveFill(std::size_t i) {
    auto theta = 2.f * M_PI * i / WAVETABLE_LENGTH;
    return std::sin(theta);
}

//! Returns the corresponding square wave table value at index i.
[[nodiscard]] /* C++26: constexpr */ inline float squareWaveFill(std::size_t i) {
    auto theta = 2.f * M_PI * i / WAVETABLE_LENGTH;
    return std::sin(theta) > 0 ? 1.0 : -1.0;
}

//! Returns the corresponding square wave table value at index i.
[[nodiscard]] /* C++26: constexpr */ inline float triangleWaveFill(std::size_t i) {
    auto theta = 2.f * M_PI * i / WAVETABLE_LENGTH;
    return std::asin(std::sin(theta)) * (2.0 / M_PI);
}

}

}
