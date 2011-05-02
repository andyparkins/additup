// ----------------------------------------------------------------------------
// Project: bitcoin
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

// -------------

//
// Class: TExecutionStack
// Description:
//
class TExecutionStack
{
  public:
	TExecutionStack();

	void push( int );
	void push( const string & );

	list<TStackElement *> Stack;
	list<TStackElement *> AltStack;

	bool Invalid;
};

//
// Class: TBitcoinProgram
// Description:
//
class TBitcoinProgram
{
  public:
	TBitcoinProgram();
	virtual ~TBitcoinProgram();

	istream &read( istream & );

	void execute( TExecutionStack & ) const;

	virtual uint32_t getMinimumAcceptedVersion() const = 0;

  protected:
	virtual void init();

  protected:
	bool Initialised;
	list<const TStackOperatorFromStream *> Templates;
	list<TStackOperator *> Program;
};

//
// Class: TBitcoinProgram_0
// Description:
//
class TBitcoinProgram_0 : public TBitcoinProgram
{
  public:
	TBitcoinProgram_0();

	uint32_t getMinimumAcceptedVersion() const;

  protected:
	void init();
};

// -------------

//
// Class: TStackOperator
// Description:
// Manipulates a stack in some way.
//
class TStackOperator
{
  public:
	TStackOperator() {}
	virtual const char *className() const { return "TStackOperator"; }

	virtual void execute( TExecutionStack & ) const = 0;
};

//
// Class: TStackOperatorFromStream
// Description:
// A TStackOperator which is configured from a script.
//
// An operation is not the same as the code that represents that
// operation in a script.  This class represents an operation that can
// be read from a script.
//
class TStackOperatorFromStream : public TStackOperator
{
  public:
	const char *className() const { return "TStackOperatorFromStream"; }
	virtual TStackOperatorFromStream *clone() const = 0;
	virtual istream &read( istream & ) = 0;

	virtual bool acceptOpcode( eScriptOp ) const = 0;
};

//
// Class: TStackOperatorFromOpcode
// Description:
// T
//
class TStackOperatorFromOpcode : public TStackOperatorFromStream
{
  public:
	const char *className() const { return "TStackOperatorFromOpcode"; }
	bool acceptOpcode( eScriptOp op ) const { return op == getOpcode(); }
	virtual eScriptOp getOpcode() const = 0;

	istream &read( istream &is ) {
		// Discard the Opcode, we already know it's getOpcode()
		is.get();
		return is;
	}

};

class TStackOperator_OP_FALSE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_FALSE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_FALSE(*this); }
	eScriptOp getOpcode() const { return OP_FALSE; }

	void execute( TExecutionStack &Stack ) const { Stack.push(0); }
};

class TStackOperator_OP_PUSHDATAN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_PUSHDATAN"; }

	istream &read( istream &is ) {
		// Get the opcode byte
		TStackOperatorFromOpcode::read(is);
		// The next bytes tells us how many bytes to read
		streamsize n = getRawReadCount(is);
		char buffer[n];
		is.read(buffer, n);
		Raw.assign(buffer, n);
		return is;
	}

	void execute( TExecutionStack &Stack ) const { Stack.push( Raw ); }

  protected:
	virtual streamsize getRawReadCount( istream & ) const = 0;

  protected:
	string Raw;
};

class TStackOperator_OP_PUSHDATA1 : public TStackOperator_OP_PUSHDATAN
{
  public:
	const char *className() const { return "TStackOperator_OP_PUSHDATA1"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_PUSHDATA1(*this); }
	eScriptOp getOpcode() const { return OP_PUSHDATA1; }

  protected:
	streamsize getRawReadCount( istream &is ) const {
		return is.get();
	}
};

class TStackOperator_OP_PUSHDATA2 : public TStackOperator_OP_PUSHDATAN
{
  public:
	const char *className() const { return "TStackOperator_OP_PUSHDATA2"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_PUSHDATA2(*this); }
	eScriptOp getOpcode() const { return OP_PUSHDATA2; }

