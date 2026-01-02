#pragma once

#include "common/midi_handle.hpp"
#include "common/ring_buffer.hpp"

namespace synth {

//! Produces samples, responds to midi events.
class I_Source {
public:
    virtual ~I_Source() = default;
    [[nodiscard]] virtual common::audio::FrameBlock getNextBlock() = 0;
    virtual void respondToKeyboardChanges(const common::midi::Keyboard& keyboard) = 0;

    [[nodiscard]] virtual double sampleRate() const = 0;
};

//! Optionally accepts samples.
class I_Sink {
public:
    virtual ~I_Sink() = default;
    [[nodiscard]] virtual bool isFull() const = 0;
    virtual bool push(const common::audio::FrameBlock& block) const = 0;
};

//! Warning! This doesn't guard access between threads.
class AudioOutput : public I_Sink {
public:
    explicit AudioOutput(std::shared_ptr<common::RingBuffer<common::audio::FrameBlock>> buffer) : _buffer{std::move(buffer)} {}

    [[nodiscard]] bool isFull() const override { return _buffer->isFull(); }
    bool push(const common::audio::FrameBlock& block) const override {
        return _buffer->push(block);
    }

private:
    std::shared_ptr<common::RingBuffer<common::audio::FrameBlock>> _buffer;
};

}