// ----------------------------------------------------------------------------
// Project: additup
/// @file   script.h
/// @author Andy Parkins
//
// Version Control
//    $Author$
//      $Date$
//        $Id$
//
// Legal
//    Copyright 2011  Andy Parkins
//
// ----------------------------------------------------------------------------

// Catch multiple includes
#ifndef SCRIPT_H
#define SCRIPT_H

// -------------- Includes
// --- C
#include <stdint.h>
// --- C++
#include <list>
#include <memory>
#include <iostream>
#include <stdexcept>
// --- Qt
// --- OS
// --- Project lib
// --- Project
#include "extraint.h"


// -------------- Namespace
	// --- Imported namespaces
	using namespace std;


// -------------- Defines
// General
// Project


// -------------- Constants


// -------------- Typedefs (pre-structure)


// -------------- Enumerations

enum eScriptOp {
	// Constants
	OP_FALSE = 0,
	PUSH_1 = 1,
	PUSH_75 = 75,
	OP_PUSHDATA1 = 76,
	OP_PUSHDATA2,
	OP_PUSHDATA4,
	OP_1NEGATE = 79,
	OP_TRUE = 81,
	OP_2 = 82, OP_3, OP_4, OP_5, OP_6, OP_7, OP_8, OP_9, OP_10, OP_11,
	OP_12, OP_13, OP_14, OP_15, OP_16,
	// Flow control
	OP_NOP = 97,
	OP_IF = 99,
	OP_NOTIF,
	OP_ELSE = 103,
	OP_ENDIF,
	OP_VERIFY,
	OP_RETURN,
	// Stack
	OP_TOALTSTACK = 107,
	OP_FROMALTSTACK,
	OP_IFDUP = 115,
	OP_DEPTH,
	OP_DROP,
	OP_DUP,
	OP_NIP,
	OP_OVER,
	OP_PICK,
	OP_ROLL,
	OP_ROT,
	OP_SWAP,
	OP_TUCK,

	OP_2DROP = 109,
	OP_2DUP,
	OP_3DUP,
	OP_2OVER,
	OP_2ROT,
	OP_2SWAP,
	// Splice
	OP_CAT = 126,
	OP_SUBSTR,
	OP_LEFT,
	OP_RIGHT,
	OP_SIZE,
	// Bitwise logic
	OP_INVERT = 131,
	OP_AND,
	OP_OR,
	OP_XOR,
	OP_EQUAL,
	OP_EQUALVERIFY,
	// Arithmetic
	OP_1ADD = 139,
	OP_1SUB,
	OP_2MUL,
	OP_2DIV,
	OP_NEGATE,
	OP_ABS,
	OP_NOT,
	OP_0NOTEQUAL,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_LSHIFT,
	OP_RSHIFT,
	OP_BOOLAND,
	OP_BOOLOR,
	OP_NUMEQUAL,
	OP_NUMEQUALVERIFY,
	OP_NUMNOTEQUAL,
	OP_LESSTHAN,
	OP_GREATERTHAN,
	OP_LESSTHANOREQUAL,
	OP_GREATERTHANOREQUAL,
	OP_MIN,
	OP_MAX,
	OP_WITHIN,
	// Crypto
	OP_RIPEMD160 = 166,
	OP_SHA1,
	OP_SHA256,
	OP_HASH160,
	OP_HASH256,
	OP_CODESEPARATOR,
	OP_CHECKSIG,
	OP_CHECKSIGVERIFY,
	OP_CHECKMULTISIG,
	OP_CHECKMULTISIGVERIFY,
	// Pseudo-words
	OP_PUBKEYHASH = 253,
	OP_PUBKEY,
	OP_INVALIDOPCODE,
	// Reserved
	OP_RESERVED = 80,
	OP_VER = 98,
	OP_VERIF = 101,
	OP_VERNOTIF = 102,
	OP_RESERVED1 = 137,
	OP_RESERVED2 = 138,
	OP_NOP1 = 176,
	OP_NOP2 = 177,
	OP_NOP3 = 178,
	OP_NOP4 = 179,
	OP_NOP5 = 180,
	OP_NOP6 = 181,
	OP_NOP7 = 182,
	OP_NOP8 = 183,
	OP_NOP9 = 184,
	OP_NOP10 = 185,
	// Done
	WORD_COUNT
};