  protected:
	streamsize getRawReadCount( istream &is ) const {
		// Assume big endian, although it's not made clear in the docs
		streamsize N;
		N = is.get() << 8;
		N |= is.get() << 0;
		return N;
	}
};

class TStackOperator_OP_PUSHDATA4 : public TStackOperator_OP_PUSHDATAN
{
  public:
	const char *className() const { return "TStackOperator_OP_PUSHDATA4"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_PUSHDATA4(*this); }
	eScriptOp getOpcode() const { return OP_PUSHDATA4; }

  protected:
	streamsize getRawReadCount( istream &is ) const {
		// Assume big endian, although it's not made clear in the docs
		streamsize N;
		N = is.get() << 24;
		N |= is.get() << 16;
		N |= is.get() << 8;
		N |= is.get() << 0;
		return N;
	}
};

class TStackOperator_OP_1NEGATE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_1NEGATE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_1NEGATE(*this); }
	eScriptOp getOpcode() const { return OP_1NEGATE; }

	void execute( TExecutionStack &Stack ) const { Stack.push(-1); }
};

class TStackOperator_OP_TRUE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_TRUE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_TRUE(*this); }
	eScriptOp getOpcode() const { return OP_TRUE; }

	void execute( TExecutionStack &Stack ) const { Stack.push(1); }
};

class TStackOperator_OP_NOP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NOP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NOP(*this); }
	eScriptOp getOpcode() const { return OP_NOP; }

	void execute( TExecutionStack &Stack ) const {}
};

