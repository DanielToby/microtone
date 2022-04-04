#include <display.hpp>

#include "ftxui/component/component.hpp"         // for Checkbox, Renderer, Horizontal, Vertical, Input, Menu, Radiobox, ResizableSplitLeft, Tab
#include "ftxui/component/component_base.hpp"    // for ComponentBase
#include "ftxui/component/component_options.hpp" // for MenuOption, InputOption
#include "ftxui/component/event.hpp"             // for Event, Event::Custom
#include "ftxui/component/screen_interactive.hpp"// for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"                // for text, color, operator|, bgcolor, filler, Element, vbox, size, hbox, separator, flex, window, graph, EQUAL, paragraph, WIDTH, hcenter, Elements, bold, vscroll_indicator, HEIGHT, flexbox, hflow, border, frame, flex_grow, gauge, paragraphAlignCenter, paragraphAlignJustify, paragraphAlignLeft, paragraphAlignRight, dim, spinner, LESS_THAN, center, yframe, GREATER_THAN
#include "ftxui/dom/flexbox_config.hpp"          // for FlexboxConfig
#include "ftxui/screen/color.hpp"                // for Color, Color::BlueLight, Color::RedLight, Color::Black, Color::Blue, Color::Cyan, Color::CyanLight, Color::GrayDark, Color::GrayLight, Color::Green, Color::GreenLight, Color::Magenta, Color::MagentaLight, Color::Red, Color::White, Color::Yellow, Color::YellowLight, Color::Default, Color::Palette256, ftxui
#include "ftxui/screen/color_info.hpp"           // for ColorInfo
#include "ftxui/screen/terminal.hpp"             // for Size, Dimensions

namespace microtone::internal {

using namespace ftxui;

class Display::impl {
public:
    impl() :
        _screen{ScreenInteractive::Fullscreen()},
        _data{} {
    }

    void addOutputData(const std::vector<float> &data) {
        _data.insert(_data.begin(), data.begin(), data.end());
        _screen.PostEvent(ftxui::Event::Custom);
    }

    void loop() {

        auto oscilloscope = [this](int width, int height) {
            std::vector<int> output(width);
            for (auto i = 0; i < width; ++i) {
                if (i < static_cast<int>(_data.size())) {
                    output[width - i - 1] = static_cast<int>(fabs(_data[_data.size() - i - 1]) * height);
                    std::cout << output[width - i - 1];
                }
            }
            return output;
        };

        auto synthesizer = Renderer([&oscilloscope] {
            return vbox({
                text("Frequency [Mhz]") | hcenter,
                hbox({
                    vbox({
                        text("2400 "),
                        filler(),
                        text("1200 "),
                        filler(),
                        text("0 "),
                    }),
                    graph(std::ref(oscilloscope)) | flex,
                }) | flex,
            });
        });

        auto tab_index = 0;
        auto tab_entries = std::vector<std::string>{"Synthesizer"};
        auto tab_selection = Menu(&tab_entries, &tab_index, MenuOption::HorizontalAnimated());
        auto tab_content = Container::Tab(
            {synthesizer},
            &tab_index);

        auto main_container = Container::Vertical({
            tab_selection,
            tab_content,
        });

        auto main_renderer = Renderer(main_container, [&tab_selection, &tab_content] {
            return vbox({
                text("microtone") | bold | hcenter,
                tab_selection->Render(),
                tab_content->Render() | flex,
            });
        });

        _screen.Loop(main_renderer);
    }

    ftxui::ScreenInteractive _screen;
    std::vector<float> _data;
};

Display::Display() :
    _impl{new impl{}} {
}

Display::Display(Display&& other) noexcept :
    _impl{std::move(other._impl)} {
}

Display& Display::operator=(Display&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

void Display::addOutputData(const std::vector<float> &data) {
    _impl->addOutputData(data);
}

void Display::loop() {
    _impl->loop();
}

Display::~Display() = default;

}
