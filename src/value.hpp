#pragma once

#include "error.hpp"
#include "shared.hpp"

#include <string>
#include <string_view>
#include <variant>
#include <ostream>
#include <optional>
#include <vector>

namespace kn {

// Forward declarations.
class Value;
class Variable;
class Function;

// Type definitions used internally.
using number = long long;
using string = std::string;
using list = std::vector<Value>;
using null = std::monostate;

// The value type in Knight represents all the different types that can occur during
// a knight program's execution.
class Value {
	std::variant<
		null,
		bool,
		number,
		shared<string>,
		shared<list>,
		Variable*,
		shared<Function>
	> data;

public:

	// Constructors
	explicit Value() noexcept : data(null {}) {}
	explicit Value(bool boolean) noexcept : data(boolean) {}
	explicit Value(number num) noexcept : data(num) {}
	explicit Value(shared<string> str) noexcept : data(str) {}
	explicit Value(shared<list> lst) noexcept : data(lst) {}
	explicit Value(Variable* var) noexcept : data(var) {}
	explicit Value(shared<Function> func) noexcept : data(func) {}

	// Convenience constructors
	explicit Value(char chr) noexcept : Value(kn::make_shared<string>(1, chr)) {}
	explicit Value(string str) noexcept : Value(kn::make_shared<string>(str)) {}
	explicit Value(list lst) noexcept : Value(kn::make_shared<list>(lst)) {}

	// Parses a `Value` from the stream
	static std::optional<Value> parse(std::string_view& view);

	// Executes the value according to the `data` variant.
	Value run();

	// (dump only exists because i cant figure out how to get `operator<<` to be a friend)
	friend std::ostream& operator<<(std::ostream& out, Value const& value);

	// Type conversions. Throws errors for variables or functions.
	bool to_boolean() const;
	number to_number() const;
	shared<string> to_string() const;
	shared<list> to_list() const;

	// Returns the internal variable. Throws an error if it's not a variable.
	Variable* as_variable() const;

	// Native Knight functions.
	Value get(size_t start, size_t length) const;
	Value set(size_t start, size_t length, Value replacement) const;
	Value head() const;
	Value tail() const;
	Value to_ascii() const;

	// Native knight operators.
	Value operator-() const;
	Value operator+(Value const& rhs) const;
	Value operator-(Value const& rhs) const;
	Value operator*(Value const& rhs) const;
	Value operator/(Value const& rhs) const;
	Value operator%(Value const& rhs) const;
	Value pow(Value const& rhs) const;
	bool operator==(Value const& rhs) const;
	bool operator<(Value const& rhs) const;
	bool operator>(Value const& rhs) const;
};

} // namespace kn
