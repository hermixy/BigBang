#include <ppltasks.h>
#include <map>

#include "graphlet/svglet.hpp"
#include "planet.hpp"

#include "colorspace.hpp"
#include "path.hpp"
#include "draw.hpp"

using namespace WarGrey::SCADA;

using namespace Concurrency;

using namespace Windows::UI;
using namespace Windows::Foundation;

using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Svg;

static std::map<int, cancellation_token_source> lazy_tasks;

/*************************************************************************************************/
Svglet::Svglet(Platform::String^ file, Platform::String^ rootdir) : Svglet(file, 0.0F, 0.0F, rootdir) {}

Svglet::Svglet(Platform::String^ file, float width, float height, Platform::String^ rootdir) {
	this->viewport.Width = width;
	this->viewport.Height = height;
	this->ms_appx_svg = ms_appx_path(file, rootdir, ".svg");
}

Svglet::~Svglet() {
	int uuid = this->ms_appx_svg->GetHashCode();
	auto t = lazy_tasks.find(uuid);

	if (t != lazy_tasks.end()) {
		if (this->graph_svg == nullptr) {
			t->second.cancel();
		}

		lazy_tasks.erase(t);
	}
}

void Svglet::construct() {
	int uuid = this->ms_appx_svg->GetHashCode();
	cancellation_token_source svg_task;
	cancellation_token svg_token = svg_task.get_token();

	lazy_tasks.insert(std::pair<int, cancellation_token_source>(uuid, svg_task));

	create_task(StorageFile::GetFileFromApplicationUriAsync(this->ms_appx_svg), svg_token).then([=](task<StorageFile^> svg) {
		this->get_logger()->log_message(Log::Debug,
			L"Found the graphlet source: %s",
			this->ms_appx_svg->ToString()->Data());

		return create_task(svg.get()->OpenAsync(FileAccessMode::Read, StorageOpenOptions::AllowOnlyReaders), svg_token);
	}).then([=](task<IRandomAccessStream^> svg) {
		return create_task(CanvasSvgDocument::LoadAsync(CanvasDevice::GetSharedDevice(), svg.get()), svg_token);
	}).then([=](task<CanvasSvgDocument^> doc_svg) {
		try {
			this->graph_svg = doc_svg.get();
			this->root = this->graph_svg->Root;

			if ((this->viewport.Width > 0.0F) && (this->viewport.Height > 0.0F)) {
				root->SetLengthAttribute("width", 100.0F, CanvasSvgLengthUnits::Percentage);
				root->SetLengthAttribute("height", 100.0F, CanvasSvgLengthUnits::Percentage);
			} else {
				CanvasSvgLengthUnits width_units, height_units;
				float width = this->get_length_attribute("width", &width_units, false);
				float height = this->get_length_attribute("height", &height_units, false);

				// TODO: what if one of the lengths is measured in percentage
				if (this->viewport.Width <= 0.0F) {
					if (this->viewport.Height <= 0.0F) {
						this->viewport.Height = height;
					}
					this->viewport.Width = width * (this->viewport.Height / height);
				} else {
					this->viewport.Height = height * (this->viewport.Width / width);
				}
			}

			this->info->master->notify_graphlet_ready(this);
		} catch (Platform::Exception^ e) {
			this->get_logger()->log_message(Log::Error,
				L"Failed to load %s: %s",
				this->ms_appx_svg->ToString()->Data(),
				e->Message->Data());
		} catch (task_canceled&) {
			this->get_logger()->log_message(Log::Debug,
				L"Cancelled loading %s",
				this->ms_appx_svg->ToString()->Data());
		}
	});
}

bool Svglet::ready() {
	return (this->graph_svg != nullptr);
}

void Svglet::fill_extent(float x, float y, float* w, float* h) {
	SET_VALUES(w, this->viewport.Width, h, this->viewport.Height);
}

void Svglet::draw(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
	ds->DrawSvg(this->graph_svg, this->viewport, x, y);
}

void Svglet::draw_progress(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
	Platform::String^ hint = file_name_from_path(this->ms_appx_svg);

	draw_invalid_bitmap(hint, ds, x, y, this->viewport.Width, this->viewport.Height);
}

/*************************************************************************************************/
Color Svglet::get_fill_color(Platform::String^ id, Color& default_color) {
	return this->get_child_color_attribute(id, "fill", default_color);
}

void Svglet::set_fill_color(Platform::String^ id, Windows::UI::Color& c) {
	this->set_child_color_attribute(id, "fill", c);
}

void Svglet::set_fill_color(Platform::String^ id, unsigned int hex, double alpha) {
	this->set_fill_color(id, rgba(hex, alpha));
}

void Svglet::set_fill_color(Platform::String^ id, WarGrey::SCADA::Colour^ brush) {
	this->set_fill_color(id, brush->Color);
}

Color Svglet::get_stroke_color(Platform::String^ id, Color& default_color) {
	return this->get_child_color_attribute(id, "stroke", default_color);
}

void Svglet::set_stroke_color(Platform::String^ id, Windows::UI::Color& c) {
	this->set_child_color_attribute(id, "stroke", c);
}

void Svglet::set_stroke_color(Platform::String^ id, unsigned int hex, double alpha) {
	this->set_stroke_color(id, rgba(hex, alpha));
}

void Svglet::set_stroke_color(Platform::String^ id, WarGrey::SCADA::Colour^ brush) {
	this->set_stroke_color(id, brush->Color);
}

/*************************************************************************************************/
float Svglet::get_length_attribute(Platform::String^ name, CanvasSvgLengthUnits* units, bool inherited) {
	float value = 100.0F; // This value is also the default length if both the client program and the SVG itself do not specific the size.
	(*units) = CanvasSvgLengthUnits::Percentage;

	if (this->root != nullptr) {
		if (this->root->IsAttributeSpecified(name, inherited)) {
			value = this->root->GetLengthAttribute(name, units);
		}
	}

	return value;
}

Color Svglet::get_child_color_attribute(Platform::String^ id, Platform::String^ attribute_name, Color& default_color) {
	/** WARNING
	* This method is intended to be invoked correctly, so itself does not check the input.
	* That means the `id` should be defined in the SVG document, or the application will crash in development stage.
	*/
	CanvasSvgNamedElement^ child = this->graph_svg->FindElementById(id);

	return child->IsAttributeSpecified(attribute_name) ? child->GetColorAttribute(attribute_name) : default_color;
}

void Svglet::set_child_color_attribute(Platform::String^ id, Platform::String^ attribute_name, Windows::UI::Color& c) {
	// Warnings see `Svglet::get_child_color_attribute`
	CanvasSvgNamedElement^ child = this->graph_svg->FindElementById(id);

	child->SetColorAttribute(attribute_name, c);
}
