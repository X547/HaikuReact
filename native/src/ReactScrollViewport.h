#pragma once

#include <View.h>


namespace HaikuReact {


// Target of the BScrollView behind <Scroll>. BScrollView resizes its target
// to the visible area, which squeezes a layout-driven view instead of
// scrolling it; the viewport gives its content view the size the content
// needs and scrolls over it.
class ReactScrollViewport: public BView {
private:
	BView *fContent = nullptr;
	const bool fHorizontal;
	const bool fVertical;

	void LayoutContent();
	void UpdateScrollBars(BSize contentSize);

public:
	ReactScrollViewport(bool horizontal, bool vertical);

	BView *Content() const {return fContent;}
	// Replaces the content view; the old one is removed but not deleted.
	void SetContent(BView *content);

	BSize MinSize() final;
	BSize MaxSize() final;
	BSize PreferredSize() final;
	void DoLayout() final;
	void FrameResized(float newWidth, float newHeight) final;
};


} // HaikuReact
