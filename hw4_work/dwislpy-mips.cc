#include <iostream>
#include <fstream>
#include "dwislpy-inst.hh"
#include "dwislpy-ast.hh"
#include "dwislpy-check.hh"
#include "dwislpy-util.hh"

//
// dwislpy-mips.cc
//
// This gives the code for compiling the IR into MIPS32 code, outputting
// it to a provided `std::ostream`. At the top level, it defines
//
//     Prgm::compile
//
// which relies on
//
//     defn_compile
//
// to produce MIPS32 code for every `def` body and for the `main`
// script.
//
// These functions, in turn, rely on `INST::toMIPS` which is
// implemented for any sub-class of `INST`.
//

#define RETURN_ADDRESS "saved_return_address"
#define FRAME_POINTER  "saved_frame_pointer"

// compile_defn(os,symt,code)
//
// Generate MIPS32 code into `os`, relying on `symt` to figure out
// frame locations of variables and temporaries. This sets up the
// frame information, marking things in the `symt`, then walks through
// `code` and converts each IR instruction (using `toMIPS`) into
// MIPS32 code.
//
void compile_defn(std::ostream& os, SymT& symt, INST_vec& code) {
    int num_frmls = symt.get_frmls_size();
    int num_locls = symt.get_locls_size();
    int num_cargs = 4; // Max # of args of any F/PCll within this def.

    // Calculate a double-word aligned frame size.
    int frame_size = 4*(num_locls + num_cargs + 2);
    if (frame_size % 8 != 0) {
        frame_size += 4;
    }
    
    //
    // Frame layout according to calling conventions.
    //
    // See: https://courses.cs.washington.edu/courses/cse410/
    //              09sp/examples/MIPSCallingConventionsSummary.pdf
    //
    
    // Formal parameters sit above the frame.
    for (int i = 0; i < num_frmls; i++) {
        std::string frml = symt.get_frml(i)->name;
        symt.set_frame_offset(frml,i*4);
    }

    int offset = -4;
        
    // Locals sit next.
    for (int i = 0; i < num_locls; i++) {
        std::string locl = symt.get_locl(i)->name;
        symt.set_frame_offset(locl,offset);
        offset -= 4;
    }

    // Saved registers sit next.
    std::string ra = symt.add_locl(RETURN_ADDRESS, IntTy {}); // Not really an integer.
    std::string fp = symt.add_locl(FRAME_POINTER, IntTy {});  // Not really an integer.
    symt.set_frame_offset(ra,offset);
    offset -= 4;
    symt.set_frame_offset(fp,offset);
    offset -= 4;

    // Possible arguments to calls sit last.
    
    symt.set_frame_size(frame_size);

    for (INST_ptr inst : code) {
        inst->toMIPS(os,symt);
    }
}

// Prgm::compile(os)
//
// Generate MIPS32 code into `os`, relying on `compile_defn` to generate
// the machine code for each of the `def`s and the `main` script. It also
// sets up the global information about all the string constants that were
// discovered duting translation to the IR. 
//
// The resulting file (represented by `os`) will contain a SPIM-executable
// .s file.
//
void Prgm::compile(std::ostream& os) {

    // Translate the AST to IR.
    //
    trans();

    // Generate the `.data` section filled with string constants.
    //
    os << "\t.data" << std::endl;
    for (std::pair<Name,std::string> lbl_strg : glbl_symt_ptr->strings) {
        std::string lbl = lbl_strg.first;
        std::string strg = "\"" + re_escape(lbl_strg.second) + "\"";
        os << lbl << ":" << std::endl;
        os << "\t.asciiz " << strg << std::endl;
    }
    
    // Generate the `.text` section filled with `main` and each `def`'s
    // (labelled) code.
    //
    os << "\t.text" << std::endl;
    os << "\t.globl main" << std::endl;
    compile_defn(os,main_symt,main_code);
    for (std::pair<Name,Defn_ptr> dfpr : defs) {
        Defn_ptr defn = dfpr.second;
        compile_defn(os,defn->symt,defn->code);
    }
}

