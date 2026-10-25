#include "ReactIconView.h"

#include <math.h>

#include <Bitmap.h>
#include <LayoutUtils.h>


namespace HaikuReact {


void DrawIconBitmap(BView &view, const BBitmap &bitmap, BRect rect)
{
	view.PushState();
	view.SetDrawingMode(B_OP_ALPHA);
	view.SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	view.DrawBitmap(&bitmap, bitmap.Bounds(), rect, B_FILTER_BITMAP_BILINEAR);
	view.PopState();
}


ReactIconView::ReactIconView():
	BView("Icon", B_WILL_DRAW)
{
}


void ReactIconView::SetIcon(SharedBitmapRef icon, int32 size)
{
	const bool resized = size != fSize;
	fIcon = std::move(icon);
	fSize = size;
	if (resized) {
		InvalidateLayout();
	}
	Invalidate();
}


void ReactIconView::AttachedToWindow()
{
	BView::AttachedToWindow();
	AdoptParentColors();
}


void ReactIconView::Draw(BRect updateRect)
{
	(void)updateRect;
	if (fIcon == nullptr) {
		return;
	}
	// Centered when the layout made the view larger than the icon.
	const BRect bounds = Bounds();
	BRect rect(0, 0, fSize - 1, fSize - 1);
	rect.OffsetTo(floorf(bounds.left + (bounds.Width() - rect.Width()) / 2),
		floorf(bounds.top + (bounds.Height() - rect.Height()) / 2));
	DrawIconBitmap(*this, *fIcon->Bitmap(), rect);
}


BSize ReactIconView::MinSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMinSize(), BSize(fSize - 1, fSize - 1));
}


BSize ReactIconView::MaxSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(), BSize(fSize - 1, fSize - 1));
}


BSize ReactIconView::PreferredSize()
{
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(), BSize(fSize - 1, fSize - 1));
}


} // HaikuReact
