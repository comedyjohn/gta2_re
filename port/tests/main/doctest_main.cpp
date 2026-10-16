// doctest's main(). Compiled without the game's prelude, whose _WIN32 would make doctest use
// Windows APIs; the test files (port/tests/*.cpp) only use its macros.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
