#pragma once

#include <optional>

#include "Node.h"

class BGroupLayout;


namespace HaikuReact {


class ReactWindow;


// Window behaviors that map to B_NOT_* / B_CLOSE_ON_ESCAPE flags.
struct WindowBehavior {
	bool resizable = true;
	bool zoomable = true;
	bool closable = true;
	bool minimizable = true;
	bool movable = true;
	bool closeOnEscape = false;
};


// <Window>: ReactWindow with a vertical BGroupLayout root. The window thread
// runs from construction on, but the window stays hidden until Mount().
// Events: closeRequest, activate (active), move ([x, y]), resize ([width,
// height]).
class WindowNode: public Node {
private:
	ReactWindow *fWindow;
	BGroupLayout *fLayout;
	bool fMounted = false;
	bool fVisible = true;
	std::optional<float> fLeft;
	std::optional<float> fTop;
	std::optional<float> fWidth;
	std::optional<float> fHeight;
	WindowBehavior fBehavior;

	void ApplyFrame();
	void ApplyLookAndFeel(const PropSet &props);
	void UpdateVisibility();

protected:
	void AttachChild(Node &child, int32 index) final;
	void DetachChild(Node &child) final;
	void ChildLayoutHintsChanged(Node &child, int32 index) final;
	void ApplyOwnProps(const PropSet &props) final;

public:
	explicit WindowNode(uint32 id);
	~WindowNode() final;

	const char *TypeName() const final {return "Window";}
	BLooper *Looper() const final;

	void Mount() final;
	void Invoke() final;
	void SimulateKey(const std::string &bytes, uint32 modifiers) final;
	void Dump(Napi::Object &out) const final;
};


} // HaikuReact