class TStackOperator_OP_IF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_IF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_IF(*this); }
	eScriptOp getOpcode() const { return OP_IF; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_NOTIF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NOTIF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NOTIF(*this); }
	eScriptOp getOpcode() const { return OP_NOTIF; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_ELSE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ELSE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ELSE(*this); }
	eScriptOp getOpcode() const { return OP_ELSE; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_ENDIF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ENDIF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ENDIF(*this); }
	eScriptOp getOpcode() const { return OP_ENDIF; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_VERIFY : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_VERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_VERIFY(*this); }
	eScriptOp getOpcode() const { return OP_VERIFY; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_RETURN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RETURN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RETURN(*this); }
	eScriptOp getOpcode() const { return OP_RETURN; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_TOALTSTACK : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_TOALTSTACK"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_TOALTSTACK(*this); }
	eScriptOp getOpcode() const { return OP_TOALTSTACK; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_FROMALTSTACK : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_FROMALTSTACK"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_FROMALTSTACK(*this); }
	eScriptOp getOpcode() const { return OP_FROMALTSTACK; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_IFDUP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_IFDUP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_IFDUP(*this); }
	eScriptOp getOpcode() const { return OP_IFDUP; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_DEPTH : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_DEPTH"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_DEPTH(*this); }
	eScriptOp getOpcode() const { return OP_DEPTH; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_DROP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_DROP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_DROP(*this); }
	eScriptOp getOpcode() const { return OP_DROP; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_DUP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_DUP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_DUP(*this); }
	eScriptOp getOpcode() const { return OP_DUP; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_NIP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NIP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NIP(*this); }
	eScriptOp getOpcode() const { return OP_NIP; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_OVER : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_OVER"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_OVER(*this); }
	eScriptOp getOpcode() const { return OP_OVER; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_PICK : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_PICK"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_PICK(*this); }
	eScriptOp getOpcode() const { return OP_PICK; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_ROLL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ROLL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ROLL(*this); }
	eScriptOp getOpcode() const { return OP_ROLL; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_ROT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ROT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ROT(*this); }
	eScriptOp getOpcode() const { return OP_ROT; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_SWAP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SWAP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SWAP(*this); }
	eScriptOp getOpcode() const { return OP_SWAP; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_TUCK : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_TUCK"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_TUCK(*this); }
	eScriptOp getOpcode() const { return OP_TUCK; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_2DROP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2DROP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2DROP(*this); }
	eScriptOp getOpcode() const { return OP_2DROP; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_2DUP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2DUP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2DUP(*this); }
	eScriptOp getOpcode() const { return OP_2DUP; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_3DUP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_3DUP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_3DUP(*this); }
	eScriptOp getOpcode() const { return OP_3DUP; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_2OVER : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2OVER"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2OVER(*this); }
	eScriptOp getOpcode() const { return OP_2OVER; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_2ROT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2ROT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2ROT(*this); }
	eScriptOp getOpcode() const { return OP_2ROT; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_2SWAP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2SWAP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2SWAP(*this); }
	eScriptOp getOpcode() const { return OP_2SWAP; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_CAT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CAT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CAT(*this); }
	eScriptOp getOpcode() const { return OP_CAT; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_SUBSTR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SUBSTR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SUBSTR(*this); }
	eScriptOp getOpcode() const { return OP_SUBSTR; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_LEFT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_LEFT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_LEFT(*this); }
	eScriptOp getOpcode() const { return OP_LEFT; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_RIGHT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RIGHT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RIGHT(*this); }
	eScriptOp getOpcode() const { return OP_RIGHT; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_SIZE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SIZE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SIZE(*this); }
	eScriptOp getOpcode() const { return OP_SIZE; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_INVERT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_INVERT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_INVERT(*this); }
	eScriptOp getOpcode() const { return OP_INVERT; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_AND : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_AND"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_AND(*this); }
	eScriptOp getOpcode() const { return OP_AND; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_OR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_OR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_OR(*this); }
	eScriptOp getOpcode() const { return OP_OR; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_XOR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_XOR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_XOR(*this); }
	eScriptOp getOpcode() const { return OP_XOR; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_EQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_EQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_EQUAL(*this); }
	eScriptOp getOpcode() const { return OP_EQUAL; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_EQUALVERIFY : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_EQUALVERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_EQUALVERIFY(*this); }
	eScriptOp getOpcode() const { return OP_EQUALVERIFY; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_1ADD : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_1ADD"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_1ADD(*this); }
	eScriptOp getOpcode() const { return OP_1ADD; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_1SUB : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_1SUB"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_1SUB(*this); }
	eScriptOp getOpcode() const { return OP_1SUB; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_2MUL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2MUL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2MUL(*this); }
	eScriptOp getOpcode() const { return OP_2MUL; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_2DIV : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2DIV"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2DIV(*this); }
	eScriptOp getOpcode() const { return OP_2DIV; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_NEGATE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NEGATE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NEGATE(*this); }
	eScriptOp getOpcode() const { return OP_NEGATE; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_ABS : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ABS"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ABS(*this); }
	eScriptOp getOpcode() const { return OP_ABS; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_NOT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NOT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NOT(*this); }
	eScriptOp getOpcode() const { return OP_NOT; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_0NOTEQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_0NOTEQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_0NOTEQUAL(*this); }
	eScriptOp getOpcode() const { return OP_0NOTEQUAL; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_ADD : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ADD"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ADD(*this); }
	eScriptOp getOpcode() const { return OP_ADD; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_SUB : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SUB"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SUB(*this); }
	eScriptOp getOpcode() const { return OP_SUB; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_MUL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_MUL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_MUL(*this); }
	eScriptOp getOpcode() const { return OP_MUL; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_DIV : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_DIV"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_DIV(*this); }
	eScriptOp getOpcode() const { return OP_DIV; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_MOD : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_MOD"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_MOD(*this); }
	eScriptOp getOpcode() const { return OP_MOD; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_LSHIFT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_LSHIFT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_LSHIFT(*this); }
	eScriptOp getOpcode() const { return OP_LSHIFT; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_RSHIFT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RSHIFT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RSHIFT(*this); }
	eScriptOp getOpcode() const { return OP_RSHIFT; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_BOOLAND : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_BOOLAND"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_BOOLAND(*this); }
	eScriptOp getOpcode() const { return OP_BOOLAND; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_BOOLOR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_BOOLOR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_BOOLOR(*this); }
	eScriptOp getOpcode() const { return OP_BOOLOR; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_NUMEQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NUMEQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NUMEQUAL(*this); }
	eScriptOp getOpcode() const { return OP_NUMEQUAL; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_NUMEQUALVERIFY : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NUMEQUALVERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NUMEQUALVERIFY(*this); }
	eScriptOp getOpcode() const { return OP_NUMEQUALVERIFY; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_NUMNOTEQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NUMNOTEQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NUMNOTEQUAL(*this); }
	eScriptOp getOpcode() const { return OP_NUMNOTEQUAL; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_LESSTHAN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_LESSTHAN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_LESSTHAN(*this); }
	eScriptOp getOpcode() const { return OP_LESSTHAN; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_GREATERTHAN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_GREATERTHAN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_GREATERTHAN(*this); }
	eScriptOp getOpcode() const { return OP_GREATERTHAN; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_LESSTHANOREQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_LESSTHANOREQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_LESSTHANOREQUAL(*this); }
	eScriptOp getOpcode() const { return OP_LESSTHANOREQUAL; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_GREATERTHANOREQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_GREATERTHANOREQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_GREATERTHANOREQUAL(*this); }
	eScriptOp getOpcode() const { return OP_GREATERTHANOREQUAL; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_MIN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_MIN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_MIN(*this); }
	eScriptOp getOpcode() const { return OP_MIN; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_MAX : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_MAX"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_MAX(*this); }
	eScriptOp getOpcode() const { return OP_MAX; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_WITHIN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_WITHIN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_WITHIN(*this); }
	eScriptOp getOpcode() const { return OP_WITHIN; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_RIPEMD160 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RIPEMD160"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RIPEMD160(*this); }
	eScriptOp getOpcode() const { return OP_RIPEMD160; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_SHA1 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SHA1"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SHA1(*this); }
	eScriptOp getOpcode() const { return OP_SHA1; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_SHA256 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SHA256"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SHA256(*this); }
	eScriptOp getOpcode() const { return OP_SHA256; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_HASH160 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_HASH160"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_HASH160(*this); }
	eScriptOp getOpcode() const { return OP_HASH160; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_HASH256 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_HASH256"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_HASH256(*this); }
	eScriptOp getOpcode() const { return OP_HASH256; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_CODESEPARATOR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CODESEPARATOR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CODESEPARATOR(*this); }
	eScriptOp getOpcode() const { return OP_CODESEPARATOR; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_CHECKSIG : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CHECKSIG"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CHECKSIG(*this); }
	eScriptOp getOpcode() const { return OP_CHECKSIG; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_CHECKSIGVERIFY : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CHECKSIGVERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CHECKSIGVERIFY(*this); }
	eScriptOp getOpcode() const { return OP_CHECKSIGVERIFY; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_CHECKMULTISIG : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CHECKMULTISIG"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CHECKMULTISIG(*this); }
	eScriptOp getOpcode() const { return OP_CHECKMULTISIG; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_CHECKMULTISIGVERIFY : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CHECKMULTISIGVERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CHECKMULTISIGVERIFY(*this); }
	eScriptOp getOpcode() const { return OP_CHECKMULTISIGVERIFY; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_PUBKEYHASH : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_PUBKEYHASH"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_PUBKEYHASH(*this); }
	eScriptOp getOpcode() const { return OP_PUBKEYHASH; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_PUBKEY : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_PUBKEY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_PUBKEY(*this); }
	eScriptOp getOpcode() const { return OP_PUBKEY; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_RESERVED : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RESERVED"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RESERVED(*this); }
	eScriptOp getOpcode() const { return OP_RESERVED; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_VER : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_VER"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_VER(*this); }
	eScriptOp getOpcode() const { return OP_VER; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_VERIF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_VERIF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_VERIF(*this); }
	eScriptOp getOpcode() const { return OP_VERIF; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_VERNOTIF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_VERNOTIF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_VERNOTIF(*this); }
	eScriptOp getOpcode() const { return OP_VERNOTIF; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_RESERVED1 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RESERVED1"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RESERVED1(*this); }
	eScriptOp getOpcode() const { return OP_RESERVED1; }

	void execute( TExecutionStack &Stack ) const;
};

class TStackOperator_OP_RESERVED2 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RESERVED2"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RESERVED2(*this); }
	eScriptOp getOpcode() const { return OP_RESERVED2; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperatorFromOpcodes
// Description:
//
class TStackOperatorFromOpcodes : public TStackOperatorFromStream
{
  public:
	const char *className() const { return "TStackOperatorFromOpcodes"; }

	istream &read( istream &is ) {
		OP = static_cast<eScriptOp>( is.get() );
		return is;
	}

  protected:
	eScriptOp OP;
};

//
// Class:	TStackOperator_OP_INVALIDOPCODE
// Description:
// OP_INVALIDOPCODE is special, the spec says "matches any opcode
// not yet assigned".  It does still have an opcode as it happens
// (255), so it is still a TStackOperatorFromOpcode.  However, it's
// expected that it will always be tried last and has an acceptOpcode()
// that always returns true -- it will take any opcode.
//
class TStackOperator_OP_INVALIDOPCODE : public TStackOperatorFromOpcodes
{
  public:
	const char *className() const { return "TStackOperator_OP_INVALIDOPCODE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_INVALIDOPCODE(*this); }
	eScriptOp getOpcode() const { return OP_INVALIDOPCODE; }
	bool acceptOpcode( eScriptOp op ) const { return true; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_N
// Description:
// OP_N pushes N onto the stack.
//
class TStackOperator_OP_N : public TStackOperatorFromOpcodes
{
  public:
	const char *className() const { return "TStackOperator_OP_N"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_N(*this); }
	bool acceptOpcode( eScriptOp op ) const { return op >= OP_2 && op <= OP_16; }

	void execute( TExecutionStack &Stack ) const {
		Stack.push( static_cast<uint8_t>( OP - OP_2 ) );
	}
};

//
// Class: TStackOperator_PUSH_N
// Description:
// Handle PUSH_N opcodes.
//
// The PUSH_N codes are commands to push the next N bytes of raw script
// data onto the stack.  They are unusual because N is the number of the
// opcode itself.  These are fast ways of pushing small numbers of
// literal bytes in a script.
//
class TStackOperator_PUSH_N : public TStackOperatorFromOpcodes
{
  public:
	const char *className() const { return "TStackOperator_PUSH_N"; }
	bool acceptOpcode( eScriptOp op ) const { return op >= PUSH_1 && op <= PUSH_75; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_PUSH_N(*this); }

	istream &read( istream &is ) {
		TStackOperatorFromOpcodes::read(is);
		// Now we have OP we can read N bytes
		char buffer[OP];
		is.read( buffer, OP );
		// Assign the bytes to a string
		Raw.assign( buffer, OP );
		return is;
	}

	void execute( TExecutionStack &Stack ) const { Stack.push( Raw ); }

  protected:
	string Raw;
};

//
// Class: TStackOperator_OP_NOP_N
// Description:
// OP_NOP_N does nothing.
//
class TStackOperator_OP_NOP_N : public TStackOperatorFromOpcodes
{
  public:
	const char *className() const { return "TStackOperator_OP_NOP_N"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NOP_N(*this); }
	bool acceptOpcode( eScriptOp op ) const { return op >= OP_NOP1 && op <= OP_NOP10; }

	void execute( TExecutionStack &Stack ) const {}
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
