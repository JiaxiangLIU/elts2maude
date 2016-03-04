/*********************************************************************************
 * Copyright   : Copyright (c) 2013-2015. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 *   Street, Fifth Floor, Boston, MA 02110-1301, USA
 *
 *   To contact the program development board, email to <ctsvthss@gmail.com>.
 *
 * FileName    : GrammarModel.h
 *
 * Author      : Dexi Wang
 *
 * LastUpdate  : 2013-12-03
 *
 * Description : Defines the grammar model
 ********************************************************************************/
#ifndef GRAMMARMODEL_H_
#define GRAMMARMODEL_H_

#include <cassert>
#include <ostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>

#define LEN_DEFAULT_INT 32

//===============================================================================
// Classes Predefines
//===============================================================================
class Parse_StringNode;
class Parse_VarRef;
class Parse_ExpressionType;
class Parse_Expression;
class Parse_Action;
class Parse_Label;
class Parse_Transition;
class Parse_Var;
class Parse_Module;
class Parse_VarGRef;
class Parse_LocationVarGRef;
class Parse_LocationExpression;
class Grammar_Model;

class Parse_Context;
class ParseError;

// DW: extern the global variables for Parser.ypp parsing
extern Grammar_Model *g_grammarModel;
extern std::map<std::string, Parse_ExpressionType*> g_i2t;
extern Parse_Context *g_context;

// DW: in this version I deleted Parse_PropertyExpression class definition
// DW: and made it a declaration of Parse_Expression
typedef Parse_Expression Parse_PropertyExpression;
typedef Parse_PropertyExpression Parse_Property;

//===============================================================================
// Global Functions
//===============================================================================
// used for print a sentence of the grammar tree in the automatic alignment format
void LevelPrint(std::ostream& output, int level);

// defien all the types of an action block
enum ActionBlockType {
	E_DO
};

enum Parse_TypeConversion {
    ETC_NONE, ETC_VALUE, ETC_ZEXT, ETC_SEXT, ETC_FEXT, ETC_BITCAST, ETC_UNSIGNED
};

enum Parse_Function {
    PF_NONE,
    PF_FINITE, PF_FINITEF, PF_FINITEL,
    PF_FPCLASSIFY, PF_FPCLASSIFYF,
    PF_ISINF, PF_ISINFF, PF_ISINFL,
    PF_ISNAN, PF_ISNANF, PF_ISNANL,
    PF_SIGNBIT, PF_SIGNBITF, PF_SIGNBITL,
    PF_FESETROUND
};

// print an action block
void PrintActionBlock(std::ostream& output, int level,
		std::vector<Parse_Action*>& actionBlock, ActionBlockType type);

// CG : global grammar model feature
class Parse_Context {
public:
    bool m_hasHexRealConstant = false;

    // DW: real multiplication may equal in FPA
    // DW: e.g., float-no-simp7_true-unreach-call.i
    bool m_hasRealConstantMultiple = false;

    // DW: for cases like NAN comparing, you should only use FPA theory
    // DW: e.g., nan_double_false-unreach-call.i, float-no-simp7_true-unreach-call.i
    bool m_hasRealCE = false;

    // DW: check nonlinear operations
    bool m_hasNonlinearMultiple = false;

    // DW: check FPA function
    bool m_hasFPAFunction = false;

    // DW: FIXME, temp fix for sine_* series
    bool m_hasSINEHEX = false;
    bool m_hasSINECompare = false;

    // DW: FIXME, temp fix for NEWTON25
    bool m_hasNEWTON = false;
    bool m_hasMinusOne = false;
    int m_countMultiple = 0;

    // DW: FIXME, temp fix for inv_square_int_true, ISIT
    bool m_hasISIT2 = false;
    bool m_hasISIT10 = false;

    // DW: FIXME, temp fix for x86_fp80 constant
    bool m_has80Constant = false;

    // DW: FIXME, T_F_FESETROUND
    bool m_hasFESETROUND = false;

    // CG: has bitcast between different type, indicate bitvector mode should be used
    bool m_hasBitCastBetweenDifferentType = false;
};

// CG: parse exception
class ParseError : public std::runtime_error {
    int _first_line, _first_column, _last_column;
    const char * _yytext;
    const char *_s;
    public:
        ParseError(int first_line, int first_column, int last_column, const char * yytext, const char * s) :
            _first_line(first_line), _first_column(first_column), _last_column(last_column), _yytext(yytext), _s(s), std::runtime_error("")  {}
        virtual const char* what() const noexcept {
            std::ostringstream msg;
            msg << "Error: LINE " << _first_line << " COLUMN " << _first_column << " - " << _last_column << ", \"" << _yytext << "\" has " << _s;
            return msg.str().c_str();
        }
};