//
// INST::toMIPS(os,symt)
//
// Method for generating MIPS code that performs the work of a
// pseudo-instruction (an object derived from class INST).
//
// The method outputs a series of MIPS instructions to the output
// stream `os`, using information about frame variables and strings
// held in `symt`. In particular, it loads variables from the stack
// frame at their assigned offset within the frame to set some MIPS
// registers and/or stores register values to the stack frame if they
// are being updated.
// 
// We define this method for each subclass of INST.
//
//
void ENTER::toMIPS(std::ostream& os, const SymT& symt) const {
    int ra_slot = symt.get_frame_offset(RETURN_ADDRESS);
    int fp_slot = symt.get_frame_offset(FRAME_POINTER);
    os << "\t" << "sw $ra," << ra_slot << "($sp)" << std::endl;
    os << "\t" << "sw $fp," << fp_slot << "($sp)" << std::endl;
    os << "\t" << "move $fp, $sp" << std::endl;
    os << "\t" << "addi $sp,$sp,-" << symt.get_frame_size() << std::endl;
    for (int argi = 0; argi < symt.get_frmls_size(); argi++) {
        std::string pram = symt.get_frml(argi)->name;
        int slot = symt.get_frame_offset(pram);
        os << "\t" << "sw $a" << argi << "," << slot << "($fp)" << std::endl;
    }
}
//
void LEAVE::toMIPS(std::ostream& os, const SymT& symt) const {
    int ra_slot = symt.get_frame_offset(RETURN_ADDRESS);
    int fp_slot = symt.get_frame_offset(FRAME_POINTER);
    os << "\t" << "lw $ra," << ra_slot << "($fp)" << std::endl;
    os << "\t" << "lw $fp," << fp_slot << "($fp)" << std::endl;
    os << "\t" << "addi $sp,$sp," << symt.get_frame_size() << std::endl;
    os << "\t" << "jr $ra" << std::endl;
}
void SET::toMIPS(std::ostream& os, const SymT& symt) const {
  os << "\t" << "li $t0," << val << std::endl;
  os << "\t" << "sw $t0," << symt.get_frame_offset(dst) << "($fp)" << std::endl;
}
//
void STL::toMIPS(std::ostream& os, const SymT& symt) const { 
    os << "\t" << "la $t0," << lbl << std::endl;
    os << "\t" << "sw $t0," << symt.get_frame_offset(dst) << "($fp)" << std::endl;
}
//
void MOV::toMIPS(std::ostream& os, const SymT& symt) const {
  os << "\t" << "lw $t1," << symt.get_frame_offset(src) << "($fp)" << std::endl;
  os << "\t" << "move $t0,$t1" << std::endl;
  os << "\t" << "sw $t0," << symt.get_frame_offset(dst) << "($fp)" << std::endl;
}
//
void RTV::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t" << "move $t0,$v0" << std::endl;
    os << "\t" << "sw $t0," << symt.get_frame_offset(dst) << "($fp)" << std::endl;
}
//
void GTI::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t" << "li $v0,5" << std::endl;
    os << "\t" << "syscall" << std::endl;
    os << "\t" << "sw $v0," << symt.get_frame_offset(dst) << "($fp)" << std::endl;
}
//
void NOP::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t" << "nop" << std::endl;
}
//
void PTI::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t" << "lw $a0," << symt.get_frame_offset(src) << "($fp)" << std::endl;
    os << "\t" << "li $v0,1" << std::endl;
    os << "\t" << "syscall" << std::endl;
}
//
void PTS::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t" << "li $v0,4" << std::endl;
    os << "\t" << "lw $a0," << symt.get_frame_offset(src) << "($fp)" << std::endl;
    os << "\t" << "syscall" << std::endl;
}
//
void ADD::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t" << "lw $t1," << symt.get_frame_offset(src1) << "($fp)" << std::endl;
    os << "\t" << "lw $t2," << symt.get_frame_offset(src2) << "($fp)" << std::endl;
    os << "\t" << "add $t0,$t1,$t2" << std::endl;
    os << "\t" << "sw $t0," << symt.get_frame_offset(dst) << "($fp)" << std::endl;
}
//
void SUB::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t" << "lw $t1," << symt.get_frame_offset(src1) << "($fp)" << std::endl;
    os << "\t" << "lw $t2," << symt.get_frame_offset(src2) << "($fp)" << std::endl;
    os << "\t" << "sub $t0,$t1,$t2" << std::endl;
    os << "\t" << "sw $t0," << symt.get_frame_offset(dst) << "($fp)" << std::endl;
}
//
void RTN::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t" << "lw $v0," << symt.get_frame_offset(src) << "($fp)" << std::endl;
}
//
void BCN::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t" << "lw $t1," << symt.get_frame_offset(src1) << "($fp)" << std::endl;
    os << "\t" << "lw $t2," << symt.get_frame_offset(src2) << "($fp)" << std::endl;
    os << "\t" << "b" << cndn << " $t1,$t2," << lblt << std::endl;
    os << "\t" << "j " << lblf << std::endl;
}
//
void BCZ::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t" << "lw $t1," << symt.get_frame_offset(src) << "($fp)" << std::endl;
    os << "\t" << "b" << cndn << " $t1," << lblt << std::endl;
    os << "\t" << "j " << lblf << std::endl;
}
//
void JMP::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t" << "j " << lbl << std::endl;
}
//
void CLL::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t" << "jal " << lbl << std::endl;
}
//
void LBL::toMIPS(std::ostream& os, const SymT& symt) const {
    os << lbl << ":" << std::endl;
}
//
void CMT::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t\t\t\t#" << msg << std::endl;
}
//
void ARG::toMIPS(std::ostream& os, const SymT& symt) const {
    os << "\t" << "lw $a" << idx << ","
       << symt.get_frame_offset(src) << "($fp)" << std::endl;
}
