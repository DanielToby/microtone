#pragma once

#include <memory>
#include <string>

#include <common/keyboard.h>

namespace io {

class MidiInputStream {
public:
    MidiInputStream();
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

    //! Thread safe.
    [[nodiscard]] common::Keyboard getKeyboardState() const;

private:
    class impl;
    std::unique_ptr<impl> _impl;
};

}