//===============================================================================
// String node for any word in the source file
//===============================================================================
class Parse_StringNode {
public:
	std::string m_node; 		// word name
	int m_lineNumber; 	// line of the word

    int accessId; //FIXME:used by beagle-abs
    std::string module; //FIXME:used by beagle-abs

public:
	// constructor
	Parse_StringNode() {
        accessId = 0;
	}

    Parse_StringNode(const Parse_StringNode& str) : m_node(str.m_node), m_lineNumber(str.m_lineNumber), module(str.module) {
        accessId = str.accessId;
    }

	// overloaded constructor
	Parse_StringNode(const std::string &s, int n) :
			m_node(s), m_lineNumber(n) {
        accessId = 0;
	}

    Parse_StringNode(const std::string &s) : Parse_StringNode(s, 0) {}

	// get node
	std::string& GetNode() {
		return m_node;
	}

	//get line number
	int GetLineNumber() {
		return m_lineNumber;
	}

	// print the std::string node
	void Print(std::ostream& output);

    // FIXME used by beagle-abs
    bool equal(const Parse_StringNode *op1) {
        //std::cout << "here";
        assert(op1);
        return this->m_node == op1->m_node;
    }
};

//===============================================================================
// -Define a variable reference. For example: p1.a, where p1 is a module and
//  a is a variable
//===============================================================================
class Parse_VarRef {
public:
	Parse_StringNode* m_moduleName; // module name
	Parse_StringNode* m_varName; 	// variable name

public:
	// constructor
	Parse_VarRef() :
		m_moduleName(NULL), m_varName(NULL) {
	}

    Parse_VarRef(const Parse_VarRef& ref) : m_moduleName(NULL), m_varName(NULL) {
        if (ref.m_moduleName != NULL) {
            m_moduleName = new Parse_StringNode(*ref.m_moduleName);
        }
        if (ref.m_varName != NULL) {
            m_varName = new Parse_StringNode(*ref.m_varName);
        }
    }

	// print the variable reference
	void Print(std::ostream& output);

    //FIXME used by beagle-abs
    bool equal(const Parse_VarRef *op1) {
        assert(op1);
        return op1->m_varName == this->m_varName
                && op1->m_moduleName == this->m_moduleName;
    }
};

//===============================================================================
// Define a boolean expression
//===============================================================================
enum Parse_Operator {
	// DW: new style Parse_Operator
	// DW: you can also see operation precedence decreases line by line

    // DW: please note that "subscript" operator "[]" has a very high precedence, ref: http://en.cppreference.com/w/cpp/language/operator_precedence
    E_SUBSCRIPT, 

	E_NOT, E_TYPE_CONVERSION, E_FUNCTION, 
	E_MULTIPLE, E_DIVIDE, E_UDIVIDE, E_MOD, E_UMOD,
	E_ADD, E_MINUS,
	E_ASHIFTR, E_LSHIFTR, E_SHIFTL,
	E_SLT, E_SLE, E_SGT, E_SGE, E_ULT, E_ULE, E_UGT, E_UGE, E_CE, E_NE, 
	E_LAND, E_LXOR, E_LOR, 
	E_AND, E_OR, 
    // DW: in sv-smt/src/SMTChecker/SMTChecker.cpp, line 1477, "E_OR" is used to determine operators' bound, pay attension when you modify behind "E_OR"

    // DW: this E_EQU should be deleted, it duplicates with E_CE
    // DW: at least we should not use it any more
	//E_EQU,

	// DW: end token, i.e., tokens that are not really "operators"
	E_CONSTANT, E_NONDET, E_VAR, E_VARREF, 
	E_GVARREF, E_LOCATIONEXPR
};

typedef Parse_Operator Parse_PropertyOperator;

//===============================================================================
// Define a var type
// DW: actually the type of the string literal, so we put bool, int, real here
// DW: INT is for short, long, long long, and so on
// DW: REAL is for float, double, and so on
// DW: REAL_E is forr float, double with E, e.g., 1e10, -1E-10
// DW: please note that these are just prefix, the actual C-like type relies on the length of Parse_ExpressionType
// DW: e.g., INT 32 is C int, INT 8 is C char, REAL 32 is C float
//===============================================================================
enum Parse_VarType {
	VTE_BOOL, VTE_INT, VTE_REAL, VTE_REAL_E, 
    VTE_BOOL_ARRAY, VTE_INT_ARRAY, VTE_REAL_ARRAY, 
    VTE_STRUCT
};

