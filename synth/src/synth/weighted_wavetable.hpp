#pragma once

#include <synth/wavetable.hpp>

namespace synth {

struct WeightedWaveTable {
    WaveTable waveTable;
    double weight;

    WeightedWaveTable(const synth::WaveTable& waveTable, double weight) :
        waveTable{waveTable},
        weight{weight} {}
};

}
