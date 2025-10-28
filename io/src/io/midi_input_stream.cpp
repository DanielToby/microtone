#include <io/midi_input_stream.hpp>

#include <common/exception.hpp>
#include <common/log.hpp>
#include <io/midi_message.hpp>

#include <RtMidi.h>

#include <atomic>
#include <chrono>
#include <thread>

namespace io {

namespace {
void addMidiData(common::midi::MidiHandle& handle, const MidiMessage& m) {
    switch (static_cast<MidiStatusMessage>(m.status)) {
    case MidiStatusMessage::NoteOn:
        handle.noteOn(m.note, m.velocity);
    case MidiStatusMessage::NoteOff:
        handle.noteOff(m.note);
    case MidiStatusMessage::ControlChange:
        switch (static_cast<MidiNoteMessage>(m.note)) {
        case MidiNoteMessage::SustainPedal:
            if (m.velocity > static_cast<int>(MidiVelocityMessage::SustainPedalOnThreshold)) {
                handle.sustainOn();
            } else {
                handle.sustainOff();
            }
        }
    }
}
}

class MidiInputStream::impl {
public:
    explicit impl(std::shared_ptr<common::midi::MidiHandle> midiHandle) :
        _midiHandle(std::move(midiHandle)),
        _rtMidiConnection{std::make_unique<RtMidiIn>()} {
        // Don't ignore sysex, timing, or active sensing messages.
        _rtMidiConnection->ignoreTypes(false, false, false);
    }

    ~impl() {
        stop();
    }

    [[nodiscard]] uint8_t portCount() const {
        return _rtMidiConnection->getPortCount();
    }

    [[nodiscard]] std::string portName(int portNumber) const {
        return _rtMidiConnection->getPortName(portNumber);
    }

    [[nodiscard]] bool isOpen() const {
        return _rtMidiConnection->isPortOpen();
    }

    void openPort(int portNumber) {
        _rtMidiConnection->openPort(portNumber);
    }

    void start() {
        if (!_rtMidiConnection->isPortOpen()) {
            throw common::MicrotoneException("A port must be open to read midi input data.");
        }

        _dataThread = std::thread([this]() {
            M_INFO("Opened midi input stream.");
            _isRunning.store(true);
            while (_isRunning.load() == true) {
                auto message = std::vector<unsigned char>{};
                _rtMidiConnection->getMessage(&message);
                if (message.size() == 3) {
                    auto status = static_cast<int>(message[0]);
                    auto note = static_cast<int>(message[1]);
                    auto velocity = static_cast<int>(message[2]);

                    addMidiData(*_midiHandle, MidiMessage{status, note, velocity});
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            M_INFO("Closed midi input stream.");
        });
    }

    void stop() {
        if (_isRunning.load() == true) {
            _isRunning.store(false);
            _dataThread.join();
        }
    }

private:
    std::shared_ptr<common::midi::MidiHandle> _midiHandle;
    std::unique_ptr<RtMidiIn> _rtMidiConnection;
    std::thread _dataThread;
    std::atomic<bool> _isRunning = false;
};

MidiInputStream::MidiInputStream(std::shared_ptr<common::midi::MidiHandle> stateHandle) :
    _impl{std::make_unique<impl>(stateHandle)} {
}

MidiInputStream::MidiInputStream(MidiInputStream&& other) noexcept :
    _impl{std::move(other._impl)} {
}

MidiInputStream& MidiInputStream::operator=(MidiInputStream&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

MidiInputStream::~MidiInputStream() = default;

int MidiInputStream::portCount() const {
    return _impl->portCount();
}

std::string MidiInputStream::portName(int portNumber) const {
    return _impl->portName(portNumber);
}

bool MidiInputStream::isOpen() const {
    return _impl->isOpen();
}

void MidiInputStream::openPort(int portNumber) {
    _impl->openPort(portNumber);
}

void MidiInputStream::start() {
    _impl->start();
}

void MidiInputStream::stop() {
    _impl->stop();
}

}
