#include "value.hpp"
#include "variable.hpp"
#include "function.hpp"
#include <algorithm>
#include <cmath>

using namespace kn;

Value::Value() noexcept : data(null {}) {}
Value::Value(bool boolean) noexcept : data(boolean) {}
Value::Value(number num) noexcept : data(num) {}
Value::Value(char chr) noexcept : Value(kn::make_shared<string>(1, chr)) {}
Value::Value(string str) noexcept : Value(kn::make_shared<string>(str)) {}
Value::Value(shared<string> str) noexcept : data(str) {}
Value::Value(list lst) noexcept : Value(kn::make_shared<list>(lst)) {}
Value::Value(shared<list> lst) noexcept : data(lst) {}
Value::Value(Variable* var) noexcept : data(var) {}
Value::Value(shared<Function> func) noexcept : data(func) {}

static void remove_keyword(std::string_view& view) {
	do {
		view.remove_prefix(1);
	} while (std::isupper(view.front()) || view.front() == '_');
}

std::optional<Value> Value::parse(std::string_view& view) {
	char front;

top:
	if (view.empty())
		return std::nullopt;

	// note that in knight, all forms of parens and `:` are considered whitespace.
	switch (front = view.front()) {
	case '#':
		do {
			view.remove_prefix(1);
		} while (!view.empty() && view.front() != '\n');
		// fallthrough, as we know the first character is `\n`.

	case ' ': case '\t': case '\n': case '\r': case '\v': case '\f':
	case '(': case  ')': case ':': 
		do {
			view.remove_prefix(1);
		} while (std::isspace(view.front()));
		goto top;

	case 'N':
		remove_keyword(view);
		return std::make_optional<Value>();

	case 'T':
	case 'F':
		remove_keyword(view);
		return std::make_optional<Value>(front == 'T');

	case '@':
		view.remove_prefix(1);
		return std::make_optional<Value>(std::vector<Value>());

	case '\'':
	case '\"': {
		view.remove_prefix(1);
		auto begin = view.cbegin();

		for(char quote = front; quote != view.front(); view.remove_prefix(1))
			if (view.empty())
				throw Error("unmatched quote encountered!");

		string ret(begin, view.cbegin());
		view.remove_prefix(1);

		return std::make_optional<Value>(ret);
	}

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9': {
		number num = 0;

		for (; std::isdigit(front = view.front()); view.remove_prefix(1)) {
			num = num * 10 + (front - '0');
		}

		return std::make_optional<Value>(num);
	}

	default:
		if (auto var = Variable::parse(view))
			return std::make_optional<Value>(*var);

		if (auto func = Function::parse(view))
			return std::make_optional<Value>(*func);

		throw Error("invalid character encountered: " + std::to_string(front));
	}
}


template <typename... Fns>
struct overload : Fns... { using Fns::operator()...; };

template <typename... Fns>
overload(Fns...) -> overload<Fns...>;

bool Value::to_boolean() const {
	return std::visit(overload {
		[](null) { return false; },
		[](bool boolean) { return boolean; },
		[](number num) { return num != 0; },
		[](shared<string> const& str) { return str->length() != 0; },
		[](shared<list> const& lst) { return lst->size() != 0; },
		[](auto) -> bool { throw Error("bad type for boolean conversion"); }
	}, data);
}

static number string_to_number(string const& str) {
	// a custom `stroll` that will will just stop at the first invalid character
	number ret = 0;
	auto begin = std::find_if_not(str.cbegin(), str.cend(), [](char c) { return std::isspace(c); });

	if (begin == str.cend())
		return (number) 0;

	int sign = (*begin == '-') ? -1 : 1;

	if (*begin == '-' || *begin == '+')
		++begin;

	for (; begin != str.cend() && std::isdigit(*begin); ++begin)
		ret = ret * 10 + (*begin - '0');

	return ret * sign;
}

number Value::to_number() const {
	return std::visit(overload {
		[](null) { return (number) 0; },
		[](bool boolean) { return (number) boolean; },
		[](number num) { return num; },
		[](shared<string> const& str) { return string_to_number(*str); },
		[](shared<list> const& lst) { return (number) lst->size(); },
		[](auto) -> number { throw Error("bad type for number conversion"); }
	}, data);
}


static string join_list(list const& lst, std::string_view sep) {
	string s;
	bool start = true;

	for (auto ele : lst) {
		if (!start)
			s.append(sep);
		start = false;
		s.append(*ele.to_string());
	}

	return s;
}

shared<string> Value::to_string() const {
	return std::visit(overload {
		[](null) {
			static shared<string> null_string = kn::make_shared<string>("");
			return null_string;
		},
		[](bool boolean) {
			static shared<string> true_string = kn::make_shared<string>("true");
			static shared<string> false_string = kn::make_shared<string>("false");
			return boolean ? true_string : false_string;
		},
		[](number num) { return kn::make_shared<string>(std::to_string(num)); },
		[](shared<string> const& str) { return str; },
		[](shared<list> const& lst) { return kn::make_shared<string>(join_list(*lst, std::string_view("\n"))); },
		[](auto) -> shared<string> { throw Error("bad type for string conversion"); }
	}, data);
}