enum Parse_ExpressionFormat {
    EF_NORMAL, EF_HEX, EF_REAL_E, EF_REAL_P, EF_HEX_K
};

// DW: typical length: bool(1), char(8), int(32), float(32), double(64)
// DW: length is number of bits, -1 means error, 0 means it's an constant
class Parse_ExpressionType {
public:
	Parse_VarType m_typePrefix;
    // DW: 20151015, for VTE_*_ARRAY, the length is the size of an array (for now)
	std::string m_typeLength;

    // DW: "format" is something like "0xFFE5A", "1e307", "0x1p2"
    // DW: and of couse it defaults to "normal"
    Parse_ExpressionFormat m_format = EF_NORMAL;

    // DW: only appears in m_typeToBe ot Parse_Expression, stores information on how to convert in TTF
    Parse_TypeConversion m_conversion = ETC_NONE;

    // CG: copy constructor
    Parse_ExpressionType(const Parse_ExpressionType & other) : m_typePrefix(other.m_typePrefix), m_typeLength(other.m_typeLength), m_format(other.m_format) {}

	// DW: basic ctor
	Parse_ExpressionType(Parse_VarType p, std::string l) : m_typePrefix(p), m_typeLength(l) {}
	Parse_ExpressionType(Parse_VarType p, const char *l) : m_typePrefix(p), m_typeLength(std::string(l)) {}

	// DW: if only given type, evaluate the length
	Parse_ExpressionType(Parse_VarType p, bool isConstant = false) : m_typePrefix(p) {
		if (p == VTE_BOOL) {
			m_typeLength = "1";
		} else if (p == VTE_INT) {
			// DW: default to C int type
			m_typeLength = "32";
		} else if (p == VTE_REAL) {
			// DW: default to C double type
			m_typeLength = "64";
		} else if (p == VTE_REAL_E) {
			// DW: default to C double type
			m_typeLength = "64";
		} else {
			// DW: means illegal
			m_typeLength = "-1";
		}

        if (isConstant) {
            m_typeLength = "0";
        }
	}

	// DW: copy from another ET
	Parse_ExpressionType(Parse_ExpressionType *et) : m_typePrefix(et->m_typePrefix), m_typeLength(et->m_typeLength), m_format(et->m_format) {}

    // DW: compare strictly, i.e., prefix and length should be same
    bool hasSameTypeAs(Parse_ExpressionType *et) {
        // DW: type REAL is same as REAL_E
        if (this->m_typePrefix == VTE_REAL_E) {
            return ((et->m_typePrefix == VTE_REAL) || (et->m_typePrefix == VTE_REAL_E)) && (this->m_typeLength == et->m_typeLength);
        }

        if (et->m_typePrefix == VTE_REAL_E) {
            return ((this->m_typePrefix == VTE_REAL) || (this->m_typePrefix == VTE_REAL_E)) && (this->m_typeLength == et->m_typeLength);
        }

        return (this->m_typePrefix == et->m_typePrefix) &&(this->m_typeLength == et->m_typeLength);
    }

    // DW: same prefix
    bool hasSamePrefixAs(Parse_ExpressionType *et) {
        // DW: type REAL is same as REAL_E
        if (this->m_typePrefix == VTE_REAL_E) {
            return ((et->m_typePrefix == VTE_REAL) || (et->m_typePrefix == VTE_REAL_E));
        }

        if (et->m_typePrefix == VTE_REAL_E) {
            return ((this->m_typePrefix == VTE_REAL) || (this->m_typePrefix == VTE_REAL_E));
        }
        return (this->m_typePrefix == et->m_typePrefix);
    }

    // DW: is "higher", has larger prefix or length or both
    // DW: please note that "equal" will return false
    bool isHigherThan(Parse_ExpressionType *et) {
        if (this->m_typePrefix > et->m_typePrefix) {
            return true;
        } else if (this->m_typePrefix < et->m_typePrefix) {
            return false;
        }

        std::stringstream ss1(this->m_typeLength), ss2(et->m_typeLength);
        int l1 = 0, l2 = 0;
        ss1 >> l1;
        ss2 >> l2;
        if (l1 > l2) {
            return true;
        } else {
            return false;
        }
    }

    bool isConstant() const {
        return this->m_typeLength == "0";
    }

