#ifndef _dwislpy_main_hh
#define _dwislpy_main_hh

// dwislpy-main.hh
//
// Object classes and types to support the main driver of the DwiSlpy
// interpreter. This invents a `Driver` class for the `DWISLPY` name-
// space. It houses the lexer, parser, and the interpreter components.
// It basically provides a housing for invoking these components. It
// also provides a top-level object that can be manipulated within the
// Bison parser code.
//
// Note: this housing was mainly added as a deliberate way of dealing
// with the mish-mash of C-like C++ interfaces invented by the C++
// variants of Flex and Bison. At issue was the sharing of pointers
// to their underlying components without C++ performing its own
// decisions about cleaning up their referenced objects. By building
// a "driver" object with some shared pointers to these components, we
// followed an idiom suggested by some C++ example code. This resulted
// in a cover to the various parse/run/etc. components that drive the
// DwiSlpy tolchain. This also provides a means to have the Bison parse
// code access the top-level components of the interpreter.
//

#include <string>
#include <istream>

#include "dwislpy-flex.hh"
#include "dwislpy-bison.tab.hh"

typedef std::shared_ptr<DWISLPY::Lexer> Lexer_ptr;
typedef std::shared_ptr<DWISLPY::Parser> Parser_ptr;
typedef std::shared_ptr<std::istream> istream_ptr;

/*
 * class DWISLPY::Driver
 * 
 * Used by `main` to invoke the interpreter components. Also used by
 * the Bison code to set up the AST as a result of the parse.
 *
 * The methods it provides are:
 *   parse - runs the parser, building the AST
 *   set - sets the AST that results from a parse
 *   run - executes the parsed DwiDlpy program
 *   dump - (pretty) prints the AST
 *
 * Note that the constructor attempts to create a stream attached to
 * the provided name of the DwiSlpy source file. However, the success
 * of that operation is only checked when `parse` is called.
 */

namespace DWISLPY {
    
    class Driver {
    public:
        Driver(std::string filename);
        void parse(void);
        void run(void);
        void check(void);
        void compile(void);
        void dump(bool pretty);
        void set(Prgm_ptr prgm) { program = prgm; }
        std::string src_name;
    private:
        istream_ptr src_stream = nullptr;
        Prgm_ptr    program = nullptr;
        Lexer_ptr   lexer = nullptr;
        Parser_ptr  parser  = nullptr;
    };

}

#endif 
