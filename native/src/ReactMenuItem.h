#pragma once

#include <mutex>

#include <MenuItem.h>

#include "BitmapCache.h"
#include "IconSource.h"


namespace HaikuReact {


// Menu item that draws an optional icon before its label; an item without a
// label shows only the icon (e.g. in a menu bar). Used for <MenuItem>, the
// superitems of <Menu> and the items of <MenuField>.
class ReactMenuItem: public BMenuItem {
private:
	IconSource fSource;
	int32 fIconSize;
	// Swapped on the JS thread while a menu window may be drawing.
	mutable std::mutex fIconLock;
	SharedBitmapRef fIcon;

	void Reload();
	SharedBitmapRef CurrentIcon() const;

public:
	ReactMenuItem(const char *label, BMessage *message);
	explicit ReactMenuItem(BMenu *submenu);

	const IconSource &Source() const {return fSource;}
	int32 IconSize() const {return fIconSize;}
	bool HasIcon() const;
	void SetIcon(const IconSource &source);
	// In pixels; set by the menu the item belongs to.
	void SetIconSize(int32 size);
	// Size of icon and label; only while the item is in a menu.
	void ContentSize(float *width, float *height) {GetContentSize(width, height);}

protected:
	void GetContentSize(float *_width, float *_height) override;
	void DrawContent() override;
};


// Relayouts a menu after one of its items changed size; redraws it when
// its window is locked by the caller.
void RefreshMenu(BMenu *menu);


} // HaikuReact