    // DW: "this" is the type of the element 
    // DW: return value is the type of the array containing the element
    Parse_ExpressionType *getArrayTypeFromElementType() {
        // DW: defaults to VET_INT_ARRAY
        Parse_VarType elementTypePrefix = VTE_INT_ARRAY;
        if (this->m_typePrefix == VTE_BOOL) {
            elementTypePrefix = VTE_BOOL_ARRAY;
        } else if (this->m_typePrefix == VTE_INT) {
            elementTypePrefix = VTE_INT_ARRAY;
        } else if (this->m_typePrefix == VTE_REAL) {
            elementTypePrefix = VTE_REAL_ARRAY;
        }

        return new Parse_ExpressionType(elementTypePrefix, this->m_typeLength);
    }

    // DW: "this" is the type of the array
    // DW: return value is the type of the element
    Parse_ExpressionType *getElementTypeFromArrayType() {
        // DW: defaults to VET_INT
        Parse_VarType elementTypePrefix = VTE_INT;
        if (this->m_typePrefix == VTE_BOOL_ARRAY) {
            elementTypePrefix = VTE_BOOL;
        } else if (this->m_typePrefix == VTE_INT_ARRAY) {
            elementTypePrefix = VTE_INT;
        } else if (this->m_typePrefix == VTE_REAL_ARRAY) {
            elementTypePrefix = VTE_REAL;
        }

        return new Parse_ExpressionType(elementTypePrefix, this->m_typeLength);
    }

    Parse_ExpressionType *getCommonType(Parse_ExpressionType *et) {
        // DW: v-?-v are not needNewLength ones
        // DW: C-?-C, C-?-v and v-?-C are needNewLength ones, needs a new common length
        bool needNewLength = true;
        if (!this->isConstant() && !et->isConstant()) {
            needNewLength = false;
        }

        // DW: higher ones will be return candidate
        // DW: please make sure the new type is copied
        Parse_ExpressionType *re = NULL;
        if (this->isHigherThan(et)) {
            re = new Parse_ExpressionType(this);
        } else {
            re = new Parse_ExpressionType(et);
        }

        // DW: if return value is "constant" but the whole exp is needNewLength mode, we need specify length to it
        // DW: e.g., d = i + 1.0;, "i + 1.0" should have length since its an variable
        if (re->isConstant() && needNewLength) {
            re->m_typeLength = "32";
        }

        return re;
    }

    // DW: get string of prefix
    std::string getStringPrefix(bool neededByType = false, bool isUsingBV = false);

	// DW: get [prefix:length] string
	std::string getString();

    // DW: get prefixlength string for TTF
    // DW: exampels: Int, Bool, BitVec31, FPFloat, FPDouble
    std::string getStringType(bool isUsingBV = false);

    std::string getStringOfFormat();

	// DW: print [prefix:length] to ostream
	void Print(std::ostream& output);

    unsigned int getTypeLength() const {return (unsigned int) std::stoi(m_typeLength);}

    bool isReal() const { return m_typePrefix == VTE_REAL || m_typePrefix == VTE_REAL_E; }

    bool isBool() const { return m_typePrefix == VTE_BOOL; }

    bool operator!=(const Parse_ExpressionType & other) const noexcept {
        return this->m_typePrefix != other.m_typePrefix || this->m_typeLength != other.m_typeLength;
    }

    bool operator==(const Parse_ExpressionType & other) const noexcept {
        return this->m_typePrefix == other.m_typePrefix && this->m_typeLength == other.m_typeLength;
    }
};

class Parse_Expression {
public:
	Parse_Operator m_op; // op can be {"!", "&", "|", "==", "var", "varRef", "constant"}
	Parse_Expression* m_operand1; // make sense when m_op belong to {"!", "&", "|", "=="}
	Parse_Expression* m_operand2; // make sense when m_op belong to {"&", "|", "=="}
	Parse_StringNode* m_varId; 		// make sense when m_op equal "var"

	Parse_VarRef* m_varRef; 		// make sense when m_op equal "varRef"
	Parse_StringNode* m_constant; 	// make sense when m_op equal "constant"
	std::string module;

	// DW: added location expr from property
	// DW: moved from previous Parse_PropertyExpression
	Parse_LocationExpression *m_locationExpr;
	// DW: m_gVarRef is formerly known as m_varId
	Parse_VarGRef *m_gVarRef;

    // DW: the type of the whole expression
    // DW: 20150806 in my eyes, the other member "type" will e deleted in the future, we will only use this "m_type" as the only type
    Parse_ExpressionType *m_type;
    // DW: m_typeToBe is the type this expression will be TTFed to
    Parse_ExpressionType *m_typeToBe;

