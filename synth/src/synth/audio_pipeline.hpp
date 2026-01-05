#pragma once

#include "common/log.hpp"
#include "common/midi_handle.hpp"
#include "common/ring_buffer.hpp"

namespace synth {

//! Produces samples, responds to midi events.
class I_SourceNode {
public:
    virtual ~I_SourceNode() = default;
    [[nodiscard]] virtual common::audio::FrameBlock getNextBlock() = 0;

    //! TODO: remove these.
    virtual void respondToKeyboardChanges(const common::midi::Keyboard&) {}
    [[nodiscard]] virtual double sampleRate() const { return 0.; }
};

//! Optionally accepts samples.
class I_SinkNode {
public:
    virtual ~I_SinkNode() = default;
    [[nodiscard]] virtual bool isFull() const = 0;
    virtual bool push(const common::audio::FrameBlock& block) = 0;
};

class OutputDevice : public I_SinkNode {
public:
    explicit OutputDevice(std::shared_ptr<common::RingBuffer<common::audio::FrameBlock>> buffer) : _buffer{std::move(buffer)} {}

    [[nodiscard]] bool isFull() const override { return _buffer->isFull(); }
    bool push(const common::audio::FrameBlock& block) override {
        return _buffer->push(block);
    }
private:
    std::shared_ptr<common::RingBuffer<common::audio::FrameBlock>> _buffer;
};

//! Anything that transforms the signal is a function node. These forward audio from the input to the output, and apply a transformation in between.
class I_FunctionNode : public I_SourceNode, public I_SinkNode {
public:
    bool push(const common::audio::FrameBlock& block) override {
        _nextBlock = block;
        for (auto& sample : _nextBlock) {
            sample = this->transform(sample);
        }
        return true;
    }

    [[nodiscard]] common::audio::FrameBlock getNextBlock() override { return _nextBlock; };

protected:
    //! Modifies the input signal. Invoked on every sample in every frame pushed.
    [[nodiscard]] virtual float transform(float in) = 0;

private:
    [[nodiscard]] bool isFull() const final { return false; }

    common::audio::FrameBlock _nextBlock{common::audio::emptyFrameBlock};
};

//! An audio pipeline (for now) consists of one input node, n effects nodes, and one output node.
//! Effects are applied in the order they are provided in.
class AudioPipeline {
public:
    AudioPipeline(std::shared_ptr<I_SourceNode> source, std::shared_ptr<I_SinkNode> sink) :
        _source{std::move(source)},
        _sink{std::move(sink)} {}

    void addEffect(std::unique_ptr<I_FunctionNode> effect) {
        _effects.push_back(std::move(effect));
    }

    [[nodiscard]] bool shouldProcessBlock() const {
        return !_sink->isFull();
    }

    //! TODO: Remove, clean up midi event handling.
    [[nodiscard]] I_SourceNode& getSource() { return *_source; }

    //! Reads from source, applies effects, writes to sink.
    void processBlock();

private:
    std::shared_ptr<I_SourceNode> _source;
    std::vector<std::unique_ptr<I_FunctionNode>> _effects;
    std::shared_ptr<I_SinkNode> _sink;
};

}