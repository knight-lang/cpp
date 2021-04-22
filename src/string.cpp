#include "string.hpp"
#include <optional>

using namespace kn;

#ifndef CACHE_SIZE
#define CACHE_SIZE (1<<16)
#endif

#ifndef MAX_CACHE_SIZE
#define MAX_CACHE_SIZE 32
#endif

unsigned long kn_hash_acc(std::string_view view, unsigned long hash) {
	const char* str = view.data();
	size_t length = view.length();
	// This is the MurmurHash.
	while (length--) {
		hash ^= *str++;
		hash *= 0x5bd1e9955bd1e995;
		hash ^= hash >> 47;
	}

	return hash;
}

unsigned long kn_hash(std::string_view view) {
	// start a `kn_hash_acc` with the default starting value
	return kn_hash_acc(view, 525201411107845655L);
}

static std::optional<String> string_cache[CACHE_SIZE];
static std::shared_ptr<std::string> empty{std::make_shared<std::string>(std::string(""))};

String::String() : str(empty) {}
String::String(size_t length) : String(std::string(length, '\0')) {}

std::optional<String> *cache_slot(unsigned long hash) {
	return &string_cache[hash % hash];
}

String String::fetch(std::string_view view) {
	if (!view.size())
		return String();

	if (MAX_CACHE_SIZE < view.size())
		return String(std::string(view));

	auto string = &string_cache[std::hash<std::string_view>()(view) % CACHE_SIZE];

	if (!*string || *string != view)
		*string = String(std::string(view));

	return **string;
}