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

static constexpr NamedValue<IconSource::Kind> kIconSourceKeys[] = {
	{"system",   IconSource::Kind::System},
	{"mimeType", IconSource::Kind::MimeType},
	{"file",     IconSource::Kind::File},
	{"app",      IconSource::Kind::App},
	{"path",     IconSource::Kind::Path},
};

static constexpr NamedValue<ColumnType> kColumnTypeNames[] = {
	{"text",    ColumnType::Text},
	{"integer", ColumnType::Integer},
	{"size",    ColumnType::Size},
	{"date",    ColumnType::Date},
	{"icon",    ColumnType::Icon},
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


void PropSet::Convert(const char *key, const Napi::Value &value, IconSource &result) const
{
	result = ParseIconSource(key, value);
}


void PropSet::Convert(const char *key, const Napi::Value &value, std::vector<ColumnSpec> &result) const
{
	const char *expected = "an array of {id, title, type, width, minWidth, maxWidth, align} objects";
	if (!value.IsArray()) {
		ThrowTypeError(key, expected);
	}
	const Napi::Array array = value.As<Napi::Array>();
	result.clear();
	for (uint32 i = 0; i < array.Length(); i++) {
		const Napi::Value item = array.Get(i);
		if (!item.IsObject()) {
			ThrowTypeError(key, expected);
		}
		const Napi::Object object = item.As<Napi::Object>();
		ColumnSpec column;
		const Napi::Value id = object.Get("id");
		if (!id.IsString()) {
			ThrowTypeError(key, expected);
		}
		column.id = id.As<Napi::String>().Utf8Value();
		if (const Napi::Value title = object.Get("title"); title.IsString()) {
			column.title = title.As<Napi::String>().Utf8Value();
		}
		if (const Napi::Value type = object.Get("type"); !type.IsUndefined() && !ParseName(type, kColumnTypeNames, column.type)) {
			ThrowTypeError(key, "column types \"text\", \"integer\", \"size\", \"date\" or \"icon\"");
		}
		if (const Napi::Value align = object.Get("align"); !align.IsUndefined() && !ParseName(align, kAlignmentNames, column.align)) {
			ThrowTypeError(key, "column alignments \"left\", \"center\" or \"right\"");
		}
		for (const auto &[name, field] : {
			std::pair{"width", &column.width}, {"minWidth", &column.minWidth}, {"maxWidth", &column.maxWidth}
		}) {
			if (const Napi::Value number = object.Get(name); number.IsNumber()) {
				*field = number.As<Napi::Number>().FloatValue();
			}
		}
		result.push_back(std::move(column));
	}
}


static bool IsIconSourceObject(const Napi::Object &object)
{
	if (object.HasOwnProperty("data")) {
		return true;
	}
	for (const NamedValue<IconSource::Kind> &item : kIconSourceKeys) {
		if (object.HasOwnProperty(item.name)) {
			return true;
		}
	}
	return false;
}


void PropSet::Convert(const char *key, const Napi::Value &value, CellMap &result) const
{
	const char *expected = "an object of cells: strings, numbers, Dates, {text, icon} or icon sources";
	if (!value.IsObject()) {
		ThrowTypeError(key, expected);
	}
	const Napi::Object object = value.As<Napi::Object>();
	const Napi::Array names = object.GetPropertyNames();
	result.clear();
	for (uint32 i = 0; i < names.Length(); i++) {
		const std::string name = names.Get(i).ToString().Utf8Value();
		const Napi::Value item = object.Get(name);
		Cell cell;
		if (item.IsUndefined() || item.IsNull()) {
			continue;
		} else if (item.IsString()) {
			cell.text = item.As<Napi::String>().Utf8Value();
		} else if (item.IsNumber()) {
			cell.number = item.As<Napi::Number>().DoubleValue();
		} else if (item.IsDate()) {
			cell.number = item.As<Napi::Date>().ValueOf();
		} else if (item.IsObject() && IsIconSourceObject(item.As<Napi::Object>())) {
			cell.icon = ParseIconSource(key, item);
		} else if (item.IsObject()) {
			const Napi::Object cellObject = item.As<Napi::Object>();
			const Napi::Value text = cellObject.Get("text");
			if (!text.IsString()) {
				ThrowTypeError(key, expected);
			}
			cell.text = text.As<Napi::String>().Utf8Value();
			if (const Napi::Value icon = cellObject.Get("icon"); !icon.IsUndefined() && !icon.IsNull()) {
				cell.icon = ParseIconSource(key, icon);
			}
		} else {
			ThrowTypeError(key, expected);
		}
		result[name] = std::move(cell);
	}
}


void PropSet::Convert(const char *key, const Napi::Value &value, std::vector<SortKey> &result) const
{
	const char *expected = "an array of {column, direction} objects";
	if (!value.IsArray()) {
		ThrowTypeError(key, expected);
	}
	const Napi::Array array = value.As<Napi::Array>();
	result.clear();
	for (uint32 i = 0; i < array.Length(); i++) {
		const Napi::Value item = array.Get(i);
		if (!item.IsObject() || !item.As<Napi::Object>().Get("column").IsString()) {
			ThrowTypeError(key, expected);
		}
		const Napi::Object object = item.As<Napi::Object>();
		SortKey sortKey;
		sortKey.column = object.Get("column").As<Napi::String>().Utf8Value();
		const Napi::Value direction = object.Get("direction");
		sortKey.ascending = !direction.IsString() || direction.As<Napi::String>().Utf8Value() != "descending";
		result.push_back(std::move(sortKey));
	}
}


void PropSet::Convert(const char *key, const Napi::Value &value, IdList &result) const
{
	if (!value.IsArray()) {
		ThrowTypeError(key, "an array of ids or indices");
	}
	const Napi::Array array = value.As<Napi::Array>();
	result.ids.clear();
	for (uint32 i = 0; i < array.Length(); i++) {
		const Napi::Value item = array.Get(i);
		if (item.IsString()) {
			result.ids.push_back(item.As<Napi::String>().Utf8Value());
		} else if (item.IsNumber()) {
			result.ids.push_back(std::to_string(item.As<Napi::Number>().Int32Value()));
		} else {
			ThrowTypeError(key, "an array of ids or indices");
		}
	}
}


void PropSet::Convert(const char *key, const Napi::Value &value, std::vector<LabeledItem> &result) const
{
	if (!value.IsArray()) {
		ThrowTypeError(key, "an array of strings or {label, icon} objects");
	}
	const Napi::Array array = value.As<Napi::Array>();
	result.clear();
	for (uint32 i = 0; i < array.Length(); i++) {
		const Napi::Value item = array.Get(i);
		LabeledItem entry;
		if (item.IsString()) {
			entry.label = item.As<Napi::String>().Utf8Value();
		} else if (item.IsObject()) {
			const Napi::Object object = item.As<Napi::Object>();
			const Napi::Value label = object.Get("label");
			if (!label.IsString()) {
				ThrowTypeError(key, "an array of strings or {label, icon} objects");
			}
			entry.label = label.As<Napi::String>().Utf8Value();
			const Napi::Value icon = object.Get("icon");
			if (!icon.IsUndefined() && !icon.IsNull()) {
				entry.icon = ParseIconSource(key, icon);
			}
		} else {
			ThrowTypeError(key, "an array of strings or {label, icon} objects");
		}
		result.push_back(std::move(entry));
	}
}


static std::shared_ptr<const std::vector<uint8>> BinaryData(const Napi::Value &value)
{
	if (value.IsTypedArray()) {
		const Napi::TypedArray array = value.As<Napi::TypedArray>();
		const auto *bytes = static_cast<const uint8*>(array.ArrayBuffer().Data()) + array.ByteOffset();
		return std::make_shared<const std::vector<uint8>>(bytes, bytes + array.ByteLength());
	}
	if (value.IsArrayBuffer()) {
		Napi::ArrayBuffer buffer = value.As<Napi::ArrayBuffer>();
		const auto *bytes = static_cast<const uint8*>(buffer.Data());
		return std::make_shared<const std::vector<uint8>>(bytes, bytes + buffer.ByteLength());
	}
	return nullptr;
}


IconSource PropSet::ParseIconSource(const char *key, const Napi::Value &value)
{
	const auto fail = [&]() {
		throw Napi::TypeError::New(value.Env(), std::string("prop \"") + key
			+ "\": expected an icon source: {system}, {mimeType}, {file}, {app}, {path} or {data}");
	};
	if (!value.IsObject()) {
		fail();
	}
	const Napi::Object object = value.As<Napi::Object>();
	IconSource result;
	if (object.HasOwnProperty("data")) {
		result.kind = IconSource::Kind::Data;
		result.data = BinaryData(object.Get("data"));
		if (result.data == nullptr) {
			fail();
		}
		return result;
	}
	for (const NamedValue<IconSource::Kind> &item : kIconSourceKeys) {
		if (object.HasOwnProperty(item.name)) {
			const Napi::Value name = object.Get(item.name);
			if (!name.IsString()) {
				fail();
			}
			result.kind = item.value;
			result.name = name.As<Napi::String>().Utf8Value();
			return result;
		}
	}
	fail();
	return result;
}


} // HaikuReact
