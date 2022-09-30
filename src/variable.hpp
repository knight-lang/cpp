#pragma once

#include "value.hpp"
#include <optional>

namespace kn {

// A variable within Knight.
//
// As per the Knight specs, all variables are global.
class Variable {
	// The name of the variable. This cannot be changed.
	std::string const name;

	// The value associated with this variable.
	std::optional<Value> value;

	// Creates a new Variable with the given name.
	Variable(std::string name) noexcept : name(name) {};

public:

	// There is no default variable.
	Variable() = delete;

	// Looks up the variable associated with `name`, or creates it if it doesnt exist
	static Variable* lookup(std::string_view name);

	// Runs the variable, looking up its last assigned value.
	//
	// Throws an `Error` if the variable was never assigned.
	Value run() const {
		if (!value)
			throw Error("unknown variable encountered: " + name);

		return *value;
	}

	// Assigns a value to this variable, discarding its previous value.
	void assign(Value newvalue) noexcept {
		value = std::move(newvalue);
	}

	// Provides debugging output of this type.
 	friend std::ostream& operator<<(std::ostream& out, const Variable& s);
};

} // namespace kn
