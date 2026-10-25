#pragma once

#include <memory>
#include <string>
#include <vector>

#include <SupportDefs.h>

class BBitmap;


namespace HaikuReact {


// Where an icon or image comes from; parsed from props such as
// {mimeType: 'text/plain'} (see PropSet).
struct IconSource {
	enum class Kind {
		None,
		System,    // vector icon resource of libbe, e.g. "dialog-warning"
		MimeType,  // file-type icon; falls back to the supertype's icon
		File,      // the icon Tracker shows for a file or folder
		App,       // application icon, by signature
		Path,      // HVIF or raster image file
		Data,      // encoded HVIF or raster bytes
	};

	Kind kind = Kind::None;
	// Name, MIME type, path or signature; unused for Data.
	std::string name;
	std::shared_ptr<const std::vector<uint8>> data;

	bool IsNone() const {return kind == Kind::None;}
	// Identifies the source for caching; Data compares by content.
	std::string Key() const;

	bool operator==(const IconSource &other) const;
};


// Icon size in pixels for a nominal size (16, 32, ...), scaled with the
// system font like the rest of the interface.
int32 ScaledIconSize(float size);

// Renders a vector source at size x size pixels. Raster sources keep their
// own size and are scaled when drawn. Returns null on failure and sets
// error. The result is B_RGBA32.
std::unique_ptr<BBitmap> LoadIcon(const IconSource &source, int32 size, std::string &error);

// Decodes a raster image (Path or Data source); may block for a long time.
std::unique_ptr<BBitmap> LoadImage(const IconSource &source, std::string &error);


} // HaikuReact
