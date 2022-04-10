#pragma once

#include <microtone/microtone_platform.hpp>
#include <microtone/synthesizer/wavetable.hpp>

namespace microtone {

struct WeightedWaveTable {
    WaveTable waveTable;
    double weight;

    WeightedWaveTable(const microtone::WaveTable& waveTable, double weight) :
        waveTable{waveTable},
        weight{weight} {}
};

}