    // DW: built-in function type
    Parse_Function m_typeFunction = PF_NONE;

    // DW: conversion type, only available when expression type is E_TYPE_CONVERSION
    Parse_TypeConversion m_typeConversion = ETC_NONE;

    // DW: 20150806 this should be deleted in the future
    // DW: used by beagle-abs, also added as member for float support
    Parse_VarType type;

    std::vector<std::string> sync_module; // FIXME used by beagle-abs
    int accessId; // FIXME used by beagle-abs
public:
    Parse_Expression(const Parse_Expression& exp);

	Parse_Expression() :
			m_operand1(NULL), m_operand2(NULL), m_varId(NULL), m_varRef(NULL), m_constant(
			NULL),m_locationExpr(NULL), m_gVarRef(NULL), m_type(NULL), m_typeToBe(NULL), type(VTE_BOOL) {
        accessId = 0;
	}

	Parse_Expression(const Parse_Operator &s) :
			m_op(s), m_operand1(NULL), m_operand2(NULL), m_varId(NULL), m_varRef(NULL), m_constant(
            NULL),m_locationExpr(NULL), m_gVarRef(NULL), m_type(NULL), m_typeToBe(NULL), type(VTE_BOOL) {
        accessId = 0;
	}

	Parse_Expression(const Parse_Operator &s, bool Is_Int) :
			m_op(s), m_operand1(NULL), m_operand2(NULL), m_varId(NULL), m_varRef(NULL), m_constant(
            NULL),m_locationExpr(NULL), m_gVarRef(NULL), m_type(NULL), m_typeToBe(NULL), type(VTE_BOOL) {
        accessId = 0;
        if (Is_Int == true) {
            type = VTE_INT;
        }
	}

	// DW: this function should support various types including bool, int, and real
	Parse_Expression(const Parse_Operator &s, Parse_VarType t) :
			m_op(s), m_operand1(NULL), m_operand2(NULL), m_varId(NULL), m_varRef(NULL), m_constant(
            NULL),m_locationExpr(NULL), m_gVarRef(NULL), m_type(NULL), m_typeToBe(NULL), type(t) {
		// DW: what is accessId?
        accessId = 0;
	}

	// construct from a Parse_StringNode, it may be a variable or a constant
	Parse_Expression(Parse_StringNode* node);

    // FIXME used by beagle-abs
    bool equal(const Parse_Expression *op1) {
        Parse_Expression *op2 = this;
        assert(op1 && op2);
        //          std::cout << "op1:" << op1->module << endl;
        //          std::cout << "op2:" << op2->module << endl;
        if (op1->m_op != op2->m_op) // || op1->module != op2->module)
            return false;
        //std::cout << "is here\n";
        switch (op1->m_op) {
            case E_CONSTANT:
                return op1->m_constant->m_node == op2->m_constant->m_node;
            case E_VAR:
                return op1->m_varId->equal(op2->m_varId);
            case E_VARREF:
                return op1->m_varRef->equal(op2->m_varRef);
            case E_NOT:
                return op1->m_operand1->equal(op2->m_operand1);
            default:
                return op1->m_operand1->equal(op2->m_operand1)
                        && op1->m_operand2->equal(op2->m_operand2);
        }
    }

	// print the expression
	void Print(std::ostream& output) const;
    std::string to_string() const {
        std::ostringstream ss;
        Print(ss);
        return ss.str();
    }

    Parse_Expression(const Parse_Expression& left, const Parse_Expression& right, Parse_Operator opt, Parse_VarType _type) : m_op(opt),
    m_varId(nullptr), m_varRef(nullptr), m_constant(nullptr), type(_type) {
        m_operand1 = new Parse_Expression(left);
        m_operand2 = new Parse_Expression(right);
    }

    // CG: override operator for convinence
    Parse_Expression operator ==(const Parse_Expression& b) const {
        if (this->type == VTE_BOOL && b.type == VTE_BOOL) {
            if (this->m_op == E_CONSTANT && this->m_constant->GetNode() == "true") {
                return b;
            }
            if (b.m_op == E_CONSTANT && b.m_constant->GetNode() == "true") {
                return *this;
            }
        }
        return Parse_Expression(*this, b, E_CE, VTE_BOOL);
    }

    Parse_Expression operator &&(const Parse_Expression& b) const {
        if (this->m_op == E_CONSTANT && this->m_constant->GetNode() == "true") {
            return b;
        }
        return Parse_Expression(*this, b, E_AND, VTE_BOOL);
    }

