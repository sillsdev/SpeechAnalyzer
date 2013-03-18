#ifndef _IUtf8String_H_
#define _IUtf8String_H_



class IUtf8String {
public:
    virtual std::string utf8() const = 0; // return temporary utf8 copy of current string
    virtual void setUtf8(const char * pUtf8) = 0; // set contents of string from utf8 source
};



#endif