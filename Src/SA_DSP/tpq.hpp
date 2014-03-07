
CTPQueue::CTPQueue()
	: myNumElts(0)
{
	init();
}

void CTPQueue::init(int n)
	// postcondition:  list reserves space for n elements
	//                 myList[0] contains garbage heap starts at myList[1]
{
	myList.reserve(n);
	Type temp;
	myList.push_back(temp);
}



CTPQueue::~CTPQueue()
	// postcondition: priority queue is "garbage"     
{
}



int CTPQueue::size() const
	// postcondition: returns number of items queued up
{
	return myNumElts;
}


int CTPQueue::empty() const
	// postcondition: returns true if pqueue is empty, else false
{
	return myNumElts == 0;
}



void CTPQueue::push(const Type & elt)
{
	myList.push_back(elt);  // increase size of heap
	myNumElts++;            // add at end (heap shape)
	int k = myNumElts;      // location of new element

	while (k > 1 && myList[k/2] < elt)
	{
		myList[k] = myList[k/2];
		k /= 2;
	}
	myList[k] = elt;
}


void CTPQueue::pop()
	// precondition: ! empty()     
	// postcondition: remove maximal element from priority queue     
{
	assert(!empty());
	myList[1] = myList[myNumElts];   // move last to top
	myList.pop_back();               // reduce size of heap
	myNumElts--;
	if (myNumElts > 1)
		heapify(1);                      // and push down
}


Type& CTPQueue::top()
	// postcondition: set ref/prio to maximal element in heap     
{
	return myList[1];
}



void CTPQueue::heapify(int vroot)

	// preconditon: subheaps of vroot satisfy heap property (and shape)
	// postcondition: heap rooted at vroot satisfies heap property

{
	Type last = myList[vroot];
	int child, k = vroot;

	while (2*k <= myNumElts)
	{
		// find maximal child (assume left, then check right)

		child = 2*k;  
		if (child < myNumElts && myList[child] < myList[child+1])
		{
			child++;
		}
		if (!(last < myList[child]))  // it goes here
		{
			break;
		}
		else
		{
			myList[k] = myList[child];
			k = child;
		}
	}
	// found "resting place", push 'last element'

	myList[k] = last;
}
