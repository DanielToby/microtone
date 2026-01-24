#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>

#include "asciiboard/state.hpp"
#include "common/log.hpp"

#include <string>

namespace asciiboard {
class InfoMessage {
public:
    explicit InfoMessage(const std::string& firstMessage, const std::string& secondMessage, const std::shared_ptr<State>& controls) :
        _firstMessage(firstMessage),
        _secondMessage(secondMessage),
        _controls(controls) {}

    [[nodiscard]] ftxui::Component component() const {
        using namespace ftxui;
        auto closeButton = Button("Ok", [this] { _controls->showInfoMessage = false; });
        return Renderer(closeButton, [=] {
            return vbox({text(_firstMessage) | hcenter,
                         separatorEmpty() | size(HEIGHT, EQUAL, 1),
                         text(_secondMessage) | hcenter,
                         separatorEmpty() | size(HEIGHT, EQUAL, 1),
                         hbox({filler(),
                               closeButton->Render(),
                               filler()})}) |
                   hcenter | border;
        });
    }

private:
    std::string _firstMessage;
    std::string _secondMessage;
    std::shared_ptr<State> _controls;
};

}
