#pragma once

#include "value.hpp"
#include <iostream>

namespace kn {

// Initializes the Knight interpreter. This must be run before all other types are.
void initialize();

// Runs the input as Knight source code, returning its result.
Value play(std::string_view view);

} // namespace kn
