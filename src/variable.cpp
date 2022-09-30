#include "variable.hpp"
#include "robin_hood_map.hpp"
#include <iostream>
#include <memory>

namespace kn {

// The set of all known variables.
//
// Yes globals are bad, and i probably shouldn't do this.
// But it was quick and dirty and i don't take pride in my work
static robin_hood::unordered_map<std::string_view, Variable*> ENVIRONMENT;

Variable* Variable::lookup(std::string_view name) {
	if (auto match = ENVIRONMENT.find(name); match != ENVIRONMENT.cend())
		return match->second;

	auto variable = new Variable(std::string(name));
	ENVIRONMENT.emplace(std::string_view(variable->name), variable);

	return variable;
}

std::ostream& operator<<(std::ostream& out, Variable const& variable) {
	return out << "Variable(" << variable.name << ")";
}

} // namespace kn
