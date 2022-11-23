#ifndef __dwislpy_util_
#define __dwislpy_util_

//
// dwislpy-util.hh
//
// Some useful utilities for the DWISLPY interpreter. Some are for error
// reporting, namely
//
//  * DwislpyError    - an exception for reporting DWISLPY errors
//  * Locn         - a (filename, line number, column number) for an error
//  * dwislpy_message - builds an error string 
//
// Some are for converting string literals to their actual strings, and
// back, namely
//
//   * de_escape, re_escape
//

//
// class Locn
//
// Houses information about the DWISLPY source file's name, along with a
// line and column within that source file.
//
// This is typically used to report errors in the DWISLPY source code.
//
class Locn {
public:
    std::string source_name;
    int line;
    int column;
    //
    Locn(std::string fn, int li, int co)
        : source_name {fn}, line {li}, column {co} { }
    Locn(std::string fn) 
        : source_name {fn}, line {-1}, column {-1} { }
    Locn(void) : Locn {"",0,0} { }
};
    
//
// s = dwislpy_message(lo,ms);
//
// Builds and returns a string `s` that gives a DWISLPY error message
// `ms` along with information `lo` about the place in the source file
// where the error occurs.
//
const std::string dwislpy_message(Locn lo, std::string ms);

//
// class DwislpyError
//
// Thrown when an error is discovered while processing a DWISLPY source
// file.
//
class DwislpyError: public std::exception {
private:
    Locn location;
    const std::string message;
    
public:    
    DwislpyError(Locn lo, std::string ms);
    virtual const char* what() const noexcept;
};

//
// Utility functions for dealing with string literals.
//
std::string re_escape(std::string s); // Replace special chars with \d ones.
std::string de_escape(std::string s); // Replace \d sequences with actuals.

struct none { };
extern none None;

#endif
