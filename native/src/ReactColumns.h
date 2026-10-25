#pragma once

#include <interface/ColumnListView.h>
#include <interface/ColumnTypes.h>

#include <napi.h>

#include "BitmapCache.h"
#include "ColumnSpec.h"


namespace HaikuReact {


// Field of a cell the row does not set. BRow validates its fields against
// the columns, so every column gets one.
class EmptyField: public BField {
};


class IconStringField: public BStringField {
private:
	SharedBitmapRef fIcon;

public:
	IconStringField(const char *string, SharedBitmapRef icon);

	const BBitmap *Icon() const {return fIcon != nullptr ? fIcon->Bitmap() : nullptr;}
};


// BBitmapField with a shared icon it keeps alive.
class SharedBitmapField: public BBitmapField {
private:
	SharedBitmapRef fIcon;

public:
	explicit SharedBitmapField(SharedBitmapRef icon);
};


// Creates the column for a spec; logical field indices are spec positions.
BColumn *CreateColumn(const ColumnSpec &spec);
// Creates the field showing a cell in a column of the given type.
BField *CreateField(ColumnType type, const Cell *cell);
// The cell's value as shown, for dumps.
Napi::Value FieldToJs(Napi::Env env, ColumnType type, BField *field);


} // HaikuReact
