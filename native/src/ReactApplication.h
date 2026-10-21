#pragma once

#include <string>

#include <Application.h>


namespace HaikuReact {


// BApplication running on a thread owned by the add-on, so that the Node main
// thread stays free for libuv and JS.
class ReactApplication: public BApplication {
public:
	ReactApplication(const char *signature, status_t *error);

	void ReadyToRun() final;
	bool QuitRequested() final;
	void MessageReceived(BMessage *message) final;

	// Spawns the application thread and blocks until ReadyToRun().
	static void Start(const std::string &signature);
	static void RequestQuit();
	static bool IsRunning();

	// Asks to quit like Deskbar or a shutdown would; JS decides. For tests.
	static void PostQuitRequest();
	// Windows of the application, including alerts.
	static int32 WindowCount();
};


} // HaikuReact