// -------------- Structures/Unions


// -------------- Typedefs (post-structure)


// -------------- Class pre-declarations
class TStackElement;
class TStackOperator;
class TStackOperatorFromStream;
class TMessageDigest;


// -------------- Function pre-class prototypes


// -------------- Class declarations

//
// Class: script_parse_error
// Description:
//
class script_parse_error : public runtime_error
{
  public:
	explicit script_parse_error( const string &s ) :
		runtime_error(s) {}
};

class script_parse_error_not_found : public script_parse_error
{
  public:
	script_parse_error_not_found() :
		script_parse_error("script opcode not found") {}
};

class script_parse_error_type : public script_parse_error
{
  public:
	script_parse_error_type() :
		script_parse_error("script opcode type wrong") {}
};

class script_parse_error_checksum : public script_parse_error
{
  public:
	script_parse_error_checksum() :
		script_parse_error("checksum") {}
};

class script_parse_error_underflow : public script_parse_error
{
  public:
	script_parse_error_underflow() :
		script_parse_error("data too short for script opcode") {}
};

class script_parse_error_version : public script_parse_error
{
  public:
	script_parse_error_version() :
		script_parse_error("script version too old") {}
};

//
// Class: script_run_error
// Description:
//
class script_run_error : public runtime_error
{
  public:
	explicit script_run_error( const string &s ) :
		runtime_error(s) {}
};

class script_run_parameter_type_error : public script_run_error
{
  public:
	script_run_parameter_type_error() :
		script_run_error("an operator expected a parameter of a type that was not supplied") {}
};

class script_run_parameter_invalid : public script_run_error
{
  public:
	script_run_parameter_invalid() :
		script_run_error("an operator was given a parameter that would give invalid results") {}
};

class script_run_verify_error : public script_run_error
{
  public:
	script_run_verify_error() :
		script_run_error("an OP_VERIFY based command failed") {}
};


// -------------

//
// Class: TStackElement
// Description:
//
class TStackElement
{
  public:
	virtual TStackElement *clone() const = 0;

	virtual ostream &printOn( ostream & ) const = 0;
};

//
// Class: TStackElement_t
// Description:
//
template <typename t>
class TStackElement_t : public TStackElement
{
  public:
	TStackElement_t( const t &d ) : Data(d) {}
	TStackElement *clone() const { return new TStackElement_t<t>(*this); }

	ostream &printOn( ostream &s ) const;

  public:
	t Data;
};
typedef TStackElement_t<bool> TStackElementBoolean;
typedef TStackElement_t<int> TStackElementInteger;
typedef TStackElement_t<TBigInteger> TStackElementBigInteger;
typedef TStackElement_t<string> TStackElementString;


// -------------

//
// Class: TExecutionStack
// Description:
//
class TExecutionStack
{
  public:
	TExecutionStack();

	ostream &printOn( ostream &s ) const;

	list<TStackElement*> &operator()() { return Stack; }

	TStackElement *take() { TStackElement *x = Stack.back(); Stack.pop_back(); return x; }
	void give( TStackElement *s ) { Stack.push_back(s); }

  public:
	list<TStackElement*> Stack;
	list<TStackElement*> AltStack;
	typedef list<TStackElement*>::iterator iterator;
	typedef list<TStackElement*>::const_iterator const_iterator;

	bool Invalid;
};

//
// Class: TBitcoinScript
// Description:
//
class TBitcoinScript
{
  public:
	TBitcoinScript();
	virtual ~TBitcoinScript();

	istream &read( istream & );

	void execute( TExecutionStack & ) const;

	virtual uint32_t getMinimumAcceptedVersion() const = 0;

	void append( TStackOperator *op ) { Program.push_back(op); }

	typedef list<TStackOperator*>::const_iterator tInstructionPointer;

  protected:
	virtual void init();

  protected:
	bool Initialised;
	list<const TStackOperatorFromStream *> Templates;
	list<TStackOperator *> Program;
};

//
// Class: TBitcoinScript_0
// Description:
//
class TBitcoinScript_0 : public TBitcoinScript
{
  public:
	TBitcoinScript_0();

	uint32_t getMinimumAcceptedVersion() const;

  protected:
	void init();
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
