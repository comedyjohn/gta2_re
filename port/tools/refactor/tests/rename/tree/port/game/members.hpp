#pragma once
// The same member name in two classes: renaming A's must leave B's alone.
struct A
{
    int field_0_x;
    int y;
    int Get() const { return field_0_x; }
};

struct B
{
    int field_0_x;
};

// An overload set: both functions are renamed, and so are all calls.
void Draw_401000(int);
void Draw_401000(float);

// A method called through a base pointer and through a derived object.
struct Base
{
    void Update_402000() {}
};
struct Derived : Base
{
};

// Constructor, destructor, a friend declaration and a template argument.
template <class T>
struct Holder
{
    T item;
};
struct Widget_10
{
    Widget_10();
    ~Widget_10();
    Widget_10(const Widget_10& other) : Widget_10() { (void)other; }
    int v;
};
struct Friend
{
    friend struct Widget_10;
};
extern Holder<Widget_10> gHolder;

// A virtual function: refused.
struct V
{
    virtual void Tick_403000() {}
};
