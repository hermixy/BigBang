#pragma once

private value struct TextExtent {
    float width;
    float height;
    float tspace;
    float rspace;
    float bspace;
    float lspace;
};

Microsoft::Graphics::Canvas::Text::CanvasTextLayout^ make_text_layout(
    Platform::String^ pare,
    Microsoft::Graphics::Canvas::Text::CanvasTextFormat^ font);

Microsoft::Graphics::Canvas::Text::CanvasTextLayout^ make_vertical_layout(
    Platform::String^ pare,
    Microsoft::Graphics::Canvas::Text::CanvasTextFormat^ font,
    float spacing,
    Microsoft::Graphics::Canvas::Text::CanvasHorizontalAlignment align);

Microsoft::Graphics::Canvas::Text::CanvasTextFormat^ make_text_format(
    float size = 12.0F,
    Microsoft::Graphics::Canvas::Text::CanvasWordWrapping wrapping = Microsoft::Graphics::Canvas::Text::CanvasWordWrapping::NoWrap,
    Microsoft::Graphics::Canvas::Text::CanvasHorizontalAlignment align = Microsoft::Graphics::Canvas::Text::CanvasHorizontalAlignment::Left);

TextExtent get_text_extent(
    Platform::String^ message,
    Microsoft::Graphics::Canvas::Text::CanvasTextFormat^ font,
    bool is_trim = false);

TextExtent get_text_extent(
    Microsoft::Graphics::Canvas::ICanvasResourceCreator^ ds,
    Platform::String^ message,
    Microsoft::Graphics::Canvas::Text::CanvasTextFormat^ font,
    bool is_trim = false);