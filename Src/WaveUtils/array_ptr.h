#ifndef ARRAY_PTR_H
#define ARRAY_PTR_H

/**
* a template class to manage a pointer of an array of t
*/

template<class T> class array_ptr {
public:
    explicit array_ptr(size_t size = 0) :
        pointer(NULL),
        length(size) {
        if (length>0) {
            pointer = new T[length];
            memset(pointer,0,length);
        }
    }

    explicit array_ptr(T * ptr, size_t size) :
        pointer(ptr),
        length(size) {
    }

    ~array_ptr() {
        if (pointer!=NULL) {
            delete [] pointer;
            pointer = NULL;
        }
    }

    T & operator*() const {
        return *pointer;
    }

    T * operator->() const {
        return pointer;
    }

	T & operator[](size_t index) const {
		return pointer[index];
	}

    T * get() const {
        return pointer;
    }

    size_t size() const {
        return length;
    }

    void reallocate(size_t size) {
        if (pointer!=NULL) {
            delete [] pointer;
        }
        length = size;
        pointer = new T[length];
        memset(pointer,0,length);
    }

	/**
	* get rid of the current data and use the new
	* pointer
	*/
	void reassign( T * ptr, size_t size) {
        if (pointer!=NULL) {
            delete [] pointer;
        }
        length = size;
        pointer = ptr;
	}

	/**
	* move data between to instances of array_ptr
	*/
	void transfer( array_ptr<T> & src) {
        if (pointer!=NULL) {
            delete [] pointer;
        }
		length = src.size();
		pointer = src.get();
		src.reassign(NULL,0);
	}

private:
    T * pointer;
    size_t length;
};

#endif
