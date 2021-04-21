#include "variable.hpp"
#include "robin_hood_map.hpp"
#include <iostream>
#include <memory>

using namespace kn;

Variable::Variable(std::string name) noexcept : name(name) {}

// The set of all known variables.
// note that variables last for the lifetime of the program, which is why we have pointers
static robin_hood::unordered_map<std::string_view, Variable*> ENVIRONMENT;

std::optional<Value> Variable::parse(std::string_view& view) {
	char front = view.front();

	if (!std::islower(front) && front != '_')
		return std::nullopt;

	auto start = view.cbegin();

	do {
		view.remove_prefix(1);
		front = view.front();
	} while (std::islower(front) || front == '_' || std::isdigit(front));

	auto identifier = std::string_view(start, view.cbegin() - start);

	if (auto match = ENVIRONMENT.find(identifier); match != ENVIRONMENT.cend())
		return std::make_optional<Value>(match->second);

	auto variable = new Variable(std::string(identifier));
	ENVIRONMENT.emplace(std::string_view(variable->name), variable);

	return std::make_optional<Value>(variable);
}

std::ostream& Variable::dump(std::ostream& out) const noexcept {
	return out << "Variable(" << name << ")";
}

Value Variable::run() {
	if (!value)
		throw Error("unknown variable encountered: " + name);

	return *value;
}

void Variable::assign(Value newvalue) noexcept {
	value = std::move(newvalue);
}
