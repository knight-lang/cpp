#pragma once

#include "knight.hpp"
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
	public:

		// Creates a new Variable with the given name.
		explicit Variable(std::string name) noexcept;

		// There is no default variable.
		Variable() = delete;

		// Parses an Variable out, or returns `nullptr` if the first character isn't a letter or `_`.
		static std::optional<Value> parse(std::string_view& view);

		// Runs the variable, looking up its last assigned value.
		//
		// Throws an `Error` if the variable was never assigned.
		Value run();

		// Provides debugging output of this type.
		friend inline std::ostream& operator<<(std::ostream& out, Variable const& variable) {
			return out << "Variable(" << variable.name << ")";
		}

		// Assigns a value to this variable, discarding its previous value.
		void assign(Value newvalue) noexcept;
	};
}
