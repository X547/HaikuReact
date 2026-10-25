#pragma once

#include <string>
#include <variant>
#include <vector>

#include <SupportDefs.h>

#include <napi.h>


namespace HaikuReact {


using EventValue = std::variant<
	std::monostate, bool, int32, double, std::string, std::vector<int32>, std::vector<std::string>
>;

// A native event on its way to JS. Node id 0 addresses the application.
struct Event {
	uint32 nodeId {};
	std::string name;
	EventValue value;
};


// Delivers events from Haiku looper threads to the JS thread. Posting never
// blocks, so a looper thread can never wait for JS.
class EventBridge {
public:
	static void Start(Napi::Env env, Napi::Function callback);
	static void Stop();

	static void Post(Event event);
};


} // HaikuReact
