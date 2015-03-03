#pragma once

class CCmdTarget;

/**
* A simple class to manage a wait cursor
* 1) The instance is declared on the stack.
* 2) When the instance goes out of scope, the wait cursor
*    is automatically cleaned up
*/
class CScopedCursor {
public:
    CScopedCursor(CCmdTarget * parent);
    ~CScopedCursor();
private:
    CCmdTarget * parent;
};