static shared<list> make_single_list(Value v) {
	list lst;
	lst.reserve(1);
	lst.push_back(v);
	return kn::make_shared<list>(lst);
}

shared<list> Value::to_list() const {
	static auto empty_list = kn::make_shared<list>();
	static auto true_list = make_single_list(Value(true));
	static auto zero_list = make_single_list(Value((number) 0));

	return std::visit(overload {
		[](null) { return empty_list; },
		[](bool boolean) { return boolean ? true_list : empty_list; },
		[](number num) {
			if (num == 0) return zero_list;

			list lst;
			for (; num != 0; num /= 10)
				lst.push_back(Value(num % 10));

			std::reverse(lst.begin(), lst.end());
			lst.shrink_to_fit();
			return kn::make_shared<list>(lst);
		},
		[](shared<string> const& str) {
			if (str->length() == 0)
				return empty_list;

			list lst;
			lst.reserve(str->length());
			for (auto chr : *str)
				lst.push_back(Value(chr));
			return kn::make_shared<list>(lst);
		},
		[](shared<list> const& lst) { return lst; },
		[](auto) -> shared<list> { throw Error("bad type for list conversion"); }
	}, data);
}

std::ostream& Value::dump(std::ostream& out) const {
	std::visit(overload {
		[&](null) { out << "null"; }, 
		[&](bool boolean) { out << (boolean ? "true" : "false"); },
		[&](number num) { out << num; },
		[&](shared<string> const& str) {
			out << "\"";

			for (auto chr : *str) {
				switch (chr) {
				case '\r': out << "\\r"; break;
				case '\n': out << "\\n"; break;
				case '\t': out << "\\t"; break;

				case '\\':
				case '\"':
					out << "\\";
					[[fallthrough]];
				default:
					out << chr;
				}
			}

			out << "\"";
		},
		[&](shared<list> const& lst) {
			out << "[";
			bool first = true;
			for (auto ele : *lst) {
				if (!first)
					out << ", ";
				first = false;
				out << ele;
			}
			out << "]";
		},
		[&](Variable* var) { out << *var; },
		[&](shared<Function> const& func) { out << *func; },
	}, data);

	return out;
}

Variable *Value::as_variable() const {
	if (auto var = std::get_if<Variable*>(&data))
		return *var;

	throw Error("invalid kind for 'as_variable'");
}

Value Value::to_ascii() const {
	if (auto chr = std::get_if<number>(&data)) {
		if (*chr <= '\0' || '~' < *chr) throw Error("number is not valid ascii");
		return Value(string(1, *chr));
	}

	if (auto str = std::get_if<shared<string>>(&data)) {
		if (!(*str)->length()) throw Error("string is empty");
		return Value((number) (**str)[0]);
	}

	throw Error("invalid kind for 'to_ascii'");
}

Value Value::get(size_t start, size_t length) const {
	if (auto lst = std::get_if<shared<list>>(&data))
		return Value(list((*lst)->cbegin() + start, (*lst)->cbegin() + start + length));

	if (auto str = std::get_if<shared<string>>(&data))
		return Value((*str)->substr(start, length));

	throw Error("invalid kind for get");
}

Value Value::set(size_t start, size_t length, Value replacement) const {
	if (auto lst = std::get_if<shared<list>>(&data)) {
		auto repl = replacement.to_list();
		list res((*lst)->size() + repl->size() - length);
		std::copy((*lst)->cbegin(), (*lst)->cbegin() + start, res.begin());
		std::copy(repl->cbegin(), repl->cend(), res.begin() + start);
		std::copy((*lst)->cbegin() + start + length, (*lst)->cend(), res.begin() + start + repl->size());
		return Value(res);
	}

	if (auto str = std::get_if<shared<string>>(&data)) {
		auto repl = replacement.to_string();
		string res;
		res.reserve((*str)->length() + repl->length() - length);
		res.append(**str, 0, start);
		res.append(*repl);
		res.append(**str, start + length, std::string::npos);
		return Value(res);
	}

	throw Error("invalid kind for set");
}


Value Value::head() const {
	if (auto lst = std::get_if<shared<list>>(&data)) {
		if ((*lst)->size() == 0)
			throw Error("head on empty list");
		return (**lst)[0];
	}

	if (auto str = std::get_if<shared<string>>(&data)) {
		if ((*str)->length() == 0)
			throw Error("head on empty string");
		return Value((**str)[0]);
	}

	throw Error("head on non-list non-string");
}

