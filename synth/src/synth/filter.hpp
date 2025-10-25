#pragma once

namespace synth {

class Filter {
public:
    Filter() :
        _lastSample{0},
        _alpha{0.5} {}

    Filter(const Filter& other) = default;

    float nextSample(float in) {
        auto out = static_cast<float>(_alpha * _lastSample + (1.0 - _alpha) * in);
        _lastSample = out;
        return out;
    }

private:
    float _lastSample;
    double _alpha;
};

}
