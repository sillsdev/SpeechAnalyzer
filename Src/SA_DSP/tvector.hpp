// *******************************************************************
//  see CTVector.h for complete documentation of functions
//
//  vector class consistent with a subset of the standard C++ vector class
//  as defined in the draft ANSI standard (part of standard template library)
// *******************************************************************
#include <assert.h>

CTVector::CTVector()
//postcondition: vector has a capacity of 0 items
    : mySize(0),
      myCapacity(0),
      myList(0)
{

}


CTVector::CTVector(int size)
// precondition: size >= 0
// postcondition: vector has size/capacity of size items
   : mySize(size),
     myCapacity(size)
{
  myList = new Type[size];
}


CTVector::CTVector(int size, const Type & fillValue)
// precondition: size >= 0
// postcondition: vector has size/capacity of size items, all of which are set
//                by assignment to fillValue after default construction
    : mySize(size),
      myCapacity(size)
{
    int k;
    
    myList = new Type[size];
    for(k = 0; k < size; k++)
    {
        myList[k] = fillValue;
    }
}


CTVector::CTVector(const CTVector & vec)
// postcondition: vector is a copy of vec
    : mySize(vec.size()),
      myCapacity(vec.capacity())
{
    int k;
    myList = new Type[mySize];

    for(k = 0; k < mySize; k++)
    {
        myList[k] = vec.myList[k];
    }
}


CTVector::~CTVector ()
// postcondition: vector is destroyed
{
    delete [] myList;
    myList = 0;       // fail fast
}


const CTVector &
CTVector::operator = (const CTVector & rhs)
// postcondition: normal assignment via copying has been performed;
//                vector size and capacity match rhs
{
    if (this != &rhs)                           // don't assign to self!
    {
        delete [] myList;                       // get rid of old storage
        myCapacity = rhs.capacity();
        mySize =     rhs.size();
        myList = new Type[myCapacity];         // allocate new storage

            // copy rhs
        int k;
        for(k=0; k < mySize; k++)
        {
            myList[k] = rhs.myList[k];
        }
    }
    return *this;                               // permit a = b = c = d
}



int CTVector::capacity() const
// postcondition: returns vector's size (number of memory cells
//                allocated for vector)
{
    return myCapacity;
}


int CTVector::size() const
{
    return mySize;
}



void CTVector::push_back(const Type & t)
{
    if (mySize >= myCapacity)
    {
        reserve(myCapacity == 0 ? 2 : 2*myCapacity);
    }
    myList[mySize] = t;
    mySize++;
}


void CTVector::pop_back()
{
    if (mySize > 0)
    {
        mySize--;
    } 
}



Type & CTVector::operator [] (int k)
// description: range-checked indexing, returning kth item
// precondition: 0 <= k < length()
// postcondition: returns the kth item
{
  assert(k >= 0);
  assert(k < mySize);
  assert(k < myCapacity);

    return myList[k];
}


const Type & CTVector::operator [] (int k) const
// safe indexing, returning const reference to avoid modification
// precondition: 0 <= index < length
// postcondition: return index-th item
// exception: aborts if index is out-of-bounds
{   
  assert(k >= 0);
  assert(k < mySize);
  assert(k < myCapacity);
  
    return myList[k];
}


void CTVector::resize(int newSize)
// description:  resizes the vector to newSize elements
// precondition: the current capacity of vector is capacity(); newSize >= 0
//               the current size is size()
// postcondition: size() == newSize.  If newSize > oldsize then
//                the current capacity of vector is newSize; otherwise
//                capacity isn't changed. for each k
//                such that 0 <= k <= min(mySize, newSize), vector[k]
//                is a copy of the original; other elements of vector are
//                initialized using the 0-argument  constructor
{    
    if (newSize < mySize)
    {
        mySize = newSize;   // capacity doesn't "shrink"
        return;
    }
         // allocate new storage and copy element into new storage

    int k;
    Type * newList = new Type[newSize];
    for(k=0; k < mySize; k++)
    {
        newList[k] = myList[k];
    }
    delete [] myList;                      // de-allocate old storage
    myCapacity = mySize = newSize;         // assign new storage/size
    myList = newList;
}

Type & CTVector::back()
{
  assert(size() > 0);
  return (*this)[size()-1];
}

void CTVector::reserve(int size)
{
    // punt to resize in current implementation
    
    int oldSize = mySize;
    if (size > myCapacity)
    {
        resize(size);
    }
    mySize = oldSize;
}


void CTVector::clear()
{
    mySize = 0;
}

CTVector::iterator CTVector::begin()
{
  return myList;
}

CTVector::iterator CTVector::end()
{
  return myList + size();
}
