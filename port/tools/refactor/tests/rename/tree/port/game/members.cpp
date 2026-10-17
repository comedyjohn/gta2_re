#include "members.hpp"

Holder<Widget_10> gHolder;

Widget_10::Widget_10() : v(0) {}
Widget_10::~Widget_10() {}

void Draw_401000(int) {}
void Draw_401000(float) {}

int Use(A& a, B& b, Derived& d, Base* p)
{
    Draw_401000(1);
    Draw_401000(1.0f);
    d.Update_402000();
    p->Update_402000();
    Widget_10 w;
    Widget_10* wp = new Widget_10(w);
    delete wp;
    return a.field_0_x + b.field_0_x + a.Get();
}
