#pragma once

#include <memory>
#include <string>

#include <SupportDefs.h>

class BBitmap;


namespace HaikuReact {


struct IconSource;


// A loaded icon, shared by everything that shows the same source at the same
// size, plus a dimmed copy for disabled items. Immutable once created, so it
// can be drawn from any window thread.
class SharedBitmap {
private:
	std::unique_ptr<BBitmap> fBitmap;
	std::unique_ptr<BBitmap> fDimmed;

public:
	explicit SharedBitmap(std::unique_ptr<BBitmap> bitmap);
	~SharedBitmap();

	const BBitmap *Bitmap() const {return fBitmap.get();}
	const BBitmap *Dimmed() const {return fDimmed.get();}
};

using SharedBitmapRef = std::shared_ptr<const SharedBitmap>;


// Loaded icons by source and size, kept while anyone uses them. Usable from
// any thread; the last user may release an icon on a window thread.
class BitmapCache {
public:
	// Null if the source has no icon; error tells why.
	static SharedBitmapRef Get(const IconSource &source, int32 size, std::string &error);
	static SharedBitmapRef Get(const IconSource &source, int32 size);

	static size_t Count();
};


} // HaikuReact
