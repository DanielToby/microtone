#pragma once

#include <unordered_set>
#include <vector>

#include <synth/voice.hpp>

namespace synth {

//! The supported midi operations.
class I_Controller {
public:
    virtual ~I_Controller() = default;
    virtual void noteOn(int note, int velocity) = 0;
    virtual void noteOff(int note) = 0;
    virtual void sustainOn() = 0;
    virtual void sustainOff() = 0;
};

}