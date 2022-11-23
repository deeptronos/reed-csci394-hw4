#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "dwislpy-check.hh"
#include "dwislpy-ast.hh"
#include "dwislpy-util.hh"

bool is_int(Type type) {
    return std::holds_alternative<IntTy>(type);
}

bool is_str(Type type) {
    return std::holds_alternative<StrTy>(type);
}

bool is_bool(Type type) {
    return std::holds_alternative<BoolTy>(type);
}

bool is_None(Type type) {
    return std::holds_alternative<NoneTy>(type);
}

bool operator==(Type type1, Type type2) {
    if (is_int(type1) && is_int(type2)) {
        return true;
    }
    if (is_str(type1) && is_str(type2)) {
        return true;
    }
    if (is_bool(type1) && is_bool(type2)) {
        return true;
    }
    if (is_None(type1) && is_None(type2)) {
        return true;
    }
    return false;
}

bool operator!=(Type type1, Type type2) {
    return !(type1 == type2);
}

std::string type_name(Type type) {
    if (is_int(type)) {
        return "int";
    }
    if (is_str(type)) {
        return "str";
    }
    if (is_bool(type)) {
        return "bool";
    } 
    if (is_None(type)) {
        return "None";
    }
    return "wtf";
}

unsigned int Defn::arity(void) const {
    return symt.get_frmls_size();
}

Type Defn::returns(void) const {
    return rety;
}

SymInfo_ptr Defn::formal(int i) const {
    return symt.get_frml(i);
}

void Prgm::chck(void) {
    for (std::pair<Name,Defn_ptr> dfpr : defs) {
        dfpr.second->chck(defs);
    }
    Rtns rtns = main->chck(Rtns{Void {}},defs,main_symt);
    if (!std::holds_alternative<Void>(rtns)) {
        DwislpyError(main->where(), "Main script should not return.");
    }
}

void Defn::chck(Defs& defs) {
    Rtns rtns = body->chck(Rtns{rety}, defs, symt);
    if (std::holds_alternative<Void>(rtns)) {
        throw DwislpyError(body->where(), "Definition body never returns.");
    }
    if (std::holds_alternative<VoidOr>(rtns)) {
        throw DwislpyError(body->where(), "Definition body might not return.");
    }
}

Type type_of(Rtns rtns) {
    if (std::holds_alternative<VoidOr>(rtns)) {
        return std::get<VoidOr>(rtns).type;
    }
    if (std::holds_alternative<Type>(rtns)) {
        return std::get<Type>(rtns);
    }
    return Type {NoneTy {}}; // Should not happen.
}

Rtns Blck::chck(Rtns expd, Defs& defs, SymT& symt) {
    
    // Scan through the statements and check their return behavior.
    for (Stmt_ptr stmt : stmts) {
        
        // Check this statement.
        [[maybe_unused]]
        Rtns stmt_rtns = stmt->chck(expd, defs, symt);

        // Tosses out return behavior. Fix this !!!!
    }            

    return expd; // Fix this!!
}

Rtns Asgn::chck([[maybe_unused]] Rtns expd, Defs& defs, SymT& symt) {
    if (!symt.has_info(name)) {
        throw DwislpyError(where(), "Variable '" + name + "' never introduced.");
    }
    Type name_ty = symt.get_info(name)->type;
    Type expn_ty = expn->chck(defs,symt);
    if (name_ty != expn_ty) {
        std::string msg = "Type mismatch. Expected expression of type ";
        msg += type_name(name_ty) + ".";
        throw DwislpyError {expn->where(), msg};
    }
    return Rtns {Void {}};
}

Rtns Pass::chck([[maybe_unused]] Rtns expd,
                [[maybe_unused]] Defs& defs,
                [[maybe_unused]] SymT& symt) {
    return Rtns {Void {}};
}

Rtns Prnt::chck([[maybe_unused]] Rtns expd, Defs& defs, SymT& symt) {
    [[maybe_unused]] Type expn_ty = expn->chck(defs,symt);
    return Rtns {Void {}};
}

Rtns Ntro::chck([[maybe_unused]] Rtns expd, Defs& defs, SymT& symt) {
    return Rtns {Void {}}; // Fix this!!
}

Rtns FRtn::chck(Rtns expd, Defs& defs, SymT& symt) {
    // Doesn't check the sub-expression. Fix this!!!
    return expd;
}

Rtns PRtn::chck(Rtns expd,
                [[maybe_unused]] Defs& defs, [[maybe_unused]] SymT& symt) {
    if (std::holds_alternative<Void>(expd)) {
        throw DwislpyError {where(), "Unexpected return statement."};
    }
    Type expd_ty = type_of(expd);
    if (!is_None(expd_ty)) {
        throw DwislpyError {where(), "A procedure does not return a value."};
    }
    return Rtns {Type {NoneTy {}}};
}

