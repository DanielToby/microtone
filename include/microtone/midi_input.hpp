#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace microtone {

using OnMidiDataFn = std::function<void(double, std::string)>;

class MidiInput {
public:
    explicit MidiInput();
    MidiInput(const MidiInput&) = delete;
    MidiInput& operator=(const MidiInput&) = delete;
    MidiInput(MidiInput&&) noexcept;
    MidiInput& operator=(MidiInput&&) noexcept;
    ~MidiInput();

    int portCount() const;
    std::string portName(int portNumber) const;
    void openPort(int portNumber);
    void start(OnMidiDataFn onReceivedDataFn);
    void stop();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
