#include <iostream>
#include <fstream>
#include <cstring>

#include "dwislpy-ast.hh"
#include "dwislpy-flex.hh"
#include "dwislpy-bison.tab.hh"
#include "dwislpy-util.hh"
#include "dwislpy-main.hh"

//
// dwslpyc - a DWISLPY compiler
//
// Usage: ./dwislpyc <DWISLPY source file name>
//
// This command compiles a DWISLPY program into MIPS source. If the
// source file's name is `foo.py` (or `foo.slpy` etc.) It will
// generate the MIPS source `foo.s`. This source can be run using the
// SPIM text-based MIPS32 emulator.
//
// The code is heavily reliant upon:
//
// * dwislpy-ast.{cc,hh} - defines the AST for our language
// * dwislpy-check.{cc,hh} - annotates the AST in prep for compilation
// * dwislpy-inst.{cc,hh} - defines the IR, performs translation/compilation
//

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
    program->run();
}

// check
//
// Runs the DwiSlpy program.
//
void DWISLPY::Driver::check(void) {
    program->chck();
}

// compile
//
// Runs the DwiSlpy program.
//
void DWISLPY::Driver::compile(void) {
    std::ofstream out_stream { };
    size_t thedot = src_name.find_last_of("."); 
    std::string out_name = src_name.substr(0, thedot) + ".s"; 
    out_stream.open(out_name);
    program->compile(out_stream);
    out_stream.close();
}

// dump
//
// Outputs the DwiSlpy program, either by depicting its AST, or by
// a "pretty" version that mimics the original source code.
//
void DWISLPY::Driver::dump(bool pretty) {
    if (pretty) {
        program->output(std::cout);
    } else {
        program->dump();
    }
}

char* extract_filename(int argc, char** argv) {
    for (int i=1; i<argc; i++) {
        if (argv[i][0] != '-') return argv[i];
    }
    return nullptr;
}

// * * * * * 
//
// main - the DWISLPY interpreter
//
int main(int argc, char** argv) {
    
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
            // Check.
            //
            dwislpy.check();
            
            //
            // Compile.
            //
            dwislpy.compile();
            
        } catch (DwislpyError se) {
            
            //
            // Report the error.
            //
            std::cerr << se.what() << std::endl;

        } 
    } else {
        //
        // Give some command line help.
        //
        std::cerr << "usage: "
                  << argv[0]
                  << std::endl;
    }
}
