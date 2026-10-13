// Turns a path as the game spells it (backslashes, a drive letter, any letter case) into one that
// exists on this filesystem, matching each component case-insensitively.
#pragma once
#include <string>

std::string port_resolve_path(const char* gamePath);
