#pragma once

class BLooper;


namespace HaikuReact {


// Spans one React commit. Windows touched during the commit are locked on
// first use and released together at the end, so a window never draws a
// partially applied commit.
class CommitScope {
public:
	static void Begin();
	static void End();
	static bool IsActive();

	// Locks the looper until End() unless it is already held.
	static void Hold(BLooper *looper);
	// Drops a looper that is about to quit; its lock is released by Quit().
	static void Forget(BLooper *looper);
};


} // HaikuReact
