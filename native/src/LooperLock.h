#pragma once

class BLooper;


namespace HaikuReact {


// Scope guard for touching a node tree from the JS thread. Locks the window
// that owns the nodes (no-op for a null looper, i.e. a detached subtree).
// Inside a CommitScope the lock is handed over to the scope instead.
class LooperLock {
private:
	BLooper *fLooper = nullptr;

public:
	explicit LooperLock(BLooper *looper);
	~LooperLock();

	LooperLock(const LooperLock &other) = delete;
	LooperLock &operator=(const LooperLock &other) = delete;
};


} // HaikuReact
