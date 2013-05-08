class CTVector
{
public:

    CTVector();
    CTVector(int size);
    CTVector(int size, const Type & fillValue);
    CTVector(const CTVector & vec);
    ~CTVector();

    const CTVector & operator =(const CTVector & vec);

    int capacity() const;
    int size()     const;

    Type  & operator [](int index);
    const Type & operator [](int index) const;

    Type & back();
    void resize(int newSize);
    void reserve(int size);
    void push_back(const Type & t);
    void pop_back();
    void clear();

    typedef Type * iterator;

    iterator begin();
    iterator end();

private:

    int  mySize;                            // # elements in array
    int  myCapacity;                        // capacity of array
    Type * myList;                      // array used for storage
};


