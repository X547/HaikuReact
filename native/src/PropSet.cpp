#include "PropSet.h"

#include <Size.h>


namespace HaikuReact {


template<typename T>
struct NamedValue {
	const char *name;
	T value;
};

static constexpr NamedValue<float> kSpacingNames[] = {
	{"default", B_USE_DEFAULT_SPACING},
	{"item",    B_USE_ITEM_SPACING},
	{"half",    B_USE_HALF_ITEM_SPACING},
	{"window",  B_USE_WINDOW_SPACING},
	{"small",   B_USE_SMALL_SPACING},
	{"corner",  B_USE_CORNER_SPACING},
	{"big",     B_USE_BIG_SPACING},
	{"border",  B_USE_BORDER_SPACING},
};

static constexpr NamedValue<orientation> kOrientationNames[] = {
	{"horizontal", B_HORIZONTAL},
	{"vertical",   B_VERTICAL},
};

static constexpr NamedValue<alignment> kAlignmentNames[] = {
	{"left",   B_ALIGN_LEFT},
	{"center", B_ALIGN_HORIZONTAL_CENTER},
	{"right",  B_ALIGN_RIGHT},
	{"fill",   B_ALIGN_USE_FULL_WIDTH},
};

static constexpr NamedValue<vertical_alignment> kVerticalAlignmentNames[] = {
	{"top",    B_ALIGN_TOP},
	{"middle", B_ALIGN_VERTICAL_CENTER},
	{"bottom", B_ALIGN_BOTTOM},
	{"fill",   B_ALIGN_USE_FULL_HEIGHT},
};

static constexpr NamedValue<border_style> kBorderNames[] = {
	{"fancy", B_FANCY_BORDER},
	{"plain", B_PLAIN_BORDER},
	{"none",  B_NO_BORDER},
};


template<typename T, size_t N>
static bool ParseName(const Napi::Value &value, const NamedValue<T> (&names)[N], T &result)
{
	if (!value.IsString()) {
		return false;
	}
	const std::string name = value.As<Napi::String>().Utf8Value();
	for (const NamedValue<T> &item : names) {
		if (name == item.name) {
			result = item.value;
			return true;
		}
	}
	return false;
}


static bool ParseSpacing(const Napi::Value &value, float &result)
{
	if (value.IsNumber()) {
		result = value.As<Napi::Number>().FloatValue();
		return true;
	}
	return ParseName(value, kSpacingNames, result);
}


PropSet::PropSet(Napi::Object props):
	fProps(props)
{
}


void PropSet::ThrowTypeError(const char *key, const char *expected) const
{
	throw Napi::TypeError::New(fProps.Env(), std::string("prop \"") + key + "\": expected " + expected);
}


void PropSet::Convert(const char *key, const Napi::Value &value, std::string &result) const
{
	if (!value.IsString()) {
		ThrowTypeError(key, "a string");
	}
	result = value.As<Napi::String>().Utf8Value();
}


void PropSet::Convert(const char *key, const Napi::Value &value, bool &result) const
{
	if (!value.IsBoolean()) {
		ThrowTypeError(key, "a boolean");
	}
	result = value.As<Napi::Boolean>().Value();
}


void PropSet::Convert(const char *key, const Napi::Value &value, int32 &result) const
{
	if (!value.IsNumber()) {
		ThrowTypeError(key, "a number");
	}
	result = value.As<Napi::Number>().Int32Value();
}


void PropSet::Convert(const char *key, const Napi::Value &value, float &result) const
{
	if (!value.IsNumber()) {
		ThrowTypeError(key, "a number");
	}
	result = value.As<Napi::Number>().FloatValue();
}


void PropSet::Convert(const char *key, const Napi::Value &value, std::vector<float> &result) const
{
	if (!value.IsArray()) {
		ThrowTypeError(key, "an array of numbers");
	}
	const Napi::Array array = value.As<Napi::Array>();
	result.clear();
	for (uint32 i = 0; i < array.Length(); i++) {
		const Napi::Value item = array.Get(i);
		if (!item.IsNumber()) {
			ThrowTypeError(key, "an array of numbers");
		}
		result.push_back(item.As<Napi::Number>().FloatValue());
	}
}


void PropSet::Convert(const char *key, const Napi::Value &value, std::vector<int32> &result) const
{
	if (!value.IsArray()) {
		ThrowTypeError(key, "an array of numbers");
	}
	const Napi::Array array = value.As<Napi::Array>();
	result.clear();
	for (uint32 i = 0; i < array.Length(); i++) {
		const Napi::Value item = array.Get(i);
		if (!item.IsNumber()) {
			ThrowTypeError(key, "an array of numbers");
		}
		result.push_back(item.As<Napi::Number>().Int32Value());
	}
}


void PropSet::Convert(const char *key, const Napi::Value &value, std::vector<std::string> &result) const
{
	if (!value.IsArray()) {
		ThrowTypeError(key, "an array of strings");
	}
	const Napi::Array array = value.As<Napi::Array>();
	result.clear();
	for (uint32 i = 0; i < array.Length(); i++) {
		const Napi::Value item = array.Get(i);
		if (!item.IsString()) {
			ThrowTypeError(key, "an array of strings");
		}
		result.push_back(item.As<Napi::String>().Utf8Value());
	}
}


void PropSet::Convert(const char *key, const Napi::Value &value, Spacing &result) const
{
	if (!ParseSpacing(value, result.value)) {
		ThrowTypeError(key, "a number or a spacing name");
	}
}


void PropSet::Convert(const char *key, const Napi::Value &value, Insets &result) const
{
	if (!value.IsArray()) {
		float inset = 0;
		if (!ParseSpacing(value, inset)) {
			ThrowTypeError(key, "a spacing value or an array of 2 or 4 spacing values");
		}
		result = {inset, inset, inset, inset};
		return;
	}

	const Napi::Array array = value.As<Napi::Array>();
	const uint32 length = array.Length();
	if (length != 2 && length != 4) {
		ThrowTypeError(key, "an array of 2 or 4 spacing values");
	}
	float items[4] {};
	for (uint32 i = 0; i < length; i++) {
		if (!ParseSpacing(array.Get(i), items[i])) {
			ThrowTypeError(key, "an array of spacing values");
		}
	}
	if (length == 2) {
		result = {items[0], items[1], items[0], items[1]};
		return;
	}
	result = {items[0], items[1], items[2], items[3]};
}


void PropSet::Convert(const char *key, const Napi::Value &value, SizeValue &result) const
{
	if (value.IsNumber()) {
		// Layout sizes count like BRect::Width(): one less than the pixels.
		result.value = value.As<Napi::Number>().FloatValue() - 1;
		return;
	}
	if (value.IsString() && value.As<Napi::String>().Utf8Value() == "unlimited") {
		result.value = B_SIZE_UNLIMITED;
		return;
	}
	ThrowTypeError(key, "a number or \"unlimited\"");
}


void PropSet::Convert(const char *key, const Napi::Value &value, orientation &result) const
{
	if (!ParseName(value, kOrientationNames, result)) {
		ThrowTypeError(key, "\"horizontal\" or \"vertical\"");
	}
}


void PropSet::Convert(const char *key, const Napi::Value &value, alignment &result) const
{
	if (!ParseName(value, kAlignmentNames, result)) {
		ThrowTypeError(key, "\"left\", \"center\", \"right\" or \"fill\"");
	}
}


void PropSet::Convert(const char *key, const Napi::Value &value, vertical_alignment &result) const
{
	if (!ParseName(value, kVerticalAlignmentNames, result)) {
		ThrowTypeError(key, "\"top\", \"middle\", \"bottom\" or \"fill\"");
	}
}


void PropSet::Convert(const char *key, const Napi::Value &value, border_style &result) const
{
	if (!ParseName(value, kBorderNames, result)) {
		ThrowTypeError(key, "\"fancy\", \"plain\" or \"none\"");
	}
}


} // HaikuReact
