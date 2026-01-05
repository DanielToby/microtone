#pragma once

#include "common/midi_handle.hpp"
#include "common/ring_buffer.hpp"

namespace synth {

//! Produces samples, responds to midi events.
class I_SourceNode {
public:
    virtual ~I_SourceNode() = default;
    [[nodiscard]] virtual common::audio::FrameBlock getNextBlock() = 0;

    virtual void respondToKeyboardChanges(const common::midi::Keyboard&) {}

    [[nodiscard]] virtual double sampleRate() const = 0;
};

//! Optionally accepts samples.
class I_SinkNode {
public:
    virtual ~I_SinkNode() = default;
    [[nodiscard]] virtual bool isFull() const = 0;
    virtual bool push(const common::audio::FrameBlock& block) const = 0;
};

//! Anything that transforms the signal is a function node. These forward audio
class I_FunctionNode : public I_SourceNode, public I_SinkNode {
public:
    virtual ~I_FunctionNode() = default;

    //! Modifies the input signal. Invoked on block push.
    [[nodiscard]] virtual common::audio::FrameBlock transform(const common::audio::FrameBlock& in) const = 0;

private:
    [[nodiscard]] virtual common::audio::FrameBlock processBlock(const common::audio::FrameBlock& in) = 0;

    bool push(const common::audio::FrameBlock& block) {
        _nextBlock = this->transform(block);
        return true;
    }

    [[nodiscard]] common::audio::FrameBlock getNextBlock() override { return _nextBlock; };

    [[nodiscard]] bool isFull() const final { return false; }

    common::audio::FrameBlock _nextBlock{common::audio::emptyFrameBlock};
};

class OutputDevice : public I_SinkNode {
public:
    explicit OutputDevice(std::shared_ptr<common::RingBuffer<common::audio::FrameBlock>> buffer) : _buffer{std::move(buffer)} {}

    [[nodiscard]] bool isFull() const override { return _buffer->isFull(); }
    bool push(const common::audio::FrameBlock& block) const override {
        return _buffer->push(block);
    }

private:
    std::shared_ptr<common::RingBuffer<common::audio::FrameBlock>> _buffer;
};

}