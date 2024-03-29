#pragma once

#include <string>
#include <stdexcept>

namespace kn {

// The base class for all errors within Knight.
struct Error : public std::runtime_error {
	// Creates a new error with the given message.
	explicit Error(std::string const& what_arg) : std::runtime_error(what_arg) {};
};

} // namespace kn
