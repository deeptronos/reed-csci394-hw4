#ifndef _DWISLPY_TYPE_H
#define _DWISLPY_TYPE_H

//
// dwislpy-check.hh
//
// Defines `Type`, `Rtns`, and `SymT` used by the DWISLPY checking code.
//
// These are used to support type checking, return behavior checking,
// and other semantic analysis of a parsed DWISLPY program.
//
// For example, to check the AST of a Plus operation, the method
// call prototype is
//
//      Type Plus::chck(Defs& defs, Symt& symt);
//
// Because we use `symt` to look up the types of any variables used by
// the Plus expression, and we use `defs` to look up the type
// signature of any function called by the Plus expression. And then
// we report a `Type` of that expression. (It could be `int` or it
// could be `str`, depending on its arguments.)
//
// Another example: to check the return behavior of a Blck of statements
// forming the body of a function definition we call the method with
// prototype:
//
//     Rtns Blck::chck(Rtns expd, Defs& defs, Symt& symt);
//
// This tells that code that it should expect the return behavior `expd`
// when it scans through the code (something like "this should return a
// boolean value"), looking up the types of functions and procedures
// within `defs`. It tracks and records the types of variables within
// `symt`. Once done, it reports the return behavior `Rtns`.
//
// These C++ types are defined below. `Type` and `Rtns` are each a C++
// variant. `SymT` is essentially a dictionary of a variable's type
// information, stored as a `SymInfo_ptr` attached to the name of that
// variable.
//

#include <iostream>
#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

// * * * * *
//
// Type - the type of DWISLPY types.
//
// This defines the four DWISLPY types `int`, `str`, `bool`, and `None`.
// The C++ type `Type` is a variant that could be any of these four kinds
// of C++ values.
//
// We use these in our type checking of DWISLPY. The code for
// `Plus::chck` for example looks to see if its left and right
// sub-expressions are of type Type { IntTy {} } and then, if that is
// indeed the case, reports that its resulting value will be of type
// Type { IntTy {} }.
//

class IntTy {};
class StrTy {};
class BoolTy {};
class NoneTy {};
typedef std::variant<IntTy,StrTy,BoolTy,NoneTy> Type;

// s = type_name(t)
//
// Used to report the DWISLPY type of variables, values, and expressions.
//
std::string type_name(Type t);

// t1 == t2, t1 != t2
//
// Check whether Type t1 is the same as Type t2.
//
bool operator==(Type type1, Type type2);
bool operator!=(Type type1, Type type2);

// b = is_int(t); b = is_str(t); b = is_bool(t); b = is_None(t)
//
// Checks which alternative a Type is.
//
bool is_int(Type type);
bool is_str(Type type);
bool is_bool(Type type);
bool is_None(Type type);

// * * * * *
//
// Rtns - the type of DWISLPY return behaviors.
//
// This defines the result of our retun behavior analysis of a program.
// There are three possible behaviors:
//
// 1. A block of code does not have a `return` statement, and so execution
//    of the code "falls through."
// 2. A block of code returns a value, and that value is of type `t`.
// 3. A block of code could return a value of type `t` and it also could
//    fall through.
//
// We encode these three behaviors with a C++ variant. They each have the
// corresponding expression:
//
// 1. Rtns { Void {} }
// 2. Rtns { t }  for some Type t.
// 3. Rtns { VoidOr {t} } for some type t.
//
// For example, the DWISLPY block:
//
//     x : int = 42
//     print(x * x)
//     if int(input("?")) + x > 100:
//         return 42
//     else:
//         print("No")
//
// Can be seen to have the return behavior Rtns { IntTy {} }
//
class Void { };
class VoidOr {
public:
    VoidOr (Type ty) : type {ty} { }
    Type type;
};
typedef std::variant<Void,VoidOr,Type> Rtns;


