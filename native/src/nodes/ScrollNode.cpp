#include "ScrollNode.h"

#include <stdexcept>
#include <string>

#include <ScrollBar.h>
#include <ScrollView.h>

#include "../PropSet.h"
#include "../ReactScrollViewport.h"


namespace HaikuReact {


static BScrollView *CreateScrollView(const PropSet &props)
{
	const bool horizontal = props.Get<bool>("horizontal", false).value_or(false);
	const bool vertical = props.Get<bool>("vertical", true).value_or(true);
	const border_style border = props.Get<border_style>("border", B_FANCY_BORDER).value_or(B_FANCY_BORDER);

	auto *viewport = new ReactScrollViewport(horizontal, vertical);
	return new BScrollView("Scroll", viewport, 0, horizontal, vertical, border);
}


ScrollNode::ScrollNode(uint32 id, const PropSet &props):
	ViewNode(id, CreateScrollView(props), kFillWidthMaxSize),
	fScrollView(static_cast<BScrollView*>(View())),
	fViewport(static_cast<ReactScrollViewport*>(fScrollView->Target()))
{
}


ScrollNode::~ScrollNode()
{
	// While a self-scrolling child is the target, the viewport is not part of
	// the view tree and belongs to the node.
	if (fOwnsViewport) {
		delete fViewport;
	}
}


void ScrollNode::AttachChild(Node &child, int32 index)
{
	if (index != 0 || !Children().empty()) {
		throw std::invalid_argument("<Scroll> can only contain a single child");
	}
	BView *view = child.View();
	if (view == nullptr) {
		throw std::invalid_argument(std::string("<Scroll> cannot contain <") + child.TypeName() + ">");
	}
	if (child.ScrollsItself()) {
		fScrollView->SetTarget(view);
		fOwnsViewport = true;
		return;
	}
	fViewport->SetContent(view);
}


void ScrollNode::DetachChild(Node &child)
{
	(void)child;
	if (fOwnsViewport) {
		fScrollView->SetTarget(fViewport);
		fOwnsViewport = false;
		return;
	}
	fViewport->SetContent(nullptr);
}


void ScrollNode::ApplyOwnProps(const PropSet &props)
{
	ViewNode::ApplyOwnProps(props);

	if (auto border = props.Get<border_style>("border", B_FANCY_BORDER)) {
		fScrollView->SetBorder(*border);
	}
}


void ScrollNode::Dump(Napi::Object &out) const
{
	ViewNode::Dump(out);

	Napi::Env env = out.Env();
	const BRect bounds = fViewport->Bounds();
	Napi::Object viewport = Napi::Object::New(env);
	viewport.Set("x", bounds.left);
	viewport.Set("y", bounds.top);
	viewport.Set("width", bounds.Width() + 1);
	viewport.Set("height", bounds.Height() + 1);
	out.Set("viewport", viewport);

	float min = 0;
	float max = 0;
	if (BScrollBar *scrollBar = fScrollView->ScrollBar(B_VERTICAL); scrollBar != nullptr) {
		scrollBar->GetRange(&min, &max);
	}
	out.Set("verticalRange", max);
}


} // HaikuReact
