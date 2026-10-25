#include "BitmapCache.h"

#include <map>
#include <mutex>

#include <Bitmap.h>

#include "IconSource.h"


namespace HaikuReact {


static std::mutex sLock;
static std::map<std::string, std::weak_ptr<const SharedBitmap>> sEntries;


static std::unique_ptr<BBitmap> MakeDimmed(const BBitmap &bitmap)
{
	auto dimmed = std::make_unique<BBitmap>(bitmap.Bounds(), B_RGBA32);
	if (dimmed->InitCheck() != B_OK || dimmed->ImportBits(&bitmap) != B_OK) {
		return nullptr;
	}
	auto *bits = static_cast<uint8*>(dimmed->Bits());
	for (size_t i = 3; i < static_cast<size_t>(dimmed->BitsLength()); i += 4) {
		bits[i] /= 2;
	}
	return dimmed;
}


SharedBitmap::SharedBitmap(std::unique_ptr<BBitmap> bitmap):
	fBitmap(std::move(bitmap)),
	fDimmed(MakeDimmed(*fBitmap))
{
}


SharedBitmap::~SharedBitmap() = default;


SharedBitmapRef BitmapCache::Get(const IconSource &source, int32 size, std::string &error)
{
	if (source.IsNone()) {
		return nullptr;
	}
	std::string key = source.Key() + '@' + std::to_string(size);
	{
		std::lock_guard guard(sLock);
		const auto it = sEntries.find(key);
		if (it != sEntries.end()) {
			if (SharedBitmapRef existing = it->second.lock()) {
				return existing;
			}
		}
	}

	// Loading may ask the registrar or read files; not under the lock. Two
	// threads loading the same icon at once both succeed, one entry wins.
	std::unique_ptr<BBitmap> bitmap = LoadIcon(source, size, error);
	if (bitmap == nullptr) {
		return nullptr;
	}

	// The deleter removes the entry, unless it was replaced in between.
	SharedBitmapRef result(new SharedBitmap(std::move(bitmap)), [key](const SharedBitmap *entry) {
		{
			std::lock_guard guard(sLock);
			const auto it = sEntries.find(key);
			if (it != sEntries.end() && it->second.expired()) {
				sEntries.erase(it);
			}
		}
		delete entry;
	});
	std::lock_guard guard(sLock);
	sEntries[key] = result;
	return result;
}


SharedBitmapRef BitmapCache::Get(const IconSource &source, int32 size)
{
	std::string error;
	return Get(source, size, error);
}


size_t BitmapCache::Count()
{
	std::lock_guard guard(sLock);
	return sEntries.size();
}


} // HaikuReact
