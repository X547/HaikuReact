#include "ReactScrollViewport.h"

#include <algorithm>

#include <LayoutUtils.h>
#include <ScrollBar.h>


namespace HaikuReact {


// Smallest visible extent in a scrolling direction.
constexpr float kMinVisibleSize = 32;
constexpr float kScrollStep = 20;


static void UpdateScrollBar(BScrollBar *scrollBar, float visible, float total)
{
	if (scrollBar == nullptr) {
		return;
	}
	scrollBar->SetRange(0, std::max(0.0f, total - visible));
	scrollBar->SetProportion(total > 0 ? std::min(1.0f, visible / total) : 1.0f);
	scrollBar->SetSteps(kScrollStep, std::max(kScrollStep, visible - kScrollStep));
}


// Extent of the content along one axis: fill the visible area, grow to the
// preferred size when scrolling (or to the minimum when not), and never
// exceed the maximum unless the visible area requires it.
static float ContentExtent(float visible, bool scrolling, float min, float max, float preferred)
{
	float extent = std::max(visible, scrolling ? std::max(min, preferred) : min);
	if (extent > max) {
		extent = std::max(max, scrolling ? std::max(min, preferred) : min);
	}
	return extent;
}


ReactScrollViewport::ReactScrollViewport(bool horizontal, bool vertical):
	BView("ScrollViewport", B_FRAME_EVENTS),
	fHorizontal(horizontal),
	fVertical(vertical)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
}


void ReactScrollViewport::SetContent(BView *content)
{
	if (fContent == content) {
		return;
	}
	if (fContent != nullptr) {
		RemoveChild(fContent);
	}
	fContent = content;
	if (fContent != nullptr) {
		AddChild(fContent);
	}
	InvalidateLayout();
}


BSize ReactScrollViewport::MinSize()
{
	BSize size(kMinVisibleSize, kMinVisibleSize);
	if (fContent != nullptr) {
		const BSize contentMin = fContent->MinSize();
		if (!fHorizontal) {
			size.width = contentMin.width;
		}
		if (!fVertical) {
			size.height = contentMin.height;
		}
	}
	return BLayoutUtils::ComposeSize(ExplicitMinSize(), size);
}


BSize ReactScrollViewport::MaxSize()
{
	BSize size(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED);
	if (fContent != nullptr) {
		const BSize contentMax = fContent->MaxSize();
		if (!fHorizontal) {
			size.width = contentMax.width;
		}
		if (!fVertical) {
			size.height = contentMax.height;
		}
	}
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(), size);
}


BSize ReactScrollViewport::PreferredSize()
{
	const BSize size = fContent != nullptr ? fContent->PreferredSize() : BSize(kMinVisibleSize, kMinVisibleSize);
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(), size);
}


void ReactScrollViewport::DoLayout()
{
	LayoutContent();
}


void ReactScrollViewport::FrameResized(float newWidth, float newHeight)
{
	BView::FrameResized(newWidth, newHeight);
	// A layout pass of the BScrollView can resize the viewport without
	// laying it out again; this notification always has the final size.
	LayoutContent();
}


void ReactScrollViewport::LayoutContent()
{
	if (fContent == nullptr) {
		UpdateScrollBars(BSize(0, 0));
		return;
	}

	const BRect bounds = Bounds();
	const BSize min = fContent->MinSize();
	const BSize max = fContent->MaxSize();
	const BSize preferred = fContent->PreferredSize();
	const BSize size(
		ContentExtent(bounds.Width(), fHorizontal, min.width, max.width, preferred.width),
		ContentExtent(bounds.Height(), fVertical, min.height, max.height, preferred.height)
	);

	fContent->MoveTo(0, 0);
	fContent->ResizeTo(size.width, size.height);
	UpdateScrollBars(size);
}


void ReactScrollViewport::UpdateScrollBars(BSize contentSize)
{
	const BRect bounds = Bounds();
	UpdateScrollBar(ScrollBar(B_HORIZONTAL), bounds.Width(), contentSize.width);
	UpdateScrollBar(ScrollBar(B_VERTICAL), bounds.Height(), contentSize.height);
}


} // HaikuReact
