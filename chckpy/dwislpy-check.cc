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
    for (Stmt_ptr stmt : stmts) { // Inspect the block line by line...

        // Check this statement.
        [[maybe_unused]]
        Rtns stmt_rtns = stmt->chck(expd, defs, symt);

		//if(stmt_rtrns)
//TODO: this 
        // Tosses out return behavior. Fix this !!!!
    }

    return expd; // Fix this!!
}

Rtns Asgn::chck([[maybe_unused]] Rtns expd, Defs& defs, SymT& symt) {
    if (!symt.has_info(name)) {
        throw DwislpyError(where(), "Variable '" + name + "' never introduced.");
    }
    Type name_ty = symt.get_info(name)->type; // Look at symt to see what info is recorded for type of this variable.
    Type expn_ty = expn->chck(defs,symt);
    if (name_ty != expn_ty) { // The type stored in symt better match the type deduced by the expn->chck...
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

	// add the name to the symt (sym table)
	symt.add_locl(name);
	// and check if type of expression is same as type of introduced thing
	if(type != expn->chck(defs,symt)){
		throw DwislpyError { where(), "Type mismatch: Expression type is inconsistent with introduced variable type." };
	}
    return Rtns {Void {}};
}

Rtns FRtn::chck(Rtns expd, Defs& defs, SymT& symt) { // Function return
	Type subexp_ty = expn->chck(defs, symt);
	Type expd_ty = type_of(expd);
	if(subexp_ty != expd_ty){
		std::string msg = "Type mismatch: returned type inconsistent with expected type.";
		throw DwislpyError { where(), msg };
	}
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
	Type cndn_ty = cndn->chck(defs, symt);
	if(!is_bool(cndn_ty)){
		std::string msg = "Type Error: IfElse condition is not bool!";
		throw DwislpyError(where(), msg);
	}
    // It should check each of the two blocks return behavior.
	Rtns then_blck_rtn = then_blck ->chck(defs, symt);
	Rtns else_blck_rtn = else_blck ->chck(defs, symt);
//	Rtns summary = then_blck_rtn && else_blck_rtn;
//	if(else_blck_rtn == then_blck_rtn){
//
//	}
		// The above four lines all give me errors... I can't even == compare two Rtns :^(
    // It should summarize the return behavior.
    //
    return Rtns {Void {}}; // Fix this!!!

}

Rtns Whle::chck(Rtns expd, Defs& defs, SymT& symt) {
    //
    // This should check that the condition is a bool.
	Type cndn_ty = cndn->chck(defs, symt);
	if(!is_bool(cndn_ty)){
		std::string msg = "Type Error: While condition is not bool!";
		throw DwislpyError(where(), msg);
	}
    // It should check the block's return behavior.
	Rtns  blck_rtn = blck->chck(defs, symt);
    // It should summarize the return behavior. It shouldv be Void
    // or VoidOr because loop bodies don't always execute.
		//... Isn't blck_rtn already "summarized" return behavior? there's only one blck to *have* behavior, how much more can we summarize...
			// Should we check whether it *isn't* Void/VoidOr??
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
    Type left_ty = left->chck(defs,symt);
    Type rght_ty = rght->chck(defs,symt);
    if (is_int(left_ty) && is_int(rght_ty)) {
        return Type {IntTy {}};
    } else if (is_str(left_ty) && is_str(rght_ty)) {
        return Type {StrTy {}};
    } else {
        std::string msg = "Wrong operand types for minus.";
        throw DwislpyError { where(), msg };
    }
  }

Type Tmes::chck(Defs& defs, SymT& symt) {
    Type left_ty = left->chck(defs,symt);
    Type rght_ty = rght->chck(defs,symt);
    if (is_int(left_ty) && is_int(rght_ty)) {
        return Type {IntTy {}};
    } else if (is_str(left_ty) && is_str(rght_ty)) {
        return Type {StrTy {}};
    } else {
        std::string msg = "Wrong operand types for times.";
        throw DwislpyError { where(), msg };
    }
  }

Type IDiv::chck(Defs& defs, SymT& symt) {
    Type left_ty = left->chck(defs,symt);
    Type rght_ty = rght->chck(defs,symt);
    if (is_int(left_ty) && is_int(rght_ty)) {
        return Type {IntTy {}};
    } else if (is_str(left_ty) && is_str(rght_ty)) {
        return Type {StrTy {}};
    } else {
        std::string msg = "Wrong operand types for IDiv.";
        throw DwislpyError { where(), msg };
    }
  }

Type IMod::chck(Defs& defs, SymT& symt) {
    Type left_ty = left->chck(defs,symt);
    Type rght_ty = rght->chck(defs,symt);
    if (is_int(left_ty) && is_int(rght_ty)) {
        return Type {IntTy {}};
    } else if (is_str(left_ty) && is_str(rght_ty)) {
        return Type {StrTy {}};
    } else {
        std::string msg = "Wrong operand types for IMod.";
        throw DwislpyError { where(), msg };
    }
  }

Type Less::chck(Defs& defs, SymT& symt) { //chossing how to deal w boolean??
	Type left_ty = left->chck(defs,symt);
	Type rght_ty = rght->chck(defs,symt);
	if (is_int(left_ty) && is_int(rght_ty)) {
		//return Type {IntTy {}};
		return Type {BoolTy {}};
	} else if (is_str(left_ty) && is_str(rght_ty)) {
		//return Type{StrTy{}};
		return Type {BoolTy {}};
	}else if (is_bool(left_ty) && is_bool(rght_ty)) { // does this suffice for dealing with bools?
		return Type {BoolTy {}};
	} else {
		std::string msg = "Wrong operand types for Less.";
		throw DwislpyError { where(), msg };
	}
}

Type LsEq::chck(Defs& defs, SymT& symt) { // same as above
	Type left_ty = left->chck(defs,symt);
	Type rght_ty = rght->chck(defs,symt);
	if (is_int(left_ty) && is_int(rght_ty)) {
		//return Type {IntTy {}};
		return Type {BoolTy {}};
	} else if (is_str(left_ty) && is_str(rght_ty)) {
		//return Type{StrTy{}};
		return Type {BoolTy {}};
	}else if (is_bool(left_ty) && is_bool(rght_ty)) {
		return Type {BoolTy {}};
	} else {
		std::string msg = "Wrong operand types for LessEquals check.";
		throw DwislpyError { where(), msg };
	}
}

Type Equl::chck(Defs& defs, SymT& symt) {
	Type left_ty = left->chck(defs,symt);
	Type rght_ty = rght->chck(defs,symt);
	if (is_int(left_ty) && is_int(rght_ty)) {
		return Type {BoolTy {}};
	} else if (is_str(left_ty) && is_str(rght_ty)) {
		return Type {BoolTy {}};
	}else if (is_bool(left_ty) && is_bool(rght_ty)) {
		return Type {BoolTy {}};
	} else {
		std::string msg = "Wrong operand types for Equals check.";
		throw DwislpyError { where(), msg };
	}
}

Type And::chck(Defs& defs, SymT& symt) {
	Type left_ty = left->chck(defs,symt);
	Type rght_ty = rght->chck(defs,symt);
	if (is_int(left_ty) && is_int(rght_ty)) {
		return Type {BoolTy {}};
	} else if (is_str(left_ty) && is_str(rght_ty)) {
		return Type {BoolTy {}};
	}else if (is_bool(left_ty) && is_bool(rght_ty)) {
		return Type {BoolTy {}};
	} else {
		std::string msg = "Wrong operand types for And check.";
		throw DwislpyError { where(), msg };
	}
}

Type Or::chck(Defs& defs, SymT& symt) {
	Type left_ty = left->chck(defs,symt);
	Type rght_ty = rght->chck(defs,symt);
	if (is_int(left_ty) && is_int(rght_ty)) {
		return Type {BoolTy {}};
	} else if (is_str(left_ty) && is_str(rght_ty)) {
		return Type {BoolTy {}};
	}else if (is_bool(left_ty) && is_bool(rght_ty)) {
		return Type {BoolTy {}};
	} else {
		std::string msg = "Wrong operand types for Or check.";
		throw DwislpyError { where(), msg };
	}
}

Type Not::chck(Defs& defs, SymT& symt) {
	Type left_ty = left->chck(defs,symt);
	Type rght_ty = rght->chck(defs,symt);
	if (is_int(left_ty) && is_int(rght_ty)) {
		return Type {BoolTy {}};
	} else if (is_str(left_ty) && is_str(rght_ty)) {
		return Type {BoolTy {}};
	}else if (is_bool(left_ty) && is_bool(rght_ty)) {
		return Type {BoolTy {}};
	} else {
		std::string msg = "Wrong operand types for Not check.";
		throw DwislpyError { where(), msg };
	}
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

	Type subexpn_check  = expn->chck(defs, symt); // does this suffice to check subexpressions?
		// Should I check the result of subexpn ???
	if(is_str(subexpn_check)){ // Sure...
		return Type {StrTy {}};
	}else{
		std::string msg = "Bad use of input().";
		throw DwislpyError { where(), msg };
	}

}

Type IntC::chck(Defs& defs, SymT& symt) {

	Type subexpn_ty = expn->chck(defs, symt);
	if(is_int(subexpn_ty) || is_bool(subexpn_ty)){
		return Type {IntTy {}};
	}else{
		std::string msg = "Type Error: expected int or bool.";
		throw DwislpyError { where(), msg };
	}

}

Type StrC::chck(Defs& defs, SymT& symt) { // anything can convert to str

	Type subexpn_check = expn->chck(defs, symt);
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
