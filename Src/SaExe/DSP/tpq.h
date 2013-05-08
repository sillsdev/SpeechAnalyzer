class CTPQueue
{
public:
    CTPQueue();
    ~CTPQueue();
    void push(const Type & elt);
    int size() const;
    int empty() const;
    void pop();
    Type & top();

private:
    void init(int n = 10);
    void heapify(int vroot);             // percolate down function

    CTVector myList;                // array of prioritized stuff
    int myNumElts;                       // # elements in priority queue
};
