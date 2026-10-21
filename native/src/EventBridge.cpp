#include "EventBridge.h"

#include <stdio.h>

#include <atomic>
#include <memory>
#include <stdexcept>
#include <type_traits>


namespace HaikuReact {


static Napi::ThreadSafeFunction sEventFn;
static std::atomic<bool> sActive {false};


static Napi::Value ValueToJs(Napi::Env env, const EventValue &value)
{
	return std::visit(
		[env](const auto &item) -> Napi::Value {
			using T = std::decay_t<decltype(item)>;
			if constexpr (std::is_same_v<T, std::monostate>) {
				return env.Undefined();
			} else if constexpr (std::is_same_v<T, bool>) {
				return Napi::Boolean::New(env, item);
			} else if constexpr (std::is_same_v<T, std::string>) {
				return Napi::String::New(env, item);
			} else if constexpr (std::is_same_v<T, std::vector<int32>>) {
				Napi::Array array = Napi::Array::New(env, item.size());
				for (uint32 i = 0; i < item.size(); i++) {
					array.Set(i, item[i]);
				}
				return array;
			} else {
				return Napi::Number::New(env, item);
			}
		},
		value
	);
}


static void CallJs(Napi::Env env, Napi::Function callback, Event *event)
{
	std::unique_ptr<Event> eventDeleter(event);
	if (env == nullptr || callback.IsEmpty()) {
		return;
	}
	try {
		callback.Call({
			Napi::Number::New(env, event->nodeId),
			Napi::String::New(env, event->name),
			ValueToJs(env, event->value)
		});
	} catch (const Napi::Error &e) {
		e.ThrowAsJavaScriptException();
	}
}


void EventBridge::Start(Napi::Env env, Napi::Function callback)
{
	if (sActive) {
		throw std::logic_error("event bridge is already started");
	}
	sEventFn = Napi::ThreadSafeFunction::New(env, callback, "HaikuReactEvents", 0, 1);
	sActive = true;
}


void EventBridge::Stop()
{
	if (sActive.exchange(false)) {
		sEventFn.Release();
	}
}


void EventBridge::Post(Event event)
{
	if (!sActive) {
		return;
	}
	auto heapEvent = std::make_unique<Event>(std::move(event));
	const napi_status status = sEventFn.NonBlockingCall(heapEvent.get(), CallJs);
	if (status != napi_ok) {
		fprintf(stderr, "HaikuReact: failed to post event \"%s\": %d\n", heapEvent->name.c_str(), status);
		return;
	}
	heapEvent.release();
}


} // HaikuReact
