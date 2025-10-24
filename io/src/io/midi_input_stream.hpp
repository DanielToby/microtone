#pragma once

#include <memory>
#include <string>

#include <common/midi_handle.hpp>

namespace io {

class MidiInputStream {
public:
    //! Warning: midiHandle must remain alive while this class is alive!
    MidiInputStream(common::midi::MidiHandle& midiHandle);
    MidiInputStream(const MidiInputStream&) = delete;
    MidiInputStream& operator=(const MidiInputStream&) = delete;
    MidiInputStream(MidiInputStream&&) noexcept;
    MidiInputStream& operator=(MidiInputStream&&) noexcept;
    ~MidiInputStream();

    [[nodiscard]] int portCount() const;
    [[nodiscard]] std::string portName(int portNumber) const;
    [[nodiscard]] bool isOpen() const;
    void openPort(int portNumber);
    void start();
    void stop();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
