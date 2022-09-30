#include "knight.hpp"
#include "function.hpp"

namespace kn {

void initialize() {
	Function::initialize();
}

Value play(std::string_view view) {
	auto value = Value::parse(view);

	if (!value)
		throw Error("nothing to parse.");

	return value->run();
}

} // namespace kn
