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
#include "hashtypes.h"


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
class TTransaction;


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

// ----------

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
typedef TStackElement_t<TBitcoinHash> TStackElementHash;
typedef TStackElement_t<string> TStackElementString;


// -------------

//
// Class: TExecutionContext
// Description:
//
class TExecutionContext
{
  public:
	TExecutionContext();

	ostream &printOn( ostream &s ) const;

	list<TStackElement*> &operator()() { return Stack; }

	TStackElement *take() { TStackElement *x = Stack.back(); Stack.pop_back(); return x; }
	void give( TStackElement *s ) { Stack.push_back(s); }

	void setTransaction( TTransaction * );

  public:
	list<TStackElement*> Stack;
	list<TStackElement*> AltStack;
	typedef list<TStackElement*>::iterator iterator;
	typedef list<TStackElement*>::const_iterator const_iterator;

	TTransaction *Transaction;
	bool Invalid;
};

//
// Class: TBitcoinScriptBase
// Description:
//
class TBitcoinScriptBase
{
  public:
	TBitcoinScriptBase();
	TBitcoinScriptBase( const TStackOperator **, unsigned int );
	virtual ~TBitcoinScriptBase();

	void execute( TExecutionContext & ) const;

	void append( TStackOperator *op );

	typedef list<TStackOperator*>::const_iterator tInstructionPointer;

	virtual ostream &printOn( ostream & ) const;

  protected:
	virtual void init();

  protected:
	bool Initialised;
	list<TStackOperator *> Program;

  private:
	// Not implemented yet -- needs deep copy
	TBitcoinScriptBase( const TBitcoinScriptBase & ) {}
};

//
// Class: TBitcoinScript
// Description:
//
class TBitcoinScript : public TBitcoinScriptBase
{
  public:
	enum eReadMode {
		AuthorisationScript,
		ClaimantScript
	};

  public:
	TBitcoinScript() {};
	TBitcoinScript( const TStackOperator **, unsigned int );
	TBitcoinScript( const string &, eReadMode );
	~TBitcoinScript();
	virtual uint32_t getMinimumAcceptedVersion() const = 0;

	istream &read( istream &, eReadMode );
	ostream &write( ostream & ) const;

  protected:
	list<const TStackOperatorFromStream *> ClaimantTemplates;
	list<const TStackOperatorFromStream *> AuthorisationTemplates;
};

//
// Class: TBitcoinScript_0
// Description:
//
class TBitcoinScript_0 : public TBitcoinScript
{
  public:
	TBitcoinScript_0();
	TBitcoinScript_0( const TStackOperator **, unsigned int );
	TBitcoinScript_0( const string &, eReadMode );

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
	virtual TStackOperator *clone() const = 0;

	virtual TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer & ) const = 0;

	virtual ostream &printOn( ostream & ) const;

	static TStackOperator *createPUSH( const string & );
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
	virtual istream &readAndAppend( TBitcoinScriptBase *, istream & ) const;
	virtual istream &read( istream & ) = 0;

	virtual bool acceptOpcode( eScriptOp ) const = 0;

	virtual ostream &write( ostream & ) = 0;
};

//
// Class: TStackOperatorFromOpcode
// Description:
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

	ostream &write( ostream &os ) {
		os.put( getOpcode() );
		return os;
	}

};

//
// Class: TStackOperatorFromCompoundOpcode
// Description:
//
class TStackOperatorFromCompoundOpcode : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperatorFromCompoundOpcode"; }
	istream &readAndAppend( TBitcoinScriptBase *S, istream &is ) const {
		// Discard the Opcode, we already know it's getOpcode()
		is.get();
		// Perform the append
		explode(S);
		return is;
	}

	// Deny read() and execute()
	istream &read( istream & ) {
		throw logic_error("TStackOperatorFromCompoundOpcode should never be read()");
	}
	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer & ) const {
		throw logic_error("TStackOperatorFromCompoundOpcode should never be execute()d");
	}

	virtual void explode( TBitcoinScriptBase * ) const = 0;
};