// * * * * *
// 
// SymInfo - the type of symbol tables used by our semantic analysis.
//
// When we check a DWISLPY program, we record the names and types of all
// the variables used within a definition statement block and used with
// the main script's statement block.
//
// A block can have two kinds of variables:
//  * a formal parameter - this is an argument passed to a function/procedure
//  * a local variable - this is any variable introduced within a block
// In preparation for code generation, we provide support for a third kind:
//  * a temporary - introduced as storage for intermediate calculations
//
// A symbol table collects information about each variable, currently
// * its `name`
// * its DWISLPY `type`
// * what `kind` of variable is it: a formal, a local, or a temp
//
// Since there could be several different variables of the same name within
// a block of statements (for example, a parameter x and then a different,
// introduced x within the body) we provide support for distinguishing variables
// by an integer index `identifier`.
//
// You can add variables to symbol table using `add_frml`, `add_locl`, `add_temp`.
// You can check the symbol table with `has_info`.
// You can get a variable's information with `get_info`.
//
// The last yields a pointer to a struct of type `SynInfo` which contains the
// four pieces of information associated with a variable. This struct can be
// edited by modifying the attributes via this pointer.
//
// The collection of formal pararmeters can also be accessed by an integer
// index. The method `get_frml` allows you to access a function's 1st, 2nd,
// 3rd, etc parameter's information. The method `get_frmls_size` tells you
// how many formal parameters are stored in a symbol table.
//

enum SymKind { FRML, LOCL, TEMP };

//
// class SymInfo - struct of the variable information stored 
//
class SymInfo {
public:
    std::string name;
    int identifier;
    Type type;
    SymKind kind;
    int frame_offset;
    SymInfo(std::string nm, Type ty, int id, SymKind kd) :
        name {nm}, identifier {id}, type {ty}, kind {kd} {}
};

class SymT;
typedef std::shared_ptr<SymInfo> SymInfo_ptr;
typedef std::shared_ptr<SymT> SymT_ptr;

//
// class SymT - convenient cover to a dictionary of Name-SymInfo_ptr pairs.
//
class SymT {
public:
    std::unordered_map<std::string, std::string> strings;
    SymT() : sym_table {}, formals {}, globals {nullptr} { }
    std::string add_frml(std::string nm, Type ty) {
        sym_table[nm] = SymInfo_ptr{ new SymInfo {nm, ty, 0, FRML} };
        formals.push_back(nm);
        return nm;
    }
    std::string add_locl(std::string nm, Type ty) {
        sym_table[nm] = SymInfo_ptr{ new SymInfo {nm, ty, sym_id++, LOCL} };
        locals.push_back(nm);
        return nm;
    }
    std::string add_temp(std::string nm, Type ty) {
        sym_table[nm] = SymInfo_ptr{ new SymInfo {nm, ty, sym_id++, TEMP} };
        locals.push_back(nm);
        return nm;
    }
    std::string add_temp(Type ty) {
        int id = sym_id++;
        std::string nm = "temp_" + std::to_string(id);
        sym_table[nm] = SymInfo_ptr{ new SymInfo {nm, ty, id, TEMP} };
        locals.push_back(nm);
        return nm;
    }
    void set_parent(SymT_ptr p) {
        globals = p;
    }               
    std::string add_labl(std::string nm) {
        if (globals == nullptr) {
            return nm;
        } else {
            return globals->add_labl(nm);
        }
    }
    std::string add_labl() {
        if (globals == nullptr) {
            int id = sym_id++;
            std::string nm = "L_" + std::to_string(id);
            return add_labl(nm);
        } else {
            return globals->add_labl();
        }
    }
    std::string add_strg(std::string strg) {
        if (globals == nullptr) {
            std::string labl = this->add_labl();
            strings[labl] = strg;
            return labl;
        } else {
            return globals->add_strg(strg);
        }
    }
    bool has_info(std::string nm) const {
        return (sym_table.count(nm) > 0);
    }
    SymInfo_ptr get_info(std::string nm) const {
        return sym_table.at(nm);
    }
    SymInfo_ptr get_locl(int i) const {
        return sym_table.at(locals[i]);
    }
    SymInfo_ptr get_frml(int i) const {
        return sym_table.at(formals[i]);
    }
    unsigned int get_frmls_size(void) const {
        return formals.size();
    }
    unsigned int get_locls_size(void) const {
        return locals.size();
    }
    void set_frame_offset(std::string nm, int offset) {
        get_info(nm)->frame_offset = offset;
    }
    int get_frame_offset(std::string nm) const {
        return get_info(nm)->frame_offset;
    }
    void set_frame_size(int sz) {
        frame_size = sz;
    }
    int get_frame_size(void) const {
        return frame_size;
    }
private:
    std::unordered_map<std::string, SymInfo_ptr> sym_table;
    std::vector<std::string> formals;
    std::vector<std::string> locals;
    SymT_ptr globals;
    int sym_id = 0;
    int frame_size;
};


#endif
