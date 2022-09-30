#pragma once

#include "error.hpp"
#include <string>
#include <string_view>
#include <memory>
#include <variant>
#include <ostream>
#include <optional>
#include <vector>
#include "shared.hpp"

namespace kn {
	class Value;
	class Variable;
	class Function;

	using number = long long;
	using string = std::string;
	using list = std::vector<Value>;
	using null = std::monostate;

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

		explicit Value() noexcept;
		explicit Value(bool boolean) noexcept;
		explicit Value(number num) noexcept;
		explicit Value(char chr) noexcept;
		explicit Value(string str) noexcept;
		explicit Value(shared<string> str) noexcept;
		explicit Value(list lst) noexcept;
		explicit Value(shared<list> lst) noexcept;
		explicit Value(Variable* var) noexcept;
		explicit Value(shared<Function> func) noexcept;

		static std::optional<Value> parse(std::string_view& view);

		Value run();
		std::ostream& dump(std::ostream& out) const;
		friend inline std::ostream& operator<<(std::ostream& out, Value const& value) {
			return value.dump(out);
		}

		bool to_boolean() const;
		number to_number() const;
		shared<string> to_string() const;
		shared<list> to_list() const;

		Variable* as_variable() const;

		Value to_ascii() const;
		Value get(size_t start, size_t length) const;
		Value set(size_t start, size_t length, Value replacement) const;
		Value head() const;
		Value tail() const;

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
}