//
// Class: TStackOperator_OP_FALSE
// Description:
//
class TStackOperator_OP_FALSE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_FALSE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_FALSE(*this); }
	eScriptOp getOpcode() const { return OP_FALSE; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_PUSHDATAN
// Description:
//
class TStackOperator_OP_PUSHDATAN : public TStackOperatorFromOpcode
{
  public:
	TStackOperator_OP_PUSHDATAN() {};
	TStackOperator_OP_PUSHDATAN( const string &r ) { Raw = r; }
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

	ostream &write( ostream &os ) {
		// Write the opcode byte
		TStackOperatorFromOpcode::write(os);
		// Write the appropriate number of size bytes
		putRawWriteCount(os);
		// Write the bytes
		os.write( Raw.data(), Raw.size() );
		return os;
	}

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;

  protected:
	virtual streamsize getRawReadCount( istream & ) const = 0;
	virtual void putRawWriteCount( ostream & ) const = 0;

  protected:
	string Raw;
};

//
// Class: TStackOperator_OP_PUSHDATA1
// Description:
//
class TStackOperator_OP_PUSHDATA1 : public TStackOperator_OP_PUSHDATAN
{
  public:
	TStackOperator_OP_PUSHDATA1() {}
	TStackOperator_OP_PUSHDATA1( const string &r ) : TStackOperator_OP_PUSHDATAN(r) {}
	const char *className() const { return "TStackOperator_OP_PUSHDATA1"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_PUSHDATA1(*this); }
	eScriptOp getOpcode() const { return OP_PUSHDATA1; }

  protected:
	streamsize getRawReadCount( istream &is ) const {
		return is.get();
	}
	void putRawWriteCount( ostream &os ) const {
		os.put( Raw.size() & 0xff );
	}
};

//
// Class: TStackOperator_OP_PUSHDATA2
// Description:
//
class TStackOperator_OP_PUSHDATA2 : public TStackOperator_OP_PUSHDATAN
{
  public:
	TStackOperator_OP_PUSHDATA2() {}
	TStackOperator_OP_PUSHDATA2( const string &r ) : TStackOperator_OP_PUSHDATAN(r) {}
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
	void putRawWriteCount( ostream &os ) const {
		os.put( Raw.size() & 0xff );
		os.put( (Raw.size() & 0xff00) >> 8 );
	}
};

//
// Class: TStackOperator_OP_PUSHDATA4
// Description:
//
class TStackOperator_OP_PUSHDATA4 : public TStackOperator_OP_PUSHDATAN
{
  public:
	TStackOperator_OP_PUSHDATA4() {}
	TStackOperator_OP_PUSHDATA4( const string &r ) : TStackOperator_OP_PUSHDATAN(r) {}
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
	void putRawWriteCount( ostream &os ) const {
		os.put( Raw.size() & 0xff );
		os.put( (Raw.size() & 0xff00) >> 8 );
		os.put( (Raw.size() & 0xff0000) >> 16 );
		os.put( (Raw.size() & 0xff000000) >> 24 );
	}
};

//
// Class: TStackOperator_OP_1NEGATE
// Description:
//
class TStackOperator_OP_1NEGATE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_1NEGATE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_1NEGATE(*this); }
	eScriptOp getOpcode() const { return OP_1NEGATE; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_TRUE
// Description:
//
class TStackOperator_OP_TRUE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_TRUE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_TRUE(*this); }
	eScriptOp getOpcode() const { return OP_TRUE; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_NOP
// Description:
//
class TStackOperator_OP_NOP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NOP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NOP(*this); }
	eScriptOp getOpcode() const { return OP_NOP; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const { return ip; }
};