    Parse_Expression operator ||(const Parse_Expression& b) const {
        if (this->m_op == E_CONSTANT && this->m_constant->GetNode() == "false") {
            return b;
        }
        return Parse_Expression(*this, b, E_OR, VTE_BOOL);
    }

    Parse_Expression operator >(const Parse_Expression& b) const {
        return Parse_Expression(*this, b, E_SGT, VTE_BOOL);
    }

    Parse_Expression operator +(const Parse_Expression& b) const {
        return Parse_Expression(*this, b, E_ADD, this->type);
    }
};

// DW: AT_NORMAL_VARREF is not used by Ceagle, but for compability with VCS it is reserved
// DW: in VCS, 0: "var = expression", 1: "varRef = expression"
enum Parse_ActionType {
    AT_NORMAL, AT_NORMAL_VARREF, AT_ARRAY, AT_STRUCT
}; 

//===============================================================================
// Define an action, which is always "var = expression" or "varRef = expression"
// DW: 20151229, but now, we have arrays
//===============================================================================
class Parse_Action {
public:
    // DW: 20151229, this old "m_type" should be deprecated
	//bool m_type; // 0: "var = expression", 1: "varRef = expression"
    Parse_ActionType m_type;

	Parse_StringNode* m_varId;	// make sense when m_type equal 0
	Parse_VarRef* m_varRef;		// make sense when m_type equal 1
	Parse_Expression* m_value; 	// right part of the action

    // DW: 20151229, array support
    Parse_Expression *m_index;

public:
	Parse_Action() :
			m_type(AT_NORMAL), m_varId(NULL), m_varRef(NULL), m_value(NULL), m_index(NULL){
	}

    // used by absref
    Parse_Action(const Parse_Action& act) : m_type(act.m_type), m_varId(NULL), m_varRef(NULL), m_value(NULL), m_index(NULL) {
        if (act.m_varId != NULL) {
            m_varId = new Parse_StringNode(*act.m_varId);
        }
        if (act.m_varRef != NULL) {
            m_varRef = new Parse_VarRef(*act.m_varRef);
        }
        if (act.m_value != NULL) {
            m_value = new Parse_Expression(*act.m_value);
        }
        if (act.m_index != NULL) {
            m_index = new Parse_Expression(*act.m_index);
        }
    }

	// print the action
	void Print(std::ostream& output) const;

	// append the action information as a xml node
	//void Print_XML(TiXmlElement *parent);

    std::string to_string() const noexcept {
        std::ostringstream oss;
        Print(oss);
        return oss.str();
    };
};

//===============================================================================
// Define a lable
//===============================================================================
class Parse_Label {
public:
	Parse_StringNode* m_labelName; 				// label name

public:
	Parse_Label() :
		m_labelName(NULL) {
	}

    Parse_Label(const Parse_Label& label) : m_labelName(NULL) {
        if (label.m_labelName != NULL) {
            m_labelName = new Parse_StringNode(*label.m_labelName);
        }
    }

	// print the label
	void Print(std::ostream& output, int level);
};

//===============================================================================
// Define a transition
//===============================================================================
class Parse_Transition {
public:
	std::vector<Parse_StringNode*> m_labelIds; 		// label names
	Parse_StringNode* m_location1Id; 			// current location name
	Parse_StringNode* m_location2Id; 			// next location name
	Parse_Expression* m_guard; 					// guard of the transition
	std::vector<Parse_Action*> m_actionBlock; 		// actions of the transition
public:
	Parse_Transition() :
			m_location1Id(NULL), m_location2Id(NULL), m_guard(NULL) {
	}

    Parse_Transition(const Parse_Transition& tran) : m_location1Id(NULL), m_location2Id(NULL), m_guard(NULL) {
        for (unsigned int i = 0; i < tran.m_labelIds.size(); i++) {
            if (tran.m_labelIds[i] != NULL) {
                m_labelIds.push_back(new Parse_StringNode(*tran.m_labelIds[i]));
            } else {
                m_labelIds.push_back(NULL);
            }
        }
        if (tran.m_location1Id != NULL) {
            m_location1Id = new Parse_StringNode(*tran.m_location1Id);
        }
        if (tran.m_location2Id != NULL) {
            m_location2Id = new Parse_StringNode(*tran.m_location2Id);
        }
        if (tran.m_guard != NULL) {
            m_guard = new Parse_Expression(*tran.m_guard);
        }
        for (unsigned int i = 0; i < tran.m_actionBlock.size(); i++) {
            if (tran.m_actionBlock[i] != NULL) {
                m_actionBlock.push_back(new Parse_Action(*tran.m_actionBlock[i]));
            } else {
                m_actionBlock.push_back(NULL);
            }
        }

    }

