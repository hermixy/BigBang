#pragma once

#include "network.hxx"
#include "universe.hxx"

namespace WarGrey::SCADA {
    [::Windows::Foundation::Metadata::WebHostHidden]
    public ref class WorkSpace sealed : public Windows::UI::Xaml::Controls::StackPanel {
    public:
        WorkSpace();
        void initialize_component(Windows::Foundation::Size region);

    public:
        void reflow(float width, float height);
        void suspend(Windows::ApplicationModel::SuspendingOperation^ op);

    private:
        WarGrey::SCADA::TCPListener^ listener;
        WarGrey::SCADA::Pasteboard^ statusbar;
        WarGrey::SCADA::Pasteboard^ stage;
        WarGrey::SCADA::Pasteboard^ gauge;
        WarGrey::SCADA::Pasteboard^ taskbar;
    };
}
