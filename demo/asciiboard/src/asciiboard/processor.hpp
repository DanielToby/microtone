#pragma once

#include <atomic>
#include <thread>

#include <io/audio_output_stream.hpp>
#include <synth/synthesizer.hpp>

namespace asciiboard {

//! TODO: Move this somewhere else.
class Processor {
public:
    Processor(synth::Synthesizer& synth, io::AudioOutputStream& outputStream)
        : _synth(synth), _outputStream(outputStream) {}

    void start() {
        _running = true;
        thread_ = std::thread([this] { this->run(); });
    }

    void stop() {
        _running = false;
        if (thread_.joinable()) thread_.join();
    }

private:
    void run() {
        using namespace std::chrono;
        const auto samplePeriod = duration<double>(1.0 / _outputStream.sampleRate());
        auto nextTime = steady_clock::now();

        // for (auto frame = 0; frame < static_cast<int>(framesPerBuffer); ++frame) {
        //     auto sample = data->nextSample();
        //     data->_lastOutputBuffer[frame] = sample;
        //     for (std::size_t channel = 0; channel < 2; ++channel) {
        //         *out++ = sample;
        //     }
        // }

        while (_running) {
            float sample = _synth.nextSample();
            _outputStream.push(sample);

            // Wait until it's time for the next sample
            nextTime += duration_cast<steady_clock::duration>(samplePeriod);;
            std::this_thread::sleep_until(nextTime);
        }
    }

    synth::Synthesizer& _synth;
    io::AudioOutputStream& _outputStream;

    std::atomic<bool> _running{false};
    std::thread thread_;
};


}