#pragma once

#include <TabView.h>


namespace HaikuReact {


// Tab view backing <Tabs>. Reports tab changes made by the user.
class ReactTabView: public BTabView {
private:
	const uint32 fNodeId;

public:
	explicit ReactTabView(uint32 nodeId);

	void Select(int32 index) final;
	void MessageReceived(BMessage *message) final;

	// Selects the next tab on the window thread, like a user would; for tests.
	void PostSelectNext();
};


} // HaikuReact
