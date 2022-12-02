#ifndef _DWISLPY_INST_HH
#define _DWISLPY_INST_HH

//
// Objects used for compilation and assembly of a DwiSlpy program
// to a MIPS program.
//
// It defines subclasses of INST. These are pseudo-instructions

// during its staged conversion to a MIPS program.
//

#include <vector>
#include <utility>
#include <string>
#include <memory>
#include "dwislpy-check.hh"

class INST;
typedef std::shared_ptr<INST> INST_ptr;
typedef std::vector<INST_ptr> INST_vec;

//
//
// ************************************************************

//
// class INST
//
// A sequence of pseudo-instructions forms the intermediate
// representation for the staged conversion of a program's code
// to its assembled MIPS code.
//
// Pseudo-instructions are, for the most part, very nearly the same as
// MIPS machine instructions except, rather than operating on MIPS
// registers, the can operate on arbirary "temporary" variables. And
// so their work isn't directly tied to a specific processor.  Below
// this class definition are derived subclasses for the variety of
// pseudo-instructions, like ADD, SET, BLT, JMP, etc. that directly
// correspond to machine instructions. Others like ENTER, LEAVE, RTN,
// ARG, CLL, RTV are used for managing calls amongst the components of
// an assembled program. There are also LBL instructions for labelling
// code sections for JMP, BLT, CLL, etc.
//
// We've also included pseudo-instructions like GTS, PTI, PTS, CAT, REP,
// and others that will require system calls (for input/output) or for
// working with strings.
//
// Note that the constructors of INST subclasses do little work other
// than fill in the struct's info from the parameters.
//
// Methods of INST:
// ----------------
//
// * toMIPS - This converts the pseudo-instruction into a sequence of
//            MIPS instructions, outputting them to the give output
//            stream. This is performed in PASS 3 of compilation.
//
// This method takes a SymT object which contains information for
// assembling each function component of the program, namely the stack
// frame locations of each variable and temporary. It also tracks
// whole-program information like string constants.
//

class INST {
public:
  virtual void toMIPS(std::ostream& os, const SymT& assm) const = 0;
};

typedef std::shared_ptr<INST> INST_ptr;
typedef std::vector<INST_ptr> INST_vec;

//
// Basic pseudo-instructions.
//
//   SET d,v     - sets a temporary to an integer value.
//   STL d,l     - sets a temporary to the address of a label.
//   MOV d,s     - copies one temporary's value to another's.
//   ADD d,s1,s2 - sums two temporaries into another.
//   NOP         - does nothing; "no operation"
//
class SET : public INST {
public:
    std::string dst;
    int val;
    SET(std::string d, int v) : dst {d}, val {v} { }
    virtual ~SET(void) = default;
    void toMIPS(std::ostream& os, const SymT& assm) const;
};

class STL : public INST {
public:
    std::string dst;
    std::string lbl;
    STL(std::string d, std::string l) : dst {d}, lbl {l} { }
    virtual ~STL(void) = default;
    void toMIPS(std::ostream& os, const SymT& assm) const;
};

class MOV : public INST {
public:
    std::string dst;
    std::string src;
    MOV(std::string d, std::string s) : dst {d}, src {s} {}
    virtual ~MOV(void) = default;
    void toMIPS(std::ostream& os, const SymT& assm) const;
};

class ADD : public INST {
public:
    std::string dst;
    std::string src1;
    std::string src2;
    ADD(std::string d, std::string s1, std::string s2) : dst {d}, src1 {s1}, src2 {s2} {}
    virtual ~ADD(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};

class SUB : public INST {
public:
    std::string dst;
    std::string src1;
    std::string src2;
    SUB(std::string d, std::string s1, std::string s2) : dst {d}, src1 {s1}, src2 {s2} {}
    virtual ~SUB(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};

class NOP : public INST {
public:
    NOP(void) { } 
    virtual ~NOP(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};


//
// Pseudo-instructions for jumping around within the code.
//
//   LBL l              - labels a series of pseudo-instructions
//   JMP l              - jump unconditionally
//   BCN cn,s1,s2,lt,lf - branch according to a condition
//                        cn is "lt", "eq", "le"
//   BCZ cn,s,lt,lf - branch according to a comparison against 0
//                        cn is "ltz", "eqz", "lez"
//
class LBL : public INST {
public:
    std::string lbl;
    LBL(std::string l) : lbl {l} {}
    virtual ~LBL(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};

class BCN : public INST {
public:
    std::string cndn; // One of "lt", "eq", "le"
    std::string src1;
    std::string src2;
    std::string lblt;
    std::string lblf;
    BCN(std::string cn, std::string  s1, std::string s2,
        std::string lt, std::string lf) :
        cndn {cn}, src1 {s1}, src2 {s2}, lblt {lt}, lblf {lf} {}
    virtual ~BCN(void) = default;
    virtual void toMIPS(std::ostream& os, const SymT& symt) const;
};

class BCZ : public INST {
public:
    std::string cndn; // One of "ltz", "eqz", "lez"
    std::string src;
    std::string lblt;
    std::string lblf;
    BCZ(std::string cn, std::string  s, std::string lt, std::string lf) :
        cndn {cn}, src {s}, lblt {lt}, lblf {lf} {}
    virtual ~BCZ(void) = default;
    virtual void toMIPS(std::ostream& os, const SymT& symt) const;
};

class JMP : public INST {
public:
    std::string lbl;
    JMP(std::string l) : lbl {l} {}
    virtual ~JMP(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};

//
// Pseudo-instructions for the called function.
//
// ENTER - sets up call stack frame for the function.
// RTN s - sets the return value
// LEAVE - takes down frame; returns
// 
//
class ENTER : public INST {
public:
    ENTER(void) {}
    virtual ~ENTER(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};

class RTN : public INST {
public:
    std::string src;
    RTN(std::string s) : src {s} {}
    virtual ~RTN(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};

class LEAVE : public INST {
public:
    LEAVE(void) {}
    virtual ~LEAVE(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};

//
// Pseudo-instructions for calling a function.
//
// ARG i,s - sets the i-th argument for a call
// CLL l   - calls the labelled function code
// RTV d   - gets the returned value
// 
//
class ARG : public INST {
public:
    int idx;
    std::string src;
    ARG(int i, std::string s) : idx {i}, src {s} {}
    virtual ~ARG(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};

class RTV : public INST {
public:
    std::string dst;
    RTV(std::string d) : dst {d} {}
    virtual ~RTV(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};

class CLL : public INST {
public:
    std::string lbl;
    CLL(std::string l) : lbl {l} {}
    virtual ~CLL(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};

//
// Pseudo-instructions for system calls.
//
// GTI d - Gets a integer of input into d.
// PTI s - Outputs the integer value of s.
// PTS s - Outputs a string sitting at an address s.
//
//
class GTI : public INST {
public:
    std::string dst;
    GTI(std::string dest) : dst {dest} {} 
    virtual ~GTI(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};

class PTI : public INST {
public:
    std::string src;
    PTI(std::string s) : src {s} { } 
    virtual ~PTI(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};

class PTS : public INST {
public:
    std::string src;
    PTS(std::string srce) : src {srce} { } 
    virtual ~PTS(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};


//
// Pseudo-instructions for commenting the generated code.
//
class CMT : public INST {
public:
    std::string msg;
    CMT(std::string m) : msg {m} {}
    virtual ~CMT(void) = default;
    void toMIPS(std::ostream& os, const SymT& symt) const;
};


#endif