	// print the transition
	void Print(std::ostream& output, int level);

	// append the transition information as a xml node
	//void Print_XML(TiXmlElement *parent);
};

//===============================================================================
// Define a variable
//===============================================================================
class Parse_Var {
public:
    // DW: 20151015, these two are old, wanna delete them later
    // DW: 20151219, seems we should not delete them
    // DW: 20160221, m_varType plays an important now, do NOT delete
	Parse_VarType m_varType;
	Parse_StringNode *m_varLength;

    // DW: 20151219, add length for arrays
    // DW: please note that in the case of array, m_varLength is the length of array element, m_arrayLength is the length of the whole array
    Parse_StringNode *m_arrayLength;

	Parse_StringNode* m_varName; // variable name
	Parse_Expression* m_value; // initial value of the value, by default it is "false"

    // DW: 20151015, support for values in an array
    std::vector<Parse_Expression *> m_values;

    std::string module; //FIXME used by beagle-abs

public:
	Parse_Var() : m_varName(NULL), m_value(NULL), m_varLength(NULL), m_arrayLength(NULL) {
	}

    // used by absref
    Parse_Var(const Parse_Var& var) : m_varType(var.m_varType), m_varLength(NULL), m_arrayLength(NULL),  m_varName(NULL), m_value(NULL), m_values(), module(var.module) {
        if (var.m_varLength != NULL) {
            m_varLength = new Parse_StringNode(*var.m_varLength);
        }
        if (var.m_arrayLength!= NULL) {
            m_arrayLength = new Parse_StringNode(*var.m_arrayLength);
        }
        if (var.m_varName != NULL) {
            m_varName = new Parse_StringNode(*var.m_varName);
        }
        if (var.m_value != NULL) {
            m_value = new Parse_Expression(*var.m_value);
        }
        for (auto exp: var.m_values) {
            m_values.push_back(new Parse_Expression(*exp));
        }
    }

	// print the variable
	void Print(std::ostream& output, int level);

	// append the module information as a xml node
	//void Print_XML(TiXmlElement* parentNode);
};

//===============================================================================
// Define a module
//===============================================================================
class Parse_Module {
public:
	Parse_StringNode* m_moduleName; 				// module name
	std::vector<Parse_Var*> m_varNames; 					// local variable names
	std::vector<Parse_Label*> m_labelNames;				// label names
	std::vector<Parse_StringNode*> m_locationNames; 		// place names
	Parse_StringNode* m_initLocationId; 			// initial place name
	std::vector<Parse_Action*> m_initialActionBlock; 	// initial action block
	std::vector<Parse_Transition*> m_transitions; 		// transitions

public:
	Parse_Module() : m_moduleName(NULL), m_initLocationId(NULL) {
	}

    Parse_Module(const Parse_Module& module) : m_moduleName(NULL), m_initLocationId(NULL) {
        if (module.m_moduleName != NULL) {
            this->m_moduleName = new Parse_StringNode(*module.m_moduleName);
        }
        if (module.m_initLocationId != NULL) {
            this->m_initLocationId = new Parse_StringNode(*module.m_initLocationId);
        }
        for (unsigned int i = 0; i < module.m_varNames.size(); i++) {
            Parse_Var* var = NULL;
            if (module.m_varNames[i] != NULL) {
                var = new Parse_Var(*module.m_varNames[i]);
            }
            m_varNames.push_back(var);
        }
        for (unsigned int i = 0; i < module.m_labelNames.size(); i++) {
            Parse_Label* label = NULL;
            if (module.m_labelNames[i] != NULL) {
                label = new Parse_Label(*module.m_labelNames[i]);
            }
            m_labelNames.push_back(label);
        }
        for (unsigned int i = 0; i < module.m_locationNames.size(); i++) {
            Parse_StringNode* location = NULL;
            if (module.m_locationNames[i] != NULL) {
                location = new Parse_StringNode(*module.m_locationNames[i]);
            }
            m_locationNames.push_back(location);
        }
        for (unsigned int i = 0; i < module.m_initialActionBlock.size(); i++) {
            Parse_Action* initAction = NULL;
            if (module.m_initialActionBlock[i] != NULL) {
                initAction = new Parse_Action(*module.m_initialActionBlock[i]);
            }
            m_initialActionBlock.push_back(initAction);
        }
        for (unsigned int i = 0; i < module.m_transitions.size(); i++) {
            Parse_Transition* transition = NULL;
            if (module.m_transitions[i] != NULL) {
                transition = new Parse_Transition(*module.m_transitions[i]);
            }
            m_transitions.push_back(transition);
        }
    }

