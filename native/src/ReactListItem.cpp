#include "ReactListItem.h"

#include <math.h>

#include <algorithm>

#include <ControlLook.h>
#include <View.h>

#include "IconSource.h"
#include "ReactIconView.h"


namespace HaikuReact {


ReactListItem::ReactListItem(uint32 nodeId, bool placeholder):
	BListItem(0, false),
	fNodeId(nodeId),
	fPlaceholder(placeholder),
	fIconSize(ScaledIconSize(16))
{
}


void ReactListItem::DrawItem(BView *owner, BRect frame, bool complete)
{
	const rgb_color lowColor = owner->LowColor();
	const rgb_color highColor = owner->HighColor();

	const rgb_color background = IsSelected() ? ui_color(B_LIST_SELECTED_BACKGROUND_COLOR) : owner->ViewColor();
	if (IsSelected() || complete) {
		owner->SetLowColor(background);
		owner->FillRect(frame, B_SOLID_LOW);
	} else {
		owner->SetLowColor(background);
	}
	rgb_color textColor = ui_color(IsSelected() ? B_LIST_SELECTED_ITEM_TEXT_COLOR : B_LIST_ITEM_TEXT_COLOR);
	if (fPlaceholder) {
		textColor = disable_color(textColor, background);
	}
	owner->SetHighColor(textColor);

	float x = frame.left + be_control_look->DefaultLabelSpacing();
	if (fIcon != nullptr) {
		BRect iconRect(0, 0, fIconSize - 1, fIconSize - 1);
		iconRect.OffsetTo(x, frame.top + floorf((frame.Height() + 1 - fIconSize) / 2));
		DrawIconBitmap(*owner, *fIcon->Bitmap(), iconRect);
		x += fIconSize + be_control_look->DefaultLabelSpacing();
	}
	owner->SetDrawingMode(B_OP_OVER);
	owner->DrawString(fLabel.c_str(), BPoint(x, frame.top + fBaselineOffset));
	owner->SetDrawingMode(B_OP_COPY);

	owner->SetLowColor(lowColor);
	owner->SetHighColor(highColor);
}


void ReactListItem::Update(BView *owner, const BFont *font)
{
	(void)owner;
	font_height fontHeight;
	font->GetHeight(&fontHeight);
	const float textHeight = ceilf(fontHeight.ascent) + ceilf(fontHeight.descent) + ceilf(fontHeight.leading);
	const float spacing = be_control_look->DefaultLabelSpacing();

	float width = spacing + font->StringWidth(fLabel.c_str());
	float height = textHeight + 4;
	if (fIcon != nullptr) {
		width += fIconSize + spacing;
		height = std::max(height, static_cast<float>(fIconSize) + 4);
	}
	SetWidth(width);
	SetHeight(height);
	// Text centered vertically, as BStringItem places it for text alone.
	fBaselineOffset = floorf((height - textHeight) / 2) + ceilf(fontHeight.ascent + fontHeight.leading / 2);
}


} // HaikuReact
