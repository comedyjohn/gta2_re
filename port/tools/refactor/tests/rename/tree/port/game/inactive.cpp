#include "members.hpp"

// A branch the build does not compile.
int Branch(A& a)
{
#if defined(NOT_DEFINED_ANYWHERE)
    return a.field_0_x + 1;
#else
    return a.field_0_x;
#endif
}
