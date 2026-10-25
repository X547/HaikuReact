#pragma once

#include <View.h>

#include "BitmapCache.h"


namespace HaikuReact {


// Draws a bitmap scaled into rect with its alpha channel; for icons in views,
// list items and menu items.
void DrawIconBitmap(BView &view, const BBitmap &bitmap, BRect rect);


// View backing <Icon>: a fixed-size square showing one icon.
class ReactIconView: public BView {
private:
	SharedBitmapRef fIcon;
	int32 fSize = 16;

public:
	ReactIconView();

	const SharedBitmapRef &Icon() const {return fIcon;}
	int32 IconSize() const {return fSize;}
	// size is in pixels.
	void SetIcon(SharedBitmapRef icon, int32 size);

	void AttachedToWindow() final;
	void Draw(BRect updateRect) final;

	BSize MinSize() final;
	BSize MaxSize() final;
	BSize PreferredSize() final;
};


} // HaikuReact