//
// Class: TStackOperator_OP_IF
// Description:
//
class TStackOperator_OP_IF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_IF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_IF(*this); }
	eScriptOp getOpcode() const { return OP_IF; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_NOTIF
// Description:
//
class TStackOperator_OP_NOTIF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NOTIF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NOTIF(*this); }
	eScriptOp getOpcode() const { return OP_NOTIF; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_ELSE
// Description:
//
class TStackOperator_OP_ELSE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ELSE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ELSE(*this); }
	eScriptOp getOpcode() const { return OP_ELSE; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_ENDIF
// Description:
//
class TStackOperator_OP_ENDIF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ENDIF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ENDIF(*this); }
	eScriptOp getOpcode() const { return OP_ENDIF; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_VERIFY
// Description:
//
class TStackOperator_OP_VERIFY : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_VERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_VERIFY(*this); }
	eScriptOp getOpcode() const { return OP_VERIFY; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_RETURN
// Description:
//
class TStackOperator_OP_RETURN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RETURN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RETURN(*this); }
	eScriptOp getOpcode() const { return OP_RETURN; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_TOALTSTACK
// Description:
//
class TStackOperator_OP_TOALTSTACK : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_TOALTSTACK"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_TOALTSTACK(*this); }
	eScriptOp getOpcode() const { return OP_TOALTSTACK; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_FROMALTSTACK
// Description:
//
class TStackOperator_OP_FROMALTSTACK : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_FROMALTSTACK"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_FROMALTSTACK(*this); }
	eScriptOp getOpcode() const { return OP_FROMALTSTACK; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_IFDUP
// Description:
//
class TStackOperator_OP_IFDUP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_IFDUP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_IFDUP(*this); }
	eScriptOp getOpcode() const { return OP_IFDUP; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_DEPTH
// Description:
//
class TStackOperator_OP_DEPTH : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_DEPTH"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_DEPTH(*this); }
	eScriptOp getOpcode() const { return OP_DEPTH; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_DROP
// Description:
//
class TStackOperator_OP_DROP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_DROP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_DROP(*this); }
	eScriptOp getOpcode() const { return OP_DROP; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_DUP
// Description:
//
class TStackOperator_OP_DUP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_DUP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_DUP(*this); }
	eScriptOp getOpcode() const { return OP_DUP; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_NIP
// Description:
//
class TStackOperator_OP_NIP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NIP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NIP(*this); }
	eScriptOp getOpcode() const { return OP_NIP; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_OVER
// Description:
//
class TStackOperator_OP_OVER : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_OVER"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_OVER(*this); }
	eScriptOp getOpcode() const { return OP_OVER; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_PICK
// Description:
//
class TStackOperator_OP_PICK : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_PICK"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_PICK(*this); }
	eScriptOp getOpcode() const { return OP_PICK; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_ROLL
// Description:
//
class TStackOperator_OP_ROLL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ROLL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ROLL(*this); }
	eScriptOp getOpcode() const { return OP_ROLL; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_ROT
// Description:
//
class TStackOperator_OP_ROT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ROT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ROT(*this); }
	eScriptOp getOpcode() const { return OP_ROT; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_SWAP
// Description:
//
class TStackOperator_OP_SWAP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SWAP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SWAP(*this); }
	eScriptOp getOpcode() const { return OP_SWAP; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_TUCK
// Description:
//
class TStackOperator_OP_TUCK : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_TUCK"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_TUCK(*this); }
	eScriptOp getOpcode() const { return OP_TUCK; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_2DROP
// Description:
//
class TStackOperator_OP_2DROP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2DROP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2DROP(*this); }
	eScriptOp getOpcode() const { return OP_2DROP; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_2DUP
// Description:
//
class TStackOperator_OP_2DUP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2DUP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2DUP(*this); }
	eScriptOp getOpcode() const { return OP_2DUP; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_3DUP
// Description:
//
class TStackOperator_OP_3DUP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_3DUP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_3DUP(*this); }
	eScriptOp getOpcode() const { return OP_3DUP; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_2OVER
// Description:
//
class TStackOperator_OP_2OVER : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2OVER"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2OVER(*this); }
	eScriptOp getOpcode() const { return OP_2OVER; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_2ROT
// Description:
//
class TStackOperator_OP_2ROT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2ROT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2ROT(*this); }
	eScriptOp getOpcode() const { return OP_2ROT; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_2SWAP
// Description:
//
class TStackOperator_OP_2SWAP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2SWAP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2SWAP(*this); }
	eScriptOp getOpcode() const { return OP_2SWAP; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_CAT
// Description:
//
class TStackOperator_OP_CAT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CAT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CAT(*this); }
	eScriptOp getOpcode() const { return OP_CAT; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_SUBSTR
// Description:
//
class TStackOperator_OP_SUBSTR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SUBSTR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SUBSTR(*this); }
	eScriptOp getOpcode() const { return OP_SUBSTR; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_LEFT
// Description:
//
class TStackOperator_OP_LEFT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_LEFT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_LEFT(*this); }
	eScriptOp getOpcode() const { return OP_LEFT; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_RIGHT
// Description:
//
class TStackOperator_OP_RIGHT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RIGHT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RIGHT(*this); }
	eScriptOp getOpcode() const { return OP_RIGHT; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_SIZE
// Description:
//
class TStackOperator_OP_SIZE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SIZE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SIZE(*this); }
	eScriptOp getOpcode() const { return OP_SIZE; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_INVERT
// Description:
//
class TStackOperator_OP_INVERT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_INVERT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_INVERT(*this); }
	eScriptOp getOpcode() const { return OP_INVERT; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_AND
// Description:
//
class TStackOperator_OP_AND : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_AND"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_AND(*this); }
	eScriptOp getOpcode() const { return OP_AND; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_OR
// Description:
//
class TStackOperator_OP_OR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_OR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_OR(*this); }
	eScriptOp getOpcode() const { return OP_OR; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_XOR
// Description:
//
class TStackOperator_OP_XOR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_XOR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_XOR(*this); }
	eScriptOp getOpcode() const { return OP_XOR; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_EQUAL
// Description:
//
class TStackOperator_OP_EQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_EQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_EQUAL(*this); }
	eScriptOp getOpcode() const { return OP_EQUAL; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_EQUALVERIFY
// Description:
//
class TStackOperator_OP_EQUALVERIFY : public TStackOperatorFromCompoundOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_EQUALVERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_EQUALVERIFY(*this); }
	eScriptOp getOpcode() const { return OP_EQUALVERIFY; }

	void explode( TBitcoinScriptBase * ) const;
};

//
// Class: TStackOperator_OP_1ADD
// Description:
//
class TStackOperator_OP_1ADD : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_1ADD"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_1ADD(*this); }
	eScriptOp getOpcode() const { return OP_1ADD; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_1SUB
// Description:
//
class TStackOperator_OP_1SUB : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_1SUB"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_1SUB(*this); }
	eScriptOp getOpcode() const { return OP_1SUB; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_2MUL
// Description:
//
class TStackOperator_OP_2MUL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2MUL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2MUL(*this); }
	eScriptOp getOpcode() const { return OP_2MUL; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_2DIV
// Description:
//
class TStackOperator_OP_2DIV : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2DIV"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2DIV(*this); }
	eScriptOp getOpcode() const { return OP_2DIV; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_NEGATE
// Description:
//
class TStackOperator_OP_NEGATE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NEGATE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NEGATE(*this); }
	eScriptOp getOpcode() const { return OP_NEGATE; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_ABS
// Description:
//
class TStackOperator_OP_ABS : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ABS"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ABS(*this); }
	eScriptOp getOpcode() const { return OP_ABS; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_NOT
// Description:
//
class TStackOperator_OP_NOT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NOT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NOT(*this); }
	eScriptOp getOpcode() const { return OP_NOT; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_0NOTEQUAL
// Description:
//
class TStackOperator_OP_0NOTEQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_0NOTEQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_0NOTEQUAL(*this); }
	eScriptOp getOpcode() const { return OP_0NOTEQUAL; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_ADD
// Description:
//
class TStackOperator_OP_ADD : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ADD"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ADD(*this); }
	eScriptOp getOpcode() const { return OP_ADD; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_SUB
// Description:
//
class TStackOperator_OP_SUB : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SUB"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SUB(*this); }
	eScriptOp getOpcode() const { return OP_SUB; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_MUL
// Description:
//
class TStackOperator_OP_MUL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_MUL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_MUL(*this); }
	eScriptOp getOpcode() const { return OP_MUL; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_DIV
// Description:
//
class TStackOperator_OP_DIV : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_DIV"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_DIV(*this); }
	eScriptOp getOpcode() const { return OP_DIV; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_MOD
// Description:
//
class TStackOperator_OP_MOD : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_MOD"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_MOD(*this); }
	eScriptOp getOpcode() const { return OP_MOD; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_LSHIFT
// Description:
//
class TStackOperator_OP_LSHIFT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_LSHIFT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_LSHIFT(*this); }
	eScriptOp getOpcode() const { return OP_LSHIFT; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_RSHIFT
// Description:
//
class TStackOperator_OP_RSHIFT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RSHIFT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RSHIFT(*this); }
	eScriptOp getOpcode() const { return OP_RSHIFT; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_BOOLAND
// Description:
//
class TStackOperator_OP_BOOLAND : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_BOOLAND"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_BOOLAND(*this); }
	eScriptOp getOpcode() const { return OP_BOOLAND; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_BOOLOR
// Description:
//
class TStackOperator_OP_BOOLOR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_BOOLOR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_BOOLOR(*this); }
	eScriptOp getOpcode() const { return OP_BOOLOR; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_NUMEQUAL
// Description:
//
class TStackOperator_OP_NUMEQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NUMEQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NUMEQUAL(*this); }
	eScriptOp getOpcode() const { return OP_NUMEQUAL; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_NUMEQUALVERIFY
// Description:
//
class TStackOperator_OP_NUMEQUALVERIFY : public TStackOperatorFromCompoundOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NUMEQUALVERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NUMEQUALVERIFY(*this); }
	eScriptOp getOpcode() const { return OP_NUMEQUALVERIFY; }

	void explode( TBitcoinScriptBase * ) const;
};

//
// Class: TStackOperator_OP_NUMNOTEQUAL
// Description:
//
class TStackOperator_OP_NUMNOTEQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NUMNOTEQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NUMNOTEQUAL(*this); }
	eScriptOp getOpcode() const { return OP_NUMNOTEQUAL; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_LESSTHAN
// Description:
//
class TStackOperator_OP_LESSTHAN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_LESSTHAN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_LESSTHAN(*this); }
	eScriptOp getOpcode() const { return OP_LESSTHAN; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_GREATERTHAN
// Description:
//
class TStackOperator_OP_GREATERTHAN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_GREATERTHAN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_GREATERTHAN(*this); }
	eScriptOp getOpcode() const { return OP_GREATERTHAN; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_LESSTHANOREQUAL
// Description:
//
class TStackOperator_OP_LESSTHANOREQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_LESSTHANOREQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_LESSTHANOREQUAL(*this); }
	eScriptOp getOpcode() const { return OP_LESSTHANOREQUAL; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_GREATERTHANOREQUAL
// Description:
//
class TStackOperator_OP_GREATERTHANOREQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_GREATERTHANOREQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_GREATERTHANOREQUAL(*this); }
	eScriptOp getOpcode() const { return OP_GREATERTHANOREQUAL; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_MIN
// Description:
//
class TStackOperator_OP_MIN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_MIN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_MIN(*this); }
	eScriptOp getOpcode() const { return OP_MIN; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_MAX
// Description:
//
class TStackOperator_OP_MAX : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_MAX"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_MAX(*this); }
	eScriptOp getOpcode() const { return OP_MAX; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_WITHIN
// Description:
//
class TStackOperator_OP_WITHIN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_WITHIN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_WITHIN(*this); }
	eScriptOp getOpcode() const { return OP_WITHIN; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_CryptographicDigest
// Description:
//
class TStackOperator_CryptographicDigest : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_CryptographicDigest"; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;

  protected:
	virtual TMessageDigest *createHasher() const = 0;
};

//
// Class: TStackOperator_OP_RIPEMD160
// Description:
//
class TStackOperator_OP_RIPEMD160 : public TStackOperator_CryptographicDigest
{
  public:
	const char *className() const { return "TStackOperator_OP_RIPEMD160"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RIPEMD160(*this); }
	eScriptOp getOpcode() const { return OP_RIPEMD160; }

  protected:
	virtual TMessageDigest *createHasher() const;
};

//
// Class: TStackOperator_OP_SHA1
// Description:
//
class TStackOperator_OP_SHA1 : public TStackOperator_CryptographicDigest
{
  public:
	const char *className() const { return "TStackOperator_OP_SHA1"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SHA1(*this); }
	eScriptOp getOpcode() const { return OP_SHA1; }

  protected:
	virtual TMessageDigest *createHasher() const;
};

//
// Class: TStackOperator_OP_SHA256
// Description:
//
class TStackOperator_OP_SHA256 : public TStackOperator_CryptographicDigest
{
  public:
	const char *className() const { return "TStackOperator_OP_SHA256"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SHA256(*this); }
	eScriptOp getOpcode() const { return OP_SHA256; }

  protected:
	virtual TMessageDigest *createHasher() const;
};

//
// Class: TStackOperator_OP_HASH160
// Description:
//
class TStackOperator_OP_HASH160 : public TStackOperator_CryptographicDigest
{
  public:
	const char *className() const { return "TStackOperator_OP_HASH160"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_HASH160(*this); }
	eScriptOp getOpcode() const { return OP_HASH160; }

  protected:
	virtual TMessageDigest *createHasher() const;
};

//
// Class: TStackOperator_OP_HASH256
// Description:
//
class TStackOperator_OP_HASH256 : public TStackOperator_CryptographicDigest
{
  public:
	const char *className() const { return "TStackOperator_OP_HASH256"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_HASH256(*this); }
	eScriptOp getOpcode() const { return OP_HASH256; }

  protected:
	virtual TMessageDigest *createHasher() const;
};

//
// Class: TStackOperator_OP_CODESEPARATOR
// Description:
//
class TStackOperator_OP_CODESEPARATOR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CODESEPARATOR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CODESEPARATOR(*this); }
	eScriptOp getOpcode() const { return OP_CODESEPARATOR; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_CHECKSIG
// Description:
//
class TStackOperator_OP_CHECKSIG : public TStackOperatorFromCompoundOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CHECKSIG"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CHECKSIG(*this); }
	eScriptOp getOpcode() const { return OP_CHECKSIG; }

	void explode( TBitcoinScriptBase * ) const;
};

//
// Class: TStackOperator_OP_CHECKSIGVERIFY
// Description:
//
class TStackOperator_OP_CHECKSIGVERIFY : public TStackOperatorFromCompoundOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CHECKSIGVERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CHECKSIGVERIFY(*this); }
	eScriptOp getOpcode() const { return OP_CHECKSIGVERIFY; }

	void explode( TBitcoinScriptBase * ) const;
};

//
// Class: TStackOperator_OP_CHECKMULTISIG
// Description:
//
class TStackOperator_OP_CHECKMULTISIG : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CHECKMULTISIG"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CHECKMULTISIG(*this); }
	eScriptOp getOpcode() const { return OP_CHECKMULTISIG; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_CHECKMULTISIGVERIFY
// Description:
//
class TStackOperator_OP_CHECKMULTISIGVERIFY : public TStackOperatorFromCompoundOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CHECKMULTISIGVERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CHECKMULTISIGVERIFY(*this); }
	eScriptOp getOpcode() const { return OP_CHECKMULTISIGVERIFY; }

	void explode( TBitcoinScriptBase * ) const;
};

//
// Class: TStackOperatorTemplate
// Description:
//
class TStackOperatorTemplate : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperatorTemplate"; }

	// Deny read() and execute()
	istream &read( istream & ) {
		throw logic_error("TStackOperatorTemplate should never be read()");
	}
	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer & ) const {
		throw logic_error("TStackOperatorTemplate should never be execute()d");
	}
};

//
// Class: TStackOperator_OP_PUBKEYHASH
// Description:
//
class TStackOperator_OP_PUBKEYHASH : public TStackOperatorTemplate
{
  public:
	const char *className() const { return "TStackOperator_OP_PUBKEYHASH"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_PUBKEYHASH(*this); }
	eScriptOp getOpcode() const { return OP_PUBKEYHASH; }
};

//
// Class: TStackOperator_OP_PUBKEY
// Description:
//
class TStackOperator_OP_PUBKEY : public TStackOperatorTemplate
{
  public:
	const char *className() const { return "TStackOperator_OP_PUBKEY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_PUBKEY(*this); }
	eScriptOp getOpcode() const { return OP_PUBKEY; }
};

//
// Class: TStackOperator_OP_RESERVED
// Description:
//
class TStackOperator_OP_RESERVED : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RESERVED"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RESERVED(*this); }
	eScriptOp getOpcode() const { return OP_RESERVED; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_VER
// Description:
//
class TStackOperator_OP_VER : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_VER"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_VER(*this); }
	eScriptOp getOpcode() const { return OP_VER; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_VERIF
// Description:
//
class TStackOperator_OP_VERIF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_VERIF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_VERIF(*this); }
	eScriptOp getOpcode() const { return OP_VERIF; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_VERNOTIF
// Description:
//
class TStackOperator_OP_VERNOTIF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_VERNOTIF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_VERNOTIF(*this); }
	eScriptOp getOpcode() const { return OP_VERNOTIF; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_RESERVED1
// Description:
//
class TStackOperator_OP_RESERVED1 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RESERVED1"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RESERVED1(*this); }
	eScriptOp getOpcode() const { return OP_RESERVED1; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_OP_RESERVED2
// Description:
//
class TStackOperator_OP_RESERVED2 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RESERVED2"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RESERVED2(*this); }
	eScriptOp getOpcode() const { return OP_RESERVED2; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
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

	ostream &write( ostream &os ) {
		os.put(OP);
		return os;
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

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
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

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
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
	TStackOperator_PUSH_N() {};
	TStackOperator_PUSH_N( const string &r ) { Raw = r; OP = static_cast<eScriptOp>(Raw.size()); }

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

	ostream &write( ostream &os ) {
		// Write the opcode byte
		TStackOperatorFromOpcode::write(os);
		// Write the bytes
		os.write( Raw.data(), Raw.size() );
		return os;
	}

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;

	ostream &printOn( ostream &os ) const;

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

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const { return ip; }
};

//
// Class: TStackOperatorInternal
// Description:
//
class TStackOperatorInternal : public TStackOperator
{
  public:
	const char *className() const { return "TStackOperatorInternal"; }
};

//
// Class: TStackOperator_INTOP_PUSHSUBSCRIPT
// Description:
//
class TStackOperator_INTOP_PUSHSUBSCRIPT : public TStackOperatorInternal
{
  public:
	TStackOperator *clone() const { return new TStackOperator_INTOP_PUSHSUBSCRIPT(*this); }
	const char *className() const { return "TStackOperator_INTOP_PUSHSUBSCRIPT"; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_INTOP_DELETESIG
// Description:
//
class TStackOperator_INTOP_DELETESIG : public TStackOperatorInternal
{
  public:
	TStackOperator *clone() const { return new TStackOperator_INTOP_DELETESIG(*this); }
	const char *className() const { return "TStackOperator_INTOP_DELETESIG"; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_INTOP_REMOVEHASHTYPE
// Description:
//
class TStackOperator_INTOP_REMOVEHASHTYPE : public TStackOperatorInternal
{
  public:
	TStackOperator *clone() const { return new TStackOperator_INTOP_REMOVEHASHTYPE(*this); }
	const char *className() const { return "TStackOperator_INTOP_REMOVEHASHTYPE"; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_INTOP_PUSHCOPYTRANSACTION
// Description:
//
class TStackOperator_INTOP_PUSHCOPYTRANSACTION : public TStackOperatorInternal
{
  public:
	TStackOperator *clone() const { return new TStackOperator_INTOP_PUSHCOPYTRANSACTION(*this); }
	const char *className() const { return "TStackOperator_INTOP_PUSHCOPYTRANSACTION"; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_INTOP_REMOVECODESEPARATORS
// Description:
//
class TStackOperator_INTOP_REMOVECODESEPARATORS : public TStackOperatorInternal
{
  public:
	TStackOperator *clone() const { return new TStackOperator_INTOP_REMOVECODESEPARATORS(*this); }
	const char *className() const { return "TStackOperator_INTOP_REMOVECODESEPARATORS"; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_INTOP_REMOVETXSCRIPTS
// Description:
//
class TStackOperator_INTOP_REMOVETXSCRIPTS : public TStackOperatorInternal
{
  public:
	TStackOperator *clone() const { return new TStackOperator_INTOP_REMOVETXSCRIPTS(*this); }
	const char *className() const { return "TStackOperator_INTOP_REMOVETXSCRIPTS"; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_INTOP_REPLACETXSCRIPT
// Description:
//
class TStackOperator_INTOP_REPLACETXSCRIPT : public TStackOperatorInternal
{
  public:
	TStackOperator *clone() const { return new TStackOperator_INTOP_REPLACETXSCRIPT(*this); }
	const char *className() const { return "TStackOperator_INTOP_REPLACETXSCRIPT"; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_INTOP_SIGHASH
// Description:
//
class TStackOperator_INTOP_SIGHASH : public TStackOperatorInternal
{
  public:
	TStackOperator *clone() const { return new TStackOperator_INTOP_SIGHASH(*this); }
	const char *className() const { return "TStackOperator_INTOP_SIGHASH"; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

//
// Class: TStackOperator_INTOP_FINALSIGNATURE
// Description:
//
class TStackOperator_INTOP_FINALSIGNATURE : public TStackOperatorInternal
{
  public:
	TStackOperator *clone() const { return new TStackOperator_INTOP_FINALSIGNATURE(*this); }
	const char *className() const { return "TStackOperator_INTOP_FINALSIGNATURE"; }

	TBitcoinScript::tInstructionPointer execute( TExecutionContext &, const TBitcoinScript::tInstructionPointer &ip ) const;
};

////
//// Class:	TScriptPatternElement
//// Description:
////
//class TScriptPatternElement
//{
//  public:
//	TScriptPatternElement();
//	TScriptPatternElement *clone() { return TScriptPatternElement(*this); }
//
//	bool is( TStackOperator * ) const = 0;
//};
//
////
//// Class:	TScriptPatternOperatorElement
//// Description:
////
//class TScriptPatternOperatorElement
//{
//  public:
//	TScriptPatternElement( TStackOperator * );
//	TScriptPatternElement *clone() { return TScriptPatternOperatorElement(*this); }
//
//  protected:
//
//};
//
//
//
////
//// Class:	TScriptPattern
//// Description:
////
//class TScriptPattern
//{
//  public:
//	void push( TScriptPatternElement * );
//
//	bool is( TBitcoinScript * ) const;
//};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
