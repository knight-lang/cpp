#include "knight.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

void usage(char const* program) {
	std::cerr << "usage: " << program << " (-e 'expression' | -f file)" << std::endl;
	exit(1);
}

int main(int argc, char **argv) {
	if (argc != 3)
		usage(argv[0]);

	kn::initialize();

	try {
		if (std::string_view("-e") == argv[1])  {
			kn::play(argv[2]);
		} else if (std::string_view("-f") == argv[1]) {
			std::ifstream file(argv[2]);
			std::ostringstream contents;
			contents << file.rdbuf();
			kn::play(contents.str());
		} else {
			usage(argv[0]);
		}
	} catch (std::exception& err) {
		std::cerr << "error with your code: " << err.what() << std::endl;
		return 1;
	}
}
