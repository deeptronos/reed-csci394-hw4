#include <iostream>
#include <fstream>
#include <cstring>

#include "dwislpy-ast.hh"
#include "dwislpy-flex.hh"
#include "dwislpy-bison.tab.hh"
#include "dwislpy-util.hh"
#include "dwislpy-main.hh"

//
// dwislpy - a DWISLPY ("Def While If + Straight-Line PYthon") interpreter.
//
// Usage: ./dwislpy [--test] [--dump [--pretty]] <DWISLPY source file name>
//
// This implements a Unix command for processing a DWISLPY program.  By
// default, it executes a DWISLPY program. There are command-line flags
// for other activities, namely:
//
//    --dump - echo back the (parsed) source code instead of running it.
//
//    --pretty - do it prettily
//
//    --test - give a simple ERROR message when an error occurs.
//
// The code is heavily reliant upon:
//
// * dwislpy-ast.{cc,hh} - defines the AST for DWISLPY programs
// * dwislpy-flex.{cc,hh} - converts the source into a feed of tokens
// * dwislpy-bison.{cc,hh} - parses a DWISLPY token stream
//
// The latter two work in tandem as a Flex/Bison-based lexer/parser duo.
//
// The interpreter works as a result of the `run` method of `Prgm` AST
// nodes, the `exec` method of `Stmt` AST nodes, and the `eval` method
// of `Expn` AST nodes. See `dwislpy-ast.cc` for details on each of these
// methods.
//
// The interpreter is housed as a DWISLPY::Driver object, which also
// houses the lexer and parser (each written using Flex and Bison).
// See the .hh file for details on it, and see the methods below.
//
// * * * * * 
//
// Some utilities for extracting information from the command line.
//

bool check_flag(int argc, char** argv, std::string flag) {
    for (int i=1; i<argc; i++) {
        if (strcmp(flag.c_str(),argv[i]) == 0) return true;
    }
    return false;
}

char* extract_filename(int argc, char** argv) {
    for (int i=1; i<argc; i++) {
        if (argv[i][0] != '-') return argv[i];
    }
    return nullptr;
}

// * * * * *
//
// DWISLPY::Driver methods.
//
// This class just houses the components of the DWISLPY interpreter.
// We wrote it this way just to prevent some deallocation errors
// due to sharing pointers in our interfacing with Flex/Bison, programs
// built in a more permissive error (and originally for C).
//

DWISLPY::Driver::Driver(std::string filename) :
    src_name {filename}
{
    src_stream = istream_ptr { new std::ifstream { src_name } };
}

// parse
//
// Checks the file's stream, builds the lexer for it, then parses the
// file's contents. The parser sets the `prgm` AST using the `set`
// method.
//
void DWISLPY::Driver::parse(void) {
    if (src_stream->fail()) {
        Locn locn {src_name};
        std::string mesg = "Unable to open file. Does the file exist?";
        throw DwislpyError {locn, mesg};
    }
    lexer = Lexer_ptr { new DWISLPY::Lexer { src_stream.get(), src_name } };
    DWISLPY::Lexer& lexer_local = *lexer;
    parser = Parser_ptr { new DWISLPY::Parser { lexer_local, *this } };
    parser->parse();
}

// run
//
// Runs the DwiSlpy program.
//
void DWISLPY::Driver::run(void) {
    main->run();
}

// run
//
// Outputs the DwiSlpy program, either by depicting its AST, or by
// a "pretty" version that mimics the original source code.
//
void DWISLPY::Driver::dump(bool pretty) {
    if (pretty) {
        main->output(std::cout);
    } else {
        main->dump();
    }
}


// * * * * * 
//
// main - the DWISLPY interpreter
//
int main(int argc, char** argv) {
    
    //
    // Process the command-line, including any flags.
    //
    bool dump   = check_flag(argc,argv,"--dump");
    bool pretty = false;
    if (dump) {
        pretty = check_flag(argc,argv,"--pretty");
    }
    bool testing   = check_flag(argc,argv,"--test");
    char* filename = extract_filename(argc,argv);
    
    if (filename) {
        DWISLPY::Driver dwislpy { filename };
        //
        // Catch DWISLPY errors.
        //
        try {
            
            //
            // Parse.
            //
            dwislpy.parse();

            //
            // Either dump or run the parsed code.
            //
            if (dump) {
                dwislpy.dump(pretty);
            } else {
                dwislpy.run();
            }
            
        } catch (DwislpyError se) {
            
            if (testing) {
                //
                // If --test flag then just give "ERROR" message.
                //
                std::cout << "ERROR" << std::endl;
            } else {
                //
                // Otherwise, report the error.
                //
                std::cerr << se.what() << std::endl;
            }
        } 
    } else {
        //
        // Give some command line help.
        //
        std::cerr << "usage: "
                  << argv[0]
                  << " [--dump [--pretty]] [--test] file"
                  << std::endl;
    }
}
