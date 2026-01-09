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
void addMidiData(common::midi::TwoReaderMidiHandle& handle, const MidiMessage& m) {
    const auto status = static_cast<MidiStatusMessage>(m.status);
    if (status == MidiStatusMessage::NoteOn) {
        handle.noteOn(m.note, m.velocity);
    } else if (status == MidiStatusMessage::NoteOff) {
        handle.noteOff(m.note);
    } else if (status == MidiStatusMessage::ControlChange) {
        const auto midiNoteMessage = static_cast<MidiNoteMessage>(m.note);
        if (midiNoteMessage == MidiNoteMessage::SustainPedal) {
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
    explicit impl(std::shared_ptr<common::midi::TwoReaderMidiHandle> midiHandle) :
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

        M_INFO("Opened midi input stream.");

        _rtMidiConnection->setCallback(
            &impl::midiCallback,
            this
        );

        _isRunning.store(true, std::memory_order_release);
    }

    void stop() {
        if (!_isRunning.exchange(false)) {
            return;
        }

        M_INFO("Closed midi input stream.");

        if (_rtMidiConnection) {
            _rtMidiConnection->cancelCallback();
        }
    }

private:
    static void midiCallback(
        double /*deltaTime*/,
        std::vector<unsigned char>* message,
        void* userData
    ) {
        auto* self = static_cast<impl*>(userData);

        if (!self->_isRunning.load(std::memory_order_relaxed)) {
            return;
        }

        if (!message || message->size() != 3) {
            return;
        }

        const int status   = static_cast<int>((*message)[0]);
        const int note     = static_cast<int>((*message)[1]);
        const int velocity = static_cast<int>((*message)[2]);

        addMidiData(
            *self->_midiHandle,
            MidiMessage{status, note, velocity});
    }

    std::shared_ptr<common::midi::TwoReaderMidiHandle> _midiHandle;
    std::unique_ptr<RtMidiIn> _rtMidiConnection;
    std::atomic<bool> _isRunning{false};
};

MidiInputStream::MidiInputStream(std::shared_ptr<common::midi::TwoReaderMidiHandle> stateHandle) :
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

MidiInputStream::~MidiInputStream() {
    this->stop();
}

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
