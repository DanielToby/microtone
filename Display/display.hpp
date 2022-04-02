#pragma once

#include <functional>
#include <memory>

namespace microtone::internal {

class Display {
public:
    explicit Display();
    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;
    Display(Display&&) noexcept;
    Display& operator=(Display&&) noexcept;
    ~Display();

    void addNoteData(int status, int note, int velocity);
    void addWaveData(float data);
    void loop();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};


}
