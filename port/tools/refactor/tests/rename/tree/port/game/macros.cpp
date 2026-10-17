#include "members.hpp"

// A name written in a macro body is edited where it is spelled (the definition) ...
#define GET_X(o) ((o).field_0_x)
// ... a name made by token pasting can't be, and refuses the batch.
#define CALL_DRAW(n) Draw_##n(1)

int UseMacros(A& a)
{
    return GET_X(a) + 0;
}
void UsePaste()
{
    CALL_DRAW(401000);
}
