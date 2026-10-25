#include "IconNode.h"

#include <Bitmap.h>

#include "../BitmapCache.h"
#include "../PropSet.h"
#include "../ReactIconView.h"


namespace HaikuReact {


Napi::Value DumpBitmap(Napi::Env env, const BBitmap *bitmap)
{
	if (bitmap == nullptr) {
		return env.Null();
	}
	int32 opaque = 0;
	if (bitmap->ColorSpace() == B_RGBA32) {
		const auto *bits = static_cast<const uint8*>(bitmap->Bits());
		for (size_t i = 3; i < static_cast<size_t>(bitmap->BitsLength()); i += 4) {
			if (bits[i] != 0) {
				opaque++;
			}
		}
	}
	Napi::Object out = Napi::Object::New(env);
	out.Set("width", bitmap->Bounds().IntegerWidth() + 1);
	out.Set("height", bitmap->Bounds().IntegerHeight() + 1);
	out.Set("opaquePixels", opaque);
	return out;
}


IconNode::IconNode(uint32 id):
	ViewNode(id, new ReactIconView()),
	fIconView(static_cast<ReactIconView*>(View()))
{
	Reload();
}


void IconNode::Reload()
{
	const int32 size = ScaledIconSize(fSize);
	fError.clear();
	fIconView->SetIcon(BitmapCache::Get(fSource, size, fError), size);
}


void IconNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	bool changed = false;
	if (auto source = props.Get<IconSource>("source", {}); source && !(*source == fSource)) {
		fSource = *source;
		changed = true;
	}
	if (auto size = props.Get<float>("size", 16); size && *size != fSize) {
		fSize = *size;
		changed = true;
	}
	if (changed) {
		Reload();
	}
}


void IconNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);
	Napi::Env env = out.Env();
	out.Set("size", fIconView->IconSize());
	out.Set("bitmap", DumpBitmap(env, fIconView->Icon() != nullptr ? fIconView->Icon()->Bitmap() : nullptr));
	out.Set("error", fError);
}


} // HaikuReact
