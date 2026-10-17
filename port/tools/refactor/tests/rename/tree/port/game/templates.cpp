// A dependent name: libclang can't resolve the member access in the template, the text pass finds it.
template <class T>
struct Pool
{
    T* head;
    void Push(T* n)
    {
        n->mpNext = head;
        head = n;
    }
};
struct Node
{
    Node* mpNext;
};
Pool<Node> gPool;
void UsePool(Node* n)
{
    gPool.Push(n);
    n->mpNext = nullptr;
}
