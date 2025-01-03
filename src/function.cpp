#include "function.hpp"
#include "value.hpp"
#include "variable.hpp"
#include "shared.hpp"
#include "knight.hpp"
#include "include/robin_hood_map.hpp"

#include <iostream>
#include <cstdio>
#include <random>

namespace kn {

// The list of all _functions_
static robin_hood::unordered_map<char, std::pair<funcptr_t, size_t>> FUNCTIONS;

std::optional<Value> Function::parse(std::string_view& view) {
	char front = view.front();

	// if the first character isn't a valid function Variable, then just return early.
	if (FUNCTIONS.count(front) == 0)
		return std::nullopt;

	view.remove_prefix(1);

	auto func_pair = FUNCTIONS[front];

	// remove trailing upper-case letters for keyword functions.
	if (isupper(front)) {
		while (isupper(view.front()) || view.front() == '_')
			view.remove_prefix(1);
	}

	// parse the arguments out.
	args_t args;
	for(size_t i = 0; i < func_pair.second; ++i) {
		auto value = Value::parse(view);

		if (!value)
			throw Error("Cannot parse function.");

		args.push_back(*value);
	}

	return std::make_optional<Value>(make_shared<Function>(Function(func_pair.first, front, args)));
}

void Function::register_function(char name, size_t arity, funcptr_t func) {
	FUNCTIONS.insert({ name, std::make_pair(func, arity) });
}

std::ostream& operator<<(std::ostream& out, Function const& func) {
	out << "Function(" << func.name;

	for (auto arg : func.args)
		out << ", " << arg;

	return out << ")";
}


// Prompts for a single line from stdin.
static Value prompt(args_t&) {
	string line;
	std::getline(std::cin, line);

	if (std::cin.eof() && line.length() == 0)
		return Value();

	if (line.length() != 0 && line.back() == '\r')
		line.pop_back();

	return Value(line);
}

// Gets a random number.
static Value random(args_t&) {
	static thread_local std::random_device rd;
	static thread_local std::mt19937 gen(rd());
	static thread_local std::uniform_int_distribution<number> dist;

	return Value((number) dist(gen));
}

// Creates a block of code.
static Value block(args_t& args) {
	return args[0];
}

// Calls a block of code.
static Value call(args_t& args) {
	return args[0].run().run();
}

// Evaluates the argument as Knight source code.
#ifndef KN_NEXTENSIONS
static Value eval(args_t& args) {
	auto code = args[0].run().to_string();
	return kn::play(std::string_view(*code));
}

// Runs a shell command, returns the stdout of the command.
// effectively copied my C impl...
static Value system(args_t& args) {
	auto cmd = args[0].run().to_string();
	FILE *stream = popen(cmd->c_str(), "r");

	if (stream == NULL) {
		throw Error("unable to execute command.");
	}

	size_t cap = 2048;
	size_t length = 0;
	size_t tmp;

	char *result = (char *) malloc(cap);
	if (result == NULL) {
		throw Error("cant malloc");
	}

	while (0 != (tmp = fread((char *) (((size_t) result) + length), 1, cap - length, stream))) {
		length += tmp;
		if (length == cap) {
			cap *= 2;
			result = (char *) realloc(result, cap);
			if (result == NULL) {
				throw Error("cannot realloc");
			}
		}
	}

	// ignore any errors with the es
	if (ferror(stream)) {
		throw Error("unable to read command stream");
	}

	if (pclose(stream) == -1) {
		throw Error("unable to close command stream.");
	}

	return Value(string(result));
}
#endif /* !KN_NEXTENSIONS */

// Stops the program with the given status code.
static Value quit(args_t& args) {
	exit(args[0].run().to_number());
}

// Logical negation of its argument.
static Value not_(args_t& args) {
	return Value((bool) !args[0].run().to_boolean());
}

// Returns the length of the argument, when converted to a string.
static Value length(args_t& args) {
	return Value((number) args[0].run().to_list()->size());
}

// Returns the length of the argument, when converted to a string.
static Value dump(args_t& args) {
	auto arg = args[0].run();
	std::cout << arg;
	return arg;
}

// Runs the value, then converts it to a string and prints it. The execution result is returned.
//
// If the string ends with a backslash, its removed before printing. Otherwise, a newline is added.
static Value output(args_t& args) {
	auto str = args[0].run().to_string();

	if (!str->empty() && str->back() == '\\') {
		str->pop_back(); // delete the trailing backslash
		std::cout << *str;
		str->push_back('\\'); // add it back
	} else {
		std::cout << *str << std::endl;
	}

	return Value();
}

// Gets the ascii value if the first argument.
static Value ascii(args_t& args) {
	return args[0].run().to_ascii();
}

// Negates the first argument.
static Value negate(args_t& args) {
	return -args[0].run();
}

static Value box(args_t& args) {
	return Value(list{args[0].run()});
}

static Value head(args_t& args) {
	return args[0].run().head();
}

static Value tail(args_t& args) {
	return args[0].run().tail();
}

// Adds two values together.
static Value add(args_t& args) {
	return args[0].run() + args[1].run();
}

// Subtracts the second value from the first.
static Value sub(args_t& args) {
	return args[0].run() - args[1].run();
}

// Multiplies the two values together.
static Value mul(args_t& args) {
	return args[0].run() * args[1].run();
}
// Divides the first value by the second.
static Value div(args_t& args) {
	return args[0].run() / args[1].run();
}

// Modulos the first value by the second.
static Value mod(args_t& args) {
	return args[0].run() % args[1].run();
}

// Raises the first value to the power of the second.
static Value pow(args_t& args) {
	return args[0].run().pow(args[1].run());
}

// Checks to see if the two values are equal.
static Value eql(args_t& args) {
	return Value(args[0].run() == args[1].run());
}	

// Checks to see if the first value is less than the second.
static Value lth(args_t& args) {
	return Value(args[0].run() < args[1].run());
}

// Checks to see if the first value is greater than the second.
static Value gth(args_t& args) {
	return Value(args[0].run() > args[1].run());
}

// Evaluates the first value, returning it if it's falsey. Otherwise evaluates and returns the second.
static Value and_(args_t& args) {
	auto lhs = args[0].run();

	return lhs.to_boolean() ? args[1].run() : lhs;
}

// Evaluates the first value, returning it if it's truthy. Otherwise evaluates and returns the second.
static Value or_(args_t& args) {
	auto lhs = args[0].run();

	return lhs.to_boolean() ? lhs : args[1].run();
}

// Runs the first value, then runs the second and returns it.
static Value then(args_t& args) {
	args[0].run();

	return args[1].run();
}

// Assigns the second value to the first.
static Value assign(args_t& args) {
	auto variable = args[0].as_variable();

	if (variable == nullptr)
		throw Error("cannot assign to non-variables");

	auto value = args[1].run();
	variable->assign(value);
	return value;
}

// Evaluates the second value while the first one is truthy.
//
// The last value the body returned will be returned. If the body never ran, null will be returned.
static Value while_(args_t& args) {
	while (args[0].run().to_boolean())
		args[1].run();

	return Value();
}

// Runs the second value if the first is truthy. Otherwise, runs the third value.
static Value if_(args_t& args) {
	return args[1 + !args[0].run().to_boolean()].run();
}

// Returns a substring of the first value, with the second value as the start index and the third as the length.
//
// If the length is out of bounds, it's assumed to be the string length.
static Value get(args_t& args) {
	auto container = args[0].run();
	auto start = args[1].run().to_number();
	auto length = args[2].run().to_number();

	return container.get(start, length);
}

// Returns a new string with first string's range `[second, second+third)` replaced by the fourth value.
static Value substitute(args_t& args) {
	auto container = args[0].run();
	auto start = args[1].run().to_number();
	auto length = args[2].run().to_number();
	auto replacement = args[3].run();

	return container.set(start, length, replacement);
}

void Function::initialize(void) {
	Function::register_function('P', 0, &prompt);
	Function::register_function('R', 0, &random);

	Function::register_function('B', 1, &block);
	Function::register_function('C', 1, &call);
	Function::register_function('E', 1, &eval);

	Function::register_function('`', 1, &system);
	Function::register_function('Q', 1, &quit);
	Function::register_function('!', 1, &not_);
	Function::register_function('L', 1, &length);
	Function::register_function('D', 1, &dump);
	Function::register_function('O', 1, &output);
	Function::register_function('A', 1, &ascii);
	Function::register_function('~', 1, &negate);
	Function::register_function(',', 1, &box);
	Function::register_function('[', 1, &head);
	Function::register_function(']', 1, &tail);

	Function::register_function('+', 2, &add);
	Function::register_function('-', 2, &sub);
	Function::register_function('*', 2, &mul);
	Function::register_function('/', 2, &div);
	Function::register_function('%', 2, &mod);
	Function::register_function('^', 2, &pow);
	Function::register_function('?', 2, &eql);
	Function::register_function('<', 2, &lth);
	Function::register_function('>', 2, &gth);
	Function::register_function('&', 2, &and_);
	Function::register_function('|', 2, &or_);
	Function::register_function(';', 2, &then);
	Function::register_function('=', 2, &assign);
	Function::register_function('W', 2, &while_);

	Function::register_function('I', 3, &if_);
	Function::register_function('G', 3, &get);

	Function::register_function('S', 4, &substitute);
}

} // namespace kn
