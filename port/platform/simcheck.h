// What gta2 --simcheck hashes (simcheck.cpp): the numbers every machine of a network game must
// compute alike. The unit tests (port/tests/test_tables.cpp) check them too.
#pragma once

#include <stdint.h>

struct SimcheckHashes
{
    uint64_t trigonometry; // the sine, cosine and tangent tables
    uint64_t random; // the first 100,000 rand() values after srand(1)
    uint64_t maths; // square roots and arctangents over a grid
};

SimcheckHashes port_simcheck_hashes();
