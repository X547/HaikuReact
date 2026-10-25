#include "ReactColumns.h"

#include <math.h>
#include <stdio.h>

#include <ControlLook.h>
#include <View.h>

#include "IconSource.h"
#include "ReactIconView.h"


namespace HaikuReact {


static bool IsEmpty(const BField *field)
{
	return dynamic_cast<const EmptyField*>(field) != nullptr;
}


// Column that leaves cells with an EmptyField blank and sorts them first.
template<typename Base>
class ReactColumn: public Base {
public:
	using Base::Base;

	void DrawField(BField *field, BRect rect, BView *parent) override
	{
		if (!IsEmpty(field)) {
			Base::DrawField(field, rect, parent);
		}
	}

	int CompareFields(BField *first, BField *second) override
	{
		if (IsEmpty(first) || IsEmpty(second)) {
			return static_cast<int>(!IsEmpty(first)) - static_cast<int>(!IsEmpty(second));
		}
		return Base::CompareFields(first, second);
	}

	bool AcceptsField(const BField *field) const override
	{
		return IsEmpty(field) || Base::AcceptsField(field);
	}

	float GetPreferredWidth(BField *field, BView *parent) const override
	{
		return IsEmpty(field) ? 0 : Base::GetPreferredWidth(field, parent);
	}
};


// Text column whose cells may show an icon before the text.
class TextColumn: public ReactColumn<BStringColumn> {
private:
	int32 fIconSize = ScaledIconSize(16);

public:
	using ReactColumn<BStringColumn>::ReactColumn;

	void DrawField(BField *field, BRect rect, BView *parent) final
	{
		const auto *iconField = dynamic_cast<const IconStringField*>(field);
		if (iconField != nullptr && iconField->Icon() != nullptr) {
			const float spacing = be_control_look->DefaultLabelSpacing();
			BRect iconRect(0, 0, fIconSize - 1, fIconSize - 1);
			iconRect.OffsetTo(rect.left + spacing, rect.top + floorf((rect.Height() + 1 - fIconSize) / 2));
			DrawIconBitmap(*parent, *iconField->Icon(), iconRect);
			rect.left += spacing + fIconSize;
		}
		ReactColumn<BStringColumn>::DrawField(field, rect, parent);
	}

	float GetPreferredWidth(BField *field, BView *parent) const final
	{
		float width = ReactColumn<BStringColumn>::GetPreferredWidth(field, parent);
		const auto *iconField = dynamic_cast<const IconStringField*>(field);
		if (iconField != nullptr && iconField->Icon() != nullptr) {
			width += be_control_look->DefaultLabelSpacing() + fIconSize;
		}
		return width;
	}
};


IconStringField::IconStringField(const char *string, SharedBitmapRef icon):
	BStringField(string),
	fIcon(std::move(icon))
{
}


SharedBitmapField::SharedBitmapField(SharedBitmapRef icon):
	BBitmapField(const_cast<BBitmap*>(icon->Bitmap())),
	fIcon(std::move(icon))
{
}


BColumn *CreateColumn(const ColumnSpec &spec)
{
	const char *title = spec.title.c_str();
	BColumn *column = nullptr;
	switch (spec.type) {
		case ColumnType::Text:
			column = new TextColumn(title, spec.width, spec.minWidth, spec.maxWidth, B_TRUNCATE_END, spec.align);
			break;
		case ColumnType::Integer:
			column = new ReactColumn<BIntegerColumn>(title, spec.width, spec.minWidth, spec.maxWidth, spec.align);
			break;
		case ColumnType::Size:
			column = new ReactColumn<BSizeColumn>(title, spec.width, spec.minWidth, spec.maxWidth, spec.align);
			break;
		case ColumnType::Date:
			column = new ReactColumn<BDateColumn>(title, spec.width, spec.minWidth, spec.maxWidth, spec.align);
			break;
		case ColumnType::Icon:
			column = new ReactColumn<BBitmapColumn>(title, spec.width, spec.minWidth, spec.maxWidth, spec.align);
			break;
	}
	return column;
}


BField *CreateField(ColumnType type, const Cell *cell)
{
	if (cell == nullptr) {
		return new EmptyField();
	}
	switch (type) {
		case ColumnType::Text: {
			std::string text = cell->text;
			if (cell->number.has_value() && text.empty()) {
				char buffer[32];
				snprintf(buffer, sizeof(buffer), "%g", *cell->number);
				text = buffer;
			}
			return new IconStringField(text.c_str(), BitmapCache::Get(cell->icon, ScaledIconSize(16)));
		}
		case ColumnType::Integer:
			if (cell->number.has_value()) {
				return new BIntegerField(static_cast<int32>(*cell->number));
			}
			break;
		case ColumnType::Size:
			if (cell->number.has_value()) {
				return new BSizeField(static_cast<off_t>(*cell->number));
			}
			break;
		case ColumnType::Date:
			if (cell->number.has_value()) {
				// Dates arrive in milliseconds, like JS Date values.
				time_t seconds = static_cast<time_t>(floor(*cell->number / 1000));
				return new BDateField(&seconds);
			}
			break;
		case ColumnType::Icon:
			if (SharedBitmapRef icon = BitmapCache::Get(cell->icon, ScaledIconSize(16))) {
				return new SharedBitmapField(std::move(icon));
			}
			break;
	}
	return new EmptyField();
}


Napi::Value FieldToJs(Napi::Env env, ColumnType type, BField *field)
{
	if (field == nullptr || IsEmpty(field)) {
		return env.Null();
	}
	switch (type) {
		case ColumnType::Text: {
			auto *textField = static_cast<IconStringField*>(field);
			if (textField->Icon() != nullptr) {
				Napi::Object out = Napi::Object::New(env);
				out.Set("text", textField->String());
				out.Set("hasIcon", true);
				return out;
			}
			return Napi::String::New(env, textField->String());
		}
		case ColumnType::Integer:
			return Napi::Number::New(env, static_cast<BIntegerField*>(field)->Value());
		case ColumnType::Size:
			return Napi::Number::New(env, static_cast<double>(static_cast<BSizeField*>(field)->Size()));
		case ColumnType::Date:
			return Napi::Number::New(env, static_cast<double>(static_cast<BDateField*>(field)->UnixTime()) * 1000);
		case ColumnType::Icon:
			return Napi::Boolean::New(env, static_cast<BBitmapField*>(field)->Bitmap() != nullptr);
	}
	return env.Null();
}


} // HaikuReact