Value Value::tail() const {
	if (auto lst = std::get_if<shared<list>>(&data)) {
		auto iter = (*lst)->cbegin();
		if (iter++ == (*lst)->cend())
			throw Error("tail on empty list");
		return Value(list(iter, (*lst)->cend()));
	}

	if (auto str = std::get_if<shared<string>>(&data)) {
		auto iter = (*str)->cbegin();
		if (iter++ == (*str)->cend())
			throw Error("tail on empty string");
		return Value(string(iter, (*str)->cend()));
	}

	throw Error("tail on non-list non-string");
}

Value Value::run() {
	if (auto var = std::get_if<Variable*>(&data))
		return (*var)->run();

	if (auto func = std::get_if<shared<Function>>(&data))
		return (*func)->run();

	return *this;
}


Value Value::operator-() const {
	return Value(-to_number());
}

Value Value::operator+(Value const& rhs) const {
	if (auto str = std::get_if<shared<string>>(&data))
		return Value(kn::make_shared<string>(**str + *rhs.to_string()));

	if (auto num = std::get_if<number>(&data))
		return Value(*num + rhs.to_number());

	if (auto lst = std::get_if<shared<list>>(&data)) {
		auto rlist = rhs.to_list();
		list cat;
		cat.reserve((*lst)->size() + rlist->size());
		cat.insert(cat.end(), (*lst)->begin(), (*lst)->end());
		cat.insert(cat.end(), rlist->begin(), rlist->end());
		return Value(kn::make_shared<list>(cat));
	}

	throw Error("invalid kind given to '+'");
}

Value Value::operator-(Value const& rhs) const {
	if (auto num = std::get_if<number>(&data))
		return Value(*num - rhs.to_number());

	throw Error("invalid kind given to '-'");
}

Value Value::operator*(Value const& rhs) const {
	number amount = rhs.to_number();

	if (auto num = std::get_if<number>(&data))
		return Value(*num * amount);

	if (amount < 0)
		throw Error("cannot replicate by a negative number");

	if (auto lst = std::get_if<shared<list>>(&data)) {
		list ret((*lst)->size() * amount);
		auto iter = ret.begin();

		for (auto i = 0; i < amount; ++i)
			iter = std::copy((*lst)->cbegin(), (*lst)->cend(), iter);

		return Value(ret);
	}

	if (auto str = std::get_if<shared<string>>(&data)) {
		string ret;
		ret.reserve((*str)->length() * amount);

		for (auto i = 0; i < amount; ++i)
			ret.append(**str);

		return Value(ret);
	}

	throw Error("invalid kind given to '*'");
}

Value Value::operator/(Value const& rhs) const {
	number num;

	if (auto pnum = std::get_if<number>(&data))
		num = *pnum;
	else
		throw Error("invalid kind given to '/'");

	auto rnum = rhs.to_number();

	if (!rnum)
		throw new Error("Cannot divide by zero");

	return Value(num / rnum);
}

Value Value::operator%(Value const& rhs) const {
	number num;

	if (auto pnum = std::get_if<number>(&data))
		num = *pnum;
	else
		throw Error("invalid kind given to '%'");

	auto rnum = rhs.to_number();
	if (!rnum)
		throw new Error("Cannot modulo by zero");

	return Value(num % rnum);
}

Value Value::pow(Value const& rhs) const {
	if (auto base = std::get_if<number>(&data))
		return Value((number) std::pow(*base, rhs.to_number()));

	if (auto lst = std::get_if<shared<list>>(&data))
		return Value(join_list(**lst, std::string_view(rhs.to_string()->data())));

	throw Error("invalid kind given to '^'");
}

bool Value::operator==(Value const& rhs) const {
	return data == rhs.data;
}

bool Value::operator<(Value const& rhs) const {
	return std::visit([&](auto&& lhs) -> bool {
		using T = std::decay_t<decltype(lhs)>;

		if constexpr (std::is_same_v<T, number>) return lhs < rhs.to_number();
		else if constexpr (std::is_same_v<T, shared<string>>) return *lhs < *rhs.to_string();
		else if constexpr (std::is_same_v<T, shared<list>>) return *lhs < *rhs.to_list();
		else if constexpr (std::is_same_v<T, bool>) return !lhs && rhs.to_boolean();
		else throw Error("invalid kind given to '<'");
	}, data);
}

bool Value::operator>(Value const& rhs) const {
	return std::visit([&](auto&& lhs) -> bool {
		using T = std::decay_t<decltype(lhs)>;

		if constexpr (std::is_same_v<T, number>) return lhs > rhs.to_number();
		else if constexpr (std::is_same_v<T, shared<string>>) return *lhs > *rhs.to_string();
		else if constexpr (std::is_same_v<T, shared<list>>) return *lhs > *rhs.to_list();
		else if constexpr (std::is_same_v<T, bool>) return lhs && !rhs.to_boolean();
		else throw Error("invalid kind given to '>'");
	}, data);
}
