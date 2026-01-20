#pragma once

#include "common/log.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>

#include <string>

namespace asciiboard {
class InfoMessage {
public:
    explicit InfoMessage(const std::string& firstMessage, const std::string& secondMessage) :
        _firstMessage(firstMessage),
        _secondMessage(secondMessage) {
        using namespace ftxui;
        _closeButton = Button("Ok", [this] { _show = false; });
        _info = Renderer(_closeButton, [this] {
            return vbox({text(_firstMessage) | hcenter,
                         separatorEmpty() | size(HEIGHT, EQUAL, 1),
                         text(_secondMessage) | hcenter,
                         separatorEmpty() | size(HEIGHT, EQUAL, 1),
                         hbox({filler(),
                               _closeButton->Render(),
                               filler()})}) |
                   hcenter | border;
        });
    }

    [[nodiscard]] bool show() const { return _show; }
    [[nodiscard]] const ftxui::Component& component() const { return _info; }

private:
    bool _show{true};
    std::string _firstMessage;
    std::string _secondMessage;

    ftxui::Component _closeButton;
    ftxui::Component _info;
};

}
