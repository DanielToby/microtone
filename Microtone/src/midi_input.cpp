#include <microtone/exception.hpp>
#include <microtone/log.hpp>
#include <microtone/midi_input.hpp>

#include <rtmidi/RtMidi.h>

#include <atomic>
#include <chrono>
#include <thread>

namespace microtone {

class MidiInput::impl {
public:
    impl() {
        _rtMidiConnection = std::make_unique<RtMidiIn>();
        // Don't ignore sysex, timing, or active sensing messages.
        _rtMidiConnection->ignoreTypes(false, false, false);
    }

    ~impl() {
        stop();
    }

    int portCount() {
        return _rtMidiConnection->getPortCount();
    }

    std::string portName(int portNumber) {
        return _rtMidiConnection->getPortName(portNumber);
    }

    void openPort(int portNumber) {
        _rtMidiConnection->openPort(portNumber);
    }

    void start(OnMidiDataFn onReceivedDataFn) {
        if (!_rtMidiConnection->isPortOpen()) {
            throw MicrotoneException("A port must be open to read midi input data.");
        }

        _dataThread = std::thread([onReceivedDataFn, this]() {
            M_INFO("Started data collection in background thread.");
            _isRunning.store(true);
            while (_isRunning.load() == true) {
                auto message = std::vector<unsigned char>{};
                _rtMidiConnection->getMessage(&message);
                if (message.size() == 3) {
                    auto status = static_cast<int>(message[0]);
                    auto note = static_cast<int>(message[1]);
                    auto velocity = static_cast<int>(message[2]);

                    onReceivedDataFn(status, note, velocity);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            M_INFO("Stopped data collection.");
        });
    }

    void stop() {
        if (_isRunning.load() == true) {
            _isRunning.store(false);
            _dataThread.join();
        }
    }

    std::unique_ptr<RtMidiIn> _rtMidiConnection;
    std::thread _dataThread;
    std::atomic<bool> _isRunning = false;
};

MidiInput::MidiInput() :
    _impl{new impl{}} {
}

MidiInput::MidiInput(MidiInput&& other) noexcept :
    _impl{std::move(other._impl)} {
}

MidiInput& MidiInput::operator=(MidiInput&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

MidiInput::~MidiInput() = default;

int MidiInput::portCount() const {
    return _impl->portCount();
}

std::string MidiInput::portName(int portNumber) const {
    return _impl->portName(portNumber);
}

void MidiInput::openPort(int portNumber) {
    _impl->openPort(portNumber);
}

void MidiInput::start(OnMidiDataFn fn) {
    _impl->start(fn);
}

void MidiInput::stop() {
    _impl->stop();
}

}
