#pragma once

#include "error.hpp"
#include "string.hpp"
#include <string>
#include <string_view>
#include <memory>
#include <variant>
#include <ostream>
#include <optional>

namespace kn {
	using number = long long;
	using string = std::string;
	struct null {};


	class Variable;
	class Function;

class Value {
	std::variant<
		null,
		bool,
		number,
		String,
		Variable*,
		std::shared_ptr<Function>
	> data;

	public:

		explicit Value() noexcept;
		explicit Value(bool boolean) noexcept;
		explicit Value(number num) noexcept;
		explicit Value(String str) noexcept;
		explicit Value(Variable* var) noexcept;
		explicit Value(std::shared_ptr<Function> func) noexcept;
		static std::optional<Value> parse(std::string_view& view);

		Value run();
		std::ostream& dump(std::ostream& out) const;

		bool to_boolean();
		number to_number();
		String to_string();
		Variable* as_variable() const;

		Value operator+(Value&& rhs);
		Value operator-(Value&& rhs);
		Value operator*(Value&& rhs);
		Value operator/(Value&& rhs);
		Value operator%(Value&& rhs);
		Value pow(Value&& rhs);

		bool operator==(Value&& rhs);
		bool operator<(Value&& rhs);
		bool operator>(Value&& rhs);
	};
}