	void PrintSN(Parse_StringNode* sn);

	// print the module
	void Print(std::ostream& output, int level);

	// append the module information as a xml node
	//void Print_XML(TiXmlElement* parentNode);
};

//===============================================================================
// Define the global reference of a	variable, used in property representation
//===============================================================================
class Parse_VarGRef {
public:
	std::vector<Parse_StringNode*> m_parentIds;
	Parse_StringNode* m_variableId;

public:
	Parse_VarGRef() :
			m_variableId(NULL) {
	}

    Parse_VarGRef(const Parse_VarGRef& gref) : m_variableId(NULL) {
        for (unsigned int i = 0; i < gref.m_parentIds.size(); i++) {
            Parse_StringNode* parentId = NULL;
            if (gref.m_parentIds[i] != NULL) {
                parentId = new Parse_StringNode(*gref.m_parentIds[i]);
            }
            m_parentIds.push_back(parentId);
        }
        if (gref.m_variableId != NULL) {
            m_variableId = new Parse_StringNode(*gref.m_variableId);
        }
    }

	~Parse_VarGRef();
	void Print(std::ostream& output);
};

//===============================================================================
// Define the global reference of a	location variable, used in property representation
//===============================================================================
class Parse_LocationVarGRef {
public:
	std::vector<Parse_StringNode*> m_parentIds;

    Parse_LocationVarGRef() {}

    Parse_LocationVarGRef(const Parse_LocationVarGRef& gref) {
        for (unsigned int i = 0; i < gref.m_parentIds.size(); i++) {
            Parse_StringNode* parentId = NULL;
            if (gref.m_parentIds[i] != NULL) {
                parentId = new Parse_StringNode(*gref.m_parentIds[i]);
            }
            m_parentIds.push_back(parentId);
        }
    }

	~Parse_LocationVarGRef();
public:
	void Print(std::ostream& output);
};

//===============================================================================
// Define a location expression, it's always "x.y.location == IDLE", used in property
// representation
//===============================================================================
class Parse_LocationExpression {
public:
	Parse_LocationVarGRef* m_locationVar;
	Parse_StringNode* m_value;

public:
	Parse_LocationExpression() :
			m_locationVar(NULL), m_value(NULL) {
	}

    Parse_LocationExpression(const Parse_LocationExpression& locationExp) : m_locationVar(NULL), m_value(NULL) {
        if (locationExp.m_locationVar != NULL) {
            m_locationVar = new Parse_LocationVarGRef(*locationExp.m_locationVar);
        }
        if (locationExp.m_value != NULL) {
            m_value = new Parse_StringNode(*locationExp.m_value);
        }
    }

	~Parse_LocationExpression();
	void Print(std::ostream& output);
};

//===============================================================================
// Define the whole model
//===============================================================================
class Grammar_Model {
public:
	std::vector<Parse_Module*> m_modules; 			// modules
	std::vector<Parse_Property*> m_properties;		// properties to be check

public:
	Grammar_Model() {
	}

    Grammar_Model(const Grammar_Model& model) {
        for (unsigned int i = 0; i < model.m_modules.size(); i++) {
            Parse_Module* module = NULL;
            if (model.m_modules[i] != NULL) {
                module = new Parse_Module(*model.m_modules[i]);
            }
            m_modules.push_back(module);
        }
        for (unsigned int i = 0; i < model.m_properties.size(); i++) {
            Parse_Property* property = NULL;
            if (model.m_properties[i] != NULL) {
                property = new Parse_Property(*(model.m_properties[i]));
            }
            m_properties.push_back(property);
        }
    }

    // DW: get string of Grammar_Model, I think it's more useful than Print
    std::string getString();
    void printToFile(std::string fn);

	// print the Grammar_Model
	void Print(std::ostream& output);

	// print the Grammar_Model in xml format
	void Print_XML(FILE* file);
};


// DW: evaluates type of an expression
Parse_ExpressionType *evaluateType(Parse_Expression *o1, Parse_Operator o, Parse_Expression *o2 = NULL);

// DW: check for nonlinear operations
bool checkNonlinear(Parse_Expression *exp);

// DW: detection for special solver
void detectSolver(std::string key, std::string info = "");

//===========================================================================================
#endif /* GRAMMARMODEL_H_ */
