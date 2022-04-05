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

    void addOutputData(const std::vector<float>& data);
    void addMidiData(int status, int note, int velocity);
    void loop();

private:
    class impl;
    std::unique_ptr<impl> _impl;
};


}
