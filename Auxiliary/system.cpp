#include <algorithm>

#include "system.hpp"

using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

using namespace Windows::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::ViewManagement;

static UISettings^ sysUI = nullptr;

static inline Size adjust_size(float Width, float Height, FrameworkElement^ workspace) {
    auto margin = workspace->Margin;

    float width = Width - float(margin.Left + margin.Right);
    float height = Height - float(margin.Top + margin.Bottom);
    return Size(width, height);
}

Size adjusted_workspace_size(Rect region, FrameworkElement^ workspace) {
	return adjust_size(region.Width, region.Height, workspace);
}

Size system_screen_size() {
    auto master = DisplayInformation::GetForCurrentView();
    auto scaling = float(master->RawPixelsPerViewPixel);

    return { float(master->ScreenWidthInRawPixels) / scaling, float(master->ScreenHeightInRawPixels) / scaling };
}

float system_resolution_fit_scaling(float target_width, float target_height) {
	Size screen = system_screen_size();

	return std::fmin(screen.Width / target_width, screen.Height / target_height);
}

Color system_color(UIColorType type) {
    if (sysUI == nullptr) {
        sysUI = ref new UISettings();
    }

    return sysUI->GetColorValue(type);
}

Color system_color(UIElementType type) {
    if (sysUI == nullptr) {
        sysUI = ref new UISettings();
    }

    return sysUI->UIElementColor(type);
}