Rtns PCll::chck([[maybe_unused]] Rtns expd, Defs& defs, SymT& symt) {
    //
    // This should look up a procedure's definition. It should check:
    // * that the correct number of arguments are passed.
    // * that the return type is None
    // * that each of the argument expressions type check
    // * that the type of each argument matches the type signature
    //
    // Fix this!!!
    return Rtns {Void {}};
}

Rtns IfEl::chck(Rtns expd, Defs& defs, SymT& symt) {
    //
    // This should check that the condition is a bool.
    // It should check each of the two blocks return behavior.
    // It should summarize the return behavior.
    //
    return Rtns {Void {}}; // Fix this!!!

}

Rtns Whle::chck(Rtns expd, Defs& defs, SymT& symt) {
    //
    // This should check that the condition is a bool.
    // It should check the block's return behavior.
    // It should summarize the return behavior. It shouldv be Void
    // or VoidOr because loop bodies don't always execute.
    return Rtns {Void {}}; // Fix this!!!
}

Type Plus::chck(Defs& defs, SymT& symt) {
    Type left_ty = left->chck(defs,symt);
    Type rght_ty = rght->chck(defs,symt);
    if (is_int(left_ty) && is_int(rght_ty)) {
        return Type {IntTy {}};
    } else if (is_str(left_ty) && is_str(rght_ty)) {
        return Type {StrTy {}};
    } else {
        std::string msg = "Wrong operand types for plus.";
        throw DwislpyError { where(), msg };
    }
}

Type Mnus::chck(Defs& defs, SymT& symt) {
    // Doesn't check subexpressions.
    // Fix this!!
    return Type {IntTy {}}; 
}

Type Tmes::chck(Defs& defs, SymT& symt) {
    // Doesn't check subexpressions.
    // Fix this!!
    return Type {IntTy {}}; 
}

Type IDiv::chck(Defs& defs, SymT& symt) {
    // Doesn't check subexpressions.
    // Fix this!!
    return Type {IntTy {}}; 
}

Type IMod::chck(Defs& defs, SymT& symt) {
    // Doesn't check subexpressions.
    // Fix this!!
    return Type {IntTy {}}; 
}

Type Less::chck(Defs& defs, SymT& symt) {
    // Doesn't check subexpressions.
    // Fix this!!
    return Type {BoolTy {}}; 
}

Type LsEq::chck(Defs& defs, SymT& symt) {
    // Doesn't check subexpressions.
    // Fix this!!
    return Type {BoolTy {}}; 
}

Type Equl::chck(Defs& defs, SymT& symt) {
    // Doesn't check subexpressions.
    // Fix this!!
    return Type {BoolTy {}}; 
}

Type And::chck(Defs& defs, SymT& symt) {
    // Doesn't check subexpressions.
    // Fix this!!
    return Type {BoolTy {}}; 
}

Type Or::chck(Defs& defs, SymT& symt) {
    // Doesn't check subexpressions.
    // Fix this!!
    return Type {BoolTy {}}; 
}

Type Not::chck(Defs& defs, SymT& symt) {
    // Doesn't check subexpressions.
    // Fix this!!
    return Type {BoolTy {}}; 
}

Type Ltrl::chck([[maybe_unused]] Defs& defs, [[maybe_unused]] SymT& symt) {
    if (std::holds_alternative<int>(valu)) {
        return Type {IntTy {}};
    } else if (std::holds_alternative<std::string>(valu)) {
        return Type {StrTy {}};
    } if (std::holds_alternative<bool>(valu)) {
        return Type {BoolTy {}};
    } else {
        return Type {NoneTy {}};
    } 
}

Type Lkup::chck([[maybe_unused]] Defs& defs, SymT& symt) {
    if (symt.has_info(name)) {
        return symt.get_info(name)->type;
    } else {
        throw DwislpyError {where(), "Unknown identifier."};
    } 
}

Type Inpt::chck(Defs& defs, SymT& symt) {
    // Doesn't check subexpressions.
    // Fix this!!
    return Type {StrTy {}}; 
}

Type IntC::chck(Defs& defs, SymT& symt) {
    // Doesn't check subexpressions.
    // Fix this!!
    return Type {IntTy {}}; 
}

Type StrC::chck(Defs& defs, SymT& symt) {
    // Doesn't check subexpressions.
    // Fix this!!
    return Type {StrTy {}}; 
}


Type FCll::chck(Defs& defs, SymT& symt) {
    //
    // This should look up a function's definition. It should check:
    // * that the correct number of arguments are passed.
    // * that each of the argument expressions type check
    // * that the type of each argument matches the type signature
    // It should report the return type of the function.
    //
    // Fix this!!!
    return Type {NoneTy {}}; // Fix this!!
}
