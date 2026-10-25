#include "ReactMenuItem.h"

#include <math.h>

#include <algorithm>

#include <ControlLook.h>
#include <Menu.h>
#include <Window.h>

#include "ReactIconView.h"


namespace HaikuReact {


void RefreshMenu(BMenu *menu)
{
	if (menu == nullptr) {
		return;
	}
	menu->InvalidateLayout();
	// An open menu's window is not locked by the JS thread and is not
	// locked from it (see Phase 0 of docs/plans/images-lists-menus.md); it
	// redraws when it is shown again.
	BWindow *window = menu->Window();
	if (window != nullptr && window->IsLocked()) {
		menu->Invalidate();
	}
}


ReactMenuItem::ReactMenuItem(const char *label, BMessage *message):
	BMenuItem(label, message),
	fIconSize(ScaledIconSize(16))
{
}


ReactMenuItem::ReactMenuItem(BMenu *submenu):
	BMenuItem(submenu),
	fIconSize(ScaledIconSize(16))
{
}


SharedBitmapRef ReactMenuItem::CurrentIcon() const
{
	std::lock_guard guard(fIconLock);
	return fIcon;
}


bool ReactMenuItem::HasIcon() const
{
	return CurrentIcon() != nullptr;
}


void ReactMenuItem::Reload()
{
	SharedBitmapRef icon = BitmapCache::Get(fSource, fIconSize);
	{
		std::lock_guard guard(fIconLock);
		std::swap(fIcon, icon);
	}
	// The previous icon is released outside the lock.
	icon.reset();
	RefreshMenu(Menu());
}


void ReactMenuItem::SetIcon(const IconSource &source)
{
	if (source == fSource) {
		return;
	}
	fSource = source;
	Reload();
}


void ReactMenuItem::SetIconSize(int32 size)
{
	if (size == fIconSize) {
		return;
	}
	fIconSize = size;
	if (!fSource.IsNone()) {
		Reload();
	}
}


void ReactMenuItem::GetContentSize(float *_width, float *_height)
{
	if (CurrentIcon() == nullptr) {
		BMenuItem::GetContentSize(_width, _height);
		return;
	}
	float labelWidth = 0;
	float fontHeight = 0;
	BMenuItem::GetContentSize(&labelWidth, &fontHeight);
	float width = fIconSize;
	if (labelWidth > 0) {
		width += be_control_look->DefaultLabelSpacing() + labelWidth;
	}
	if (_width != nullptr) {
		*_width = width;
	}
	if (_height != nullptr) {
		*_height = std::max(fontHeight, static_cast<float>(fIconSize));
	}
}


void ReactMenuItem::DrawContent()
{
	const SharedBitmapRef icon = CurrentIcon();
	if (icon == nullptr) {
		BMenuItem::DrawContent();
		return;
	}
	BMenu &menu = *Menu();
	float labelWidth = 0;
	float fontHeight = 0;
	BMenuItem::GetContentSize(&labelWidth, &fontHeight);
	const float height = std::max(fontHeight, static_cast<float>(fIconSize));

	const BPoint location = menu.PenLocation();
	BRect iconRect(0, 0, fIconSize - 1, fIconSize - 1);
	iconRect.OffsetTo(location.x, location.y + floorf((height - fIconSize) / 2));
	const BBitmap *bitmap = IsEnabled() || icon->Dimmed() == nullptr ? icon->Bitmap() : icon->Dimmed();
	DrawIconBitmap(menu, *bitmap, iconRect);

	if (labelWidth > 0) {
		menu.MovePenTo(location.x + fIconSize + be_control_look->DefaultLabelSpacing(),
			location.y + floorf((height - fontHeight) / 2));
		BMenuItem::DrawContent();
	}
}


} // HaikuReact
