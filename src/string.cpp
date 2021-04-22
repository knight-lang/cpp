#include "string.hpp"
#include <optional>

using namespace kn;

#ifndef CACHE_SIZE
#define CACHE_SIZE (1<<15)
#endif

#define MAX_CACHE_SIZE 32

static std::optional<String> string_cache[CACHE_SIZE];


static std::shared_ptr<std::string> empty{std::make_shared<std::string>(std::string(""))};
String::String() : str(empty) {}
String::String(size_t length) : String(std::string(length, '\0')) {}

String String::fetch(std::string_view view) {
	if (view.size() > MAX_CACHE_SIZE)
		return String(std::string(view));

	auto hash = std::hash<std::string_view>()(view);
	std::optional<String>* string = &string_cache[hash % CACHE_SIZE];

	if (!*string || *string != view)
		*string = String(std::string(view));

	return **string;
}