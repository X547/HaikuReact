#include "IconSource.h"

#include <string.h>

#include <mutex>

#include <Application.h>
#include <Bitmap.h>
#include <ControlLook.h>
#include <DataIO.h>
#include <Entry.h>
#include <File.h>
#include <IconUtils.h>
#include <MimeType.h>
#include <NodeInfo.h>
#include <Resources.h>
#include <Roster.h>
#include <TranslationUtils.h>


namespace HaikuReact {


static constexpr const char *kKindNames[] = {"", "system", "mimeType", "file", "app", "path", "data"};


std::string IconSource::Key() const
{
	std::string key = kKindNames[static_cast<int>(kind)];
	key += ':';
	if (kind == Kind::Data && data != nullptr) {
		key.append(reinterpret_cast<const char*>(data->data()), data->size());
	} else {
		key += name;
	}
	return key;
}


bool IconSource::operator==(const IconSource &other) const
{
	if (kind != other.kind || name != other.name) {
		return false;
	}
	if (data == other.data) {
		return true;
	}
	return data != nullptr && other.data != nullptr && *data == *other.data;
}


int32 ScaledIconSize(float size)
{
	return be_control_look->ComposeIconSize(static_cast<int32>(size)).IntegerWidth() + 1;
}


static std::unique_ptr<BBitmap> NewIconBitmap(int32 size)
{
	auto bitmap = std::make_unique<BBitmap>(BRect(0, 0, size - 1, size - 1), B_RGBA32);
	if (bitmap->InitCheck() != B_OK) {
		return nullptr;
	}
	return bitmap;
}


static std::unique_ptr<BBitmap> VectorIcon(const void *data, size_t size, int32 iconSize)
{
	std::unique_ptr<BBitmap> bitmap = NewIconBitmap(iconSize);
	if (bitmap == nullptr
		|| BIconUtils::GetVectorIcon(static_cast<const uint8*>(data), size, bitmap.get()) != B_OK) {
		return nullptr;
	}
	return bitmap;
}


// BIconUtils::GetSystemIcon() deletes the caller's bitmap for unknown names,
// so the resources of libbe are read directly.
static std::unique_ptr<BBitmap> SystemIcon(const std::string &name, int32 size)
{
	static std::mutex lock;
	static BResources resources;
	static bool initialized = false;

	std::lock_guard guard(lock);
	if (!initialized) {
		// be_app_messenger lives in libbe, so this opens libbe's resources.
		resources.SetToImage(&be_app_messenger);
		initialized = true;
	}
	size_t dataSize = 0;
	const void *data = resources.LoadResource(B_VECTOR_ICON_TYPE, name.c_str(), &dataSize);
	if (data == nullptr) {
		return nullptr;
	}
	return VectorIcon(data, dataSize, size);
}


static std::unique_ptr<BBitmap> MimeTypeIcon(const std::string &type, int32 size)
{
	BMimeType mimeType(type.c_str());
	uint8 *data = nullptr;
	size_t dataSize = 0;
	status_t status = mimeType.GetIcon(&data, &dataSize);
	if (status != B_OK) {
		// Unknown subtypes show their supertype's icon, as in Tracker.
		BMimeType superType;
		if (mimeType.GetSupertype(&superType) == B_OK) {
			status = superType.GetIcon(&data, &dataSize);
		}
	}
	std::unique_ptr<BBitmap> bitmap;
	if (status == B_OK) {
		bitmap = VectorIcon(data, dataSize, size);
	}
	delete[] data;
	return bitmap;
}


static std::unique_ptr<BBitmap> TrackerIcon(const entry_ref &ref, int32 size)
{
	std::unique_ptr<BBitmap> bitmap = NewIconBitmap(size);
	if (bitmap == nullptr || BNodeInfo::GetTrackerIcon(&ref, bitmap.get(), static_cast<icon_size>(size)) != B_OK) {
		return nullptr;
	}
	return bitmap;
}


static bool ReadFile(const std::string &path, std::vector<uint8> &bytes, std::string &error)
{
	BFile file(path.c_str(), B_READ_ONLY);
	off_t size = 0;
	status_t status = file.InitCheck();
	if (status == B_OK) {
		status = file.GetSize(&size);
	}
	if (status == B_OK) {
		bytes.resize(size);
		const ssize_t read = file.ReadAt(0, bytes.data(), bytes.size());
		status = read < 0 ? static_cast<status_t>(read) : read != size ? B_IO_ERROR : B_OK;
	}
	if (status != B_OK) {
		error = path + ": " + strerror(status);
		return false;
	}
	return true;
}


// Translators return various color spaces; drawing with alpha needs RGBA32.
static std::unique_ptr<BBitmap> ToRgba32(std::unique_ptr<BBitmap> bitmap)
{
	if (bitmap->ColorSpace() == B_RGBA32) {
		return bitmap;
	}
	auto converted = std::make_unique<BBitmap>(bitmap->Bounds(), B_RGBA32);
	if (converted->InitCheck() != B_OK || converted->ImportBits(bitmap.get()) != B_OK) {
		return nullptr;
	}
	if (bitmap->ColorSpace() == B_RGB32) {
		// The fourth byte of B_RGB32 is undefined, not alpha.
		auto *bits = static_cast<uint8*>(converted->Bits());
		for (size_t i = 3; i < static_cast<size_t>(converted->BitsLength()); i += 4) {
			bits[i] = 255;
		}
	}
	return converted;
}


static std::unique_ptr<BBitmap> DecodeRaster(const std::vector<uint8> &bytes, std::string &error)
{
	BMemoryIO input(bytes.data(), bytes.size());
	std::unique_ptr<BBitmap> bitmap(BTranslationUtils::GetBitmap(&input));
	if (bitmap == nullptr || !bitmap->IsValid()) {
		error = "unsupported or damaged image data";
		return nullptr;
	}
	bitmap = ToRgba32(std::move(bitmap));
	if (bitmap == nullptr) {
		error = "failed to convert the image";
	}
	return bitmap;
}


static const std::vector<uint8> *SourceBytes(const IconSource &source, std::vector<uint8> &storage, std::string &error)
{
	if (source.kind == IconSource::Kind::Data) {
		return source.data.get();
	}
	if (!ReadFile(source.name, storage, error)) {
		return nullptr;
	}
	return &storage;
}


std::unique_ptr<BBitmap> LoadIcon(const IconSource &source, int32 size, std::string &error)
{
	std::unique_ptr<BBitmap> bitmap;
	switch (source.kind) {
		case IconSource::Kind::None:
			error = "no icon source";
			return nullptr;
		case IconSource::Kind::System:
			bitmap = SystemIcon(source.name, size);
			break;
		case IconSource::Kind::MimeType:
			bitmap = MimeTypeIcon(source.name, size);
			break;
		case IconSource::Kind::File: {
			entry_ref ref;
			if (get_ref_for_path(source.name.c_str(), &ref) == B_OK) {
				bitmap = TrackerIcon(ref, size);
			}
			break;
		}
		case IconSource::Kind::App: {
			entry_ref ref;
			if (be_roster->FindApp(source.name.c_str(), &ref) == B_OK) {
				bitmap = TrackerIcon(ref, size);
			}
			break;
		}
		case IconSource::Kind::Path:
		case IconSource::Kind::Data: {
			std::vector<uint8> storage;
			const std::vector<uint8> *bytes = SourceBytes(source, storage, error);
			if (bytes == nullptr) {
				return nullptr;
			}
			bitmap = VectorIcon(bytes->data(), bytes->size(), size);
			if (bitmap == nullptr) {
				bitmap = DecodeRaster(*bytes, error);
			}
			return bitmap;
		}
	}
	if (bitmap == nullptr) {
		error = "no icon for " + source.Key();
	}
	return bitmap;
}


std::unique_ptr<BBitmap> LoadImage(const IconSource &source, std::string &error)
{
	if (source.kind != IconSource::Kind::Path && source.kind != IconSource::Kind::Data) {
		error = "images load from {path} or {data}; use <Icon> for other sources";
		return nullptr;
	}
	std::vector<uint8> storage;
	const std::vector<uint8> *bytes = SourceBytes(source, storage, error);
	if (bytes == nullptr) {
		return nullptr;
	}
	return DecodeRaster(*bytes, error);
}


} // HaikuReact
