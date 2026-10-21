#pragma once

#include <optional>
#include <string>
#include <vector>

#include <InterfaceDefs.h>

#include <napi.h>


namespace HaikuReact {


// Layout spacing; negative B_USE_*_SPACING values are resolved by the layout.
struct Spacing {
	float value {};
};

struct Insets {
	float left {};
	float top {};
	float right {};
	float bottom {};
};

// Explicit layout size in pixels, B_SIZE_UNLIMITED or B_SIZE_UNSET.
struct SizeValue {
	float value {};
};


// Read-only view of a props object sent from JS. Only keys present in the
// object are reported; a key set to null or undefined means "reset to the
// default value".
class PropSet {
private:
	Napi::Object fProps;

	[[noreturn]] void ThrowTypeError(const char *key, const char *expected) const;

	void Convert(const char *key, const Napi::Value &value, std::string &result) const;
	void Convert(const char *key, const Napi::Value &value, bool &result) const;
	void Convert(const char *key, const Napi::Value &value, int32 &result) const;
	void Convert(const char *key, const Napi::Value &value, float &result) const;
	void Convert(const char *key, const Napi::Value &value, std::vector<float> &result) const;
	void Convert(const char *key, const Napi::Value &value, std::vector<int32> &result) const;
	void Convert(const char *key, const Napi::Value &value, std::vector<std::string> &result) const;
	void Convert(const char *key, const Napi::Value &value, Spacing &result) const;
	void Convert(const char *key, const Napi::Value &value, Insets &result) const;
	void Convert(const char *key, const Napi::Value &value, SizeValue &result) const;
	void Convert(const char *key, const Napi::Value &value, orientation &result) const;
	void Convert(const char *key, const Napi::Value &value, alignment &result) const;
	void Convert(const char *key, const Napi::Value &value, vertical_alignment &result) const;
	void Convert(const char *key, const Napi::Value &value, border_style &result) const;

	template<typename T>
	void Convert(const char *key, const Napi::Value &value, std::optional<T> &result) const;

public:
	explicit PropSet(Napi::Object props);

	template<typename T>
	std::optional<T> Get(const char *key, const T &defaultValue) const;
};


template<typename T>
void PropSet::Convert(const char *key, const Napi::Value &value, std::optional<T> &result) const
{
	T inner {};
	Convert(key, value, inner);
	result = inner;
}


template<typename T>
std::optional<T> PropSet::Get(const char *key, const T &defaultValue) const
{
	if (!fProps.HasOwnProperty(key)) {
		return std::nullopt;
	}
	const Napi::Value value = fProps.Get(key);
	if (value.IsNull() || value.IsUndefined()) {
		return defaultValue;
	}
	T result {};
	Convert(key, value, result);
	return result;
}


} // HaikuReact
