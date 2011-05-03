// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   script.cc
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

// Module include
#include "script.h"

// -------------- Includes
// --- C
// --- C++
#include <iostream>
// --- Qt
// --- OS
// --- Project libs
// --- Project
#include "logstream.h"


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Class declarations

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
	virtual istream &readAndAppend( TBitcoinScript *, istream & ) const;
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

//
// Class: TStackOperatorFromCompoundOpcode
// Description:
//
class TStackOperatorFromCompoundOpcode : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperatorFromCompoundOpcode"; }
	istream &readAndAppend( TBitcoinScript *, istream &is ) const {
		// Discard the Opcode, we already know it's getOpcode()
		is.get();
		return is;
	}

	// Deny read() and execute()
	istream &read( istream & ) {
		throw logic_error("TStackOperatorFromCompoundOpcode should never be read()");
	}
	void execute( TExecutionStack & ) const {
		throw logic_error("TStackOperatorFromCompoundOpcode should never be execute()d");
	}
};

//
// Class: TStackOperator_OP_FALSE
// Desciption:
//
class TStackOperator_OP_FALSE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_FALSE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_FALSE(*this); }
	eScriptOp getOpcode() const { return OP_FALSE; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_PUSHDATAN
// Desciption:
//
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

	void execute( TExecutionStack &Stack ) const;

  protected:
	virtual streamsize getRawReadCount( istream & ) const = 0;

  protected:
	string Raw;
};

//
// Class: TStackOperator_OP_PUSHDATA1
// Desciption:
//
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

//
// Class: TStackOperator_OP_PUSHDATA2
// Desciption:
//
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

//
// Class: TStackOperator_OP_PUSHDATA4
// Desciption:
//
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

//
// Class: TStackOperator_OP_1NEGATE
// Desciption:
//
class TStackOperator_OP_1NEGATE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_1NEGATE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_1NEGATE(*this); }
	eScriptOp getOpcode() const { return OP_1NEGATE; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_TRUE
// Desciption:
//
class TStackOperator_OP_TRUE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_TRUE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_TRUE(*this); }
	eScriptOp getOpcode() const { return OP_TRUE; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_NOP
// Desciption:
//
class TStackOperator_OP_NOP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NOP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NOP(*this); }
	eScriptOp getOpcode() const { return OP_NOP; }

	void execute( TExecutionStack &Stack ) const {}
};

//
// Class: TStackOperator_OP_IF
// Desciption:
//
class TStackOperator_OP_IF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_IF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_IF(*this); }
	eScriptOp getOpcode() const { return OP_IF; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_NOTIF
// Desciption:
//
class TStackOperator_OP_NOTIF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NOTIF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NOTIF(*this); }
	eScriptOp getOpcode() const { return OP_NOTIF; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_ELSE
// Desciption:
//
class TStackOperator_OP_ELSE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ELSE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ELSE(*this); }
	eScriptOp getOpcode() const { return OP_ELSE; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_ENDIF
// Desciption:
//
class TStackOperator_OP_ENDIF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ENDIF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ENDIF(*this); }
	eScriptOp getOpcode() const { return OP_ENDIF; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_VERIFY
// Desciption:
//
class TStackOperator_OP_VERIFY : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_VERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_VERIFY(*this); }
	eScriptOp getOpcode() const { return OP_VERIFY; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_RETURN
// Desciption:
//
class TStackOperator_OP_RETURN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RETURN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RETURN(*this); }
	eScriptOp getOpcode() const { return OP_RETURN; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_TOALTSTACK
// Desciption:
//
class TStackOperator_OP_TOALTSTACK : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_TOALTSTACK"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_TOALTSTACK(*this); }
	eScriptOp getOpcode() const { return OP_TOALTSTACK; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_FROMALTSTACK
// Desciption:
//
class TStackOperator_OP_FROMALTSTACK : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_FROMALTSTACK"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_FROMALTSTACK(*this); }
	eScriptOp getOpcode() const { return OP_FROMALTSTACK; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_IFDUP
// Desciption:
//
class TStackOperator_OP_IFDUP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_IFDUP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_IFDUP(*this); }
	eScriptOp getOpcode() const { return OP_IFDUP; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_DEPTH
// Desciption:
//
class TStackOperator_OP_DEPTH : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_DEPTH"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_DEPTH(*this); }
	eScriptOp getOpcode() const { return OP_DEPTH; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_DROP
// Desciption:
//
class TStackOperator_OP_DROP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_DROP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_DROP(*this); }
	eScriptOp getOpcode() const { return OP_DROP; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_DUP
// Desciption:
//
class TStackOperator_OP_DUP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_DUP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_DUP(*this); }
	eScriptOp getOpcode() const { return OP_DUP; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_NIP
// Desciption:
//
class TStackOperator_OP_NIP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NIP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NIP(*this); }
	eScriptOp getOpcode() const { return OP_NIP; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_OVER
// Desciption:
//
class TStackOperator_OP_OVER : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_OVER"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_OVER(*this); }
	eScriptOp getOpcode() const { return OP_OVER; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_PICK
// Desciption:
//
class TStackOperator_OP_PICK : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_PICK"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_PICK(*this); }
	eScriptOp getOpcode() const { return OP_PICK; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_ROLL
// Desciption:
//
class TStackOperator_OP_ROLL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ROLL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ROLL(*this); }
	eScriptOp getOpcode() const { return OP_ROLL; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_ROT
// Desciption:
//
class TStackOperator_OP_ROT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ROT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ROT(*this); }
	eScriptOp getOpcode() const { return OP_ROT; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_SWAP
// Desciption:
//
class TStackOperator_OP_SWAP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SWAP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SWAP(*this); }
	eScriptOp getOpcode() const { return OP_SWAP; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_TUCK
// Desciption:
//
class TStackOperator_OP_TUCK : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_TUCK"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_TUCK(*this); }
	eScriptOp getOpcode() const { return OP_TUCK; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_2DROP
// Desciption:
//
class TStackOperator_OP_2DROP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2DROP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2DROP(*this); }
	eScriptOp getOpcode() const { return OP_2DROP; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_2DUP
// Desciption:
//
class TStackOperator_OP_2DUP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2DUP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2DUP(*this); }
	eScriptOp getOpcode() const { return OP_2DUP; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_3DUP
// Desciption:
//
class TStackOperator_OP_3DUP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_3DUP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_3DUP(*this); }
	eScriptOp getOpcode() const { return OP_3DUP; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_2OVER
// Desciption:
//
class TStackOperator_OP_2OVER : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2OVER"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2OVER(*this); }
	eScriptOp getOpcode() const { return OP_2OVER; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_2ROT
// Desciption:
//
class TStackOperator_OP_2ROT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2ROT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2ROT(*this); }
	eScriptOp getOpcode() const { return OP_2ROT; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_2SWAP
// Desciption:
//
class TStackOperator_OP_2SWAP : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2SWAP"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2SWAP(*this); }
	eScriptOp getOpcode() const { return OP_2SWAP; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_CAT
// Desciption:
//
class TStackOperator_OP_CAT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CAT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CAT(*this); }
	eScriptOp getOpcode() const { return OP_CAT; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_SUBSTR
// Desciption:
//
class TStackOperator_OP_SUBSTR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SUBSTR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SUBSTR(*this); }
	eScriptOp getOpcode() const { return OP_SUBSTR; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_LEFT
// Desciption:
//
class TStackOperator_OP_LEFT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_LEFT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_LEFT(*this); }
	eScriptOp getOpcode() const { return OP_LEFT; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_RIGHT
// Desciption:
//
class TStackOperator_OP_RIGHT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RIGHT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RIGHT(*this); }
	eScriptOp getOpcode() const { return OP_RIGHT; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_SIZE
// Desciption:
//
class TStackOperator_OP_SIZE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SIZE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SIZE(*this); }
	eScriptOp getOpcode() const { return OP_SIZE; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_INVERT
// Desciption:
//
class TStackOperator_OP_INVERT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_INVERT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_INVERT(*this); }
	eScriptOp getOpcode() const { return OP_INVERT; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_AND
// Desciption:
//
class TStackOperator_OP_AND : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_AND"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_AND(*this); }
	eScriptOp getOpcode() const { return OP_AND; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_OR
// Desciption:
//
class TStackOperator_OP_OR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_OR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_OR(*this); }
	eScriptOp getOpcode() const { return OP_OR; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_XOR
// Desciption:
//
class TStackOperator_OP_XOR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_XOR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_XOR(*this); }
	eScriptOp getOpcode() const { return OP_XOR; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_EQUAL
// Desciption:
//
class TStackOperator_OP_EQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_EQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_EQUAL(*this); }
	eScriptOp getOpcode() const { return OP_EQUAL; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_EQUALVERIFY
// Desciption:
//
class TStackOperator_OP_EQUALVERIFY : public TStackOperatorFromCompoundOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_EQUALVERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_EQUALVERIFY(*this); }
	eScriptOp getOpcode() const { return OP_EQUALVERIFY; }

	istream &readAndAppend( TBitcoinScript *, istream & ) const;
};

//
// Class: TStackOperator_OP_1ADD
// Desciption:
//
class TStackOperator_OP_1ADD : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_1ADD"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_1ADD(*this); }
	eScriptOp getOpcode() const { return OP_1ADD; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_1SUB
// Desciption:
//
class TStackOperator_OP_1SUB : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_1SUB"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_1SUB(*this); }
	eScriptOp getOpcode() const { return OP_1SUB; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_2MUL
// Desciption:
//
class TStackOperator_OP_2MUL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2MUL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2MUL(*this); }
	eScriptOp getOpcode() const { return OP_2MUL; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_2DIV
// Desciption:
//
class TStackOperator_OP_2DIV : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_2DIV"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_2DIV(*this); }
	eScriptOp getOpcode() const { return OP_2DIV; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_NEGATE
// Desciption:
//
class TStackOperator_OP_NEGATE : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NEGATE"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NEGATE(*this); }
	eScriptOp getOpcode() const { return OP_NEGATE; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_ABS
// Desciption:
//
class TStackOperator_OP_ABS : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ABS"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ABS(*this); }
	eScriptOp getOpcode() const { return OP_ABS; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_NOT
// Desciption:
//
class TStackOperator_OP_NOT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NOT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NOT(*this); }
	eScriptOp getOpcode() const { return OP_NOT; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_0NOTEQUAL
// Desciption:
//
class TStackOperator_OP_0NOTEQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_0NOTEQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_0NOTEQUAL(*this); }
	eScriptOp getOpcode() const { return OP_0NOTEQUAL; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_ADD
// Desciption:
//
class TStackOperator_OP_ADD : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_ADD"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_ADD(*this); }
	eScriptOp getOpcode() const { return OP_ADD; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_SUB
// Desciption:
//
class TStackOperator_OP_SUB : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SUB"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SUB(*this); }
	eScriptOp getOpcode() const { return OP_SUB; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_MUL
// Desciption:
//
class TStackOperator_OP_MUL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_MUL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_MUL(*this); }
	eScriptOp getOpcode() const { return OP_MUL; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_DIV
// Desciption:
//
class TStackOperator_OP_DIV : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_DIV"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_DIV(*this); }
	eScriptOp getOpcode() const { return OP_DIV; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_MOD
// Desciption:
//
class TStackOperator_OP_MOD : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_MOD"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_MOD(*this); }
	eScriptOp getOpcode() const { return OP_MOD; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_LSHIFT
// Desciption:
//
class TStackOperator_OP_LSHIFT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_LSHIFT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_LSHIFT(*this); }
	eScriptOp getOpcode() const { return OP_LSHIFT; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_RSHIFT
// Desciption:
//
class TStackOperator_OP_RSHIFT : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RSHIFT"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RSHIFT(*this); }
	eScriptOp getOpcode() const { return OP_RSHIFT; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_BOOLAND
// Desciption:
//
class TStackOperator_OP_BOOLAND : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_BOOLAND"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_BOOLAND(*this); }
	eScriptOp getOpcode() const { return OP_BOOLAND; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_BOOLOR
// Desciption:
//
class TStackOperator_OP_BOOLOR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_BOOLOR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_BOOLOR(*this); }
	eScriptOp getOpcode() const { return OP_BOOLOR; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_NUMEQUAL
// Desciption:
//
class TStackOperator_OP_NUMEQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NUMEQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NUMEQUAL(*this); }
	eScriptOp getOpcode() const { return OP_NUMEQUAL; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_NUMEQUALVERIFY
// Desciption:
//
class TStackOperator_OP_NUMEQUALVERIFY : public TStackOperatorFromCompoundOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NUMEQUALVERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NUMEQUALVERIFY(*this); }
	eScriptOp getOpcode() const { return OP_NUMEQUALVERIFY; }

	istream &readAndAppend( TBitcoinScript *, istream & ) const;
};

//
// Class: TStackOperator_OP_NUMNOTEQUAL
// Desciption:
//
class TStackOperator_OP_NUMNOTEQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_NUMNOTEQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_NUMNOTEQUAL(*this); }
	eScriptOp getOpcode() const { return OP_NUMNOTEQUAL; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_LESSTHAN
// Desciption:
//
class TStackOperator_OP_LESSTHAN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_LESSTHAN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_LESSTHAN(*this); }
	eScriptOp getOpcode() const { return OP_LESSTHAN; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_GREATERTHAN
// Desciption:
//
class TStackOperator_OP_GREATERTHAN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_GREATERTHAN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_GREATERTHAN(*this); }
	eScriptOp getOpcode() const { return OP_GREATERTHAN; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_LESSTHANOREQUAL
// Desciption:
//
class TStackOperator_OP_LESSTHANOREQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_LESSTHANOREQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_LESSTHANOREQUAL(*this); }
	eScriptOp getOpcode() const { return OP_LESSTHANOREQUAL; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_GREATERTHANOREQUAL
// Desciption:
//
class TStackOperator_OP_GREATERTHANOREQUAL : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_GREATERTHANOREQUAL"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_GREATERTHANOREQUAL(*this); }
	eScriptOp getOpcode() const { return OP_GREATERTHANOREQUAL; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_MIN
// Desciption:
//
class TStackOperator_OP_MIN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_MIN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_MIN(*this); }
	eScriptOp getOpcode() const { return OP_MIN; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_MAX
// Desciption:
//
class TStackOperator_OP_MAX : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_MAX"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_MAX(*this); }
	eScriptOp getOpcode() const { return OP_MAX; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_WITHIN
// Desciption:
//
class TStackOperator_OP_WITHIN : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_WITHIN"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_WITHIN(*this); }
	eScriptOp getOpcode() const { return OP_WITHIN; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_RIPEMD160
// Desciption:
//
class TStackOperator_OP_RIPEMD160 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RIPEMD160"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RIPEMD160(*this); }
	eScriptOp getOpcode() const { return OP_RIPEMD160; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_SHA1
// Desciption:
//
class TStackOperator_OP_SHA1 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SHA1"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SHA1(*this); }
	eScriptOp getOpcode() const { return OP_SHA1; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_SHA256
// Desciption:
//
class TStackOperator_OP_SHA256 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_SHA256"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_SHA256(*this); }
	eScriptOp getOpcode() const { return OP_SHA256; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_HASH160
// Desciption:
//
class TStackOperator_OP_HASH160 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_HASH160"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_HASH160(*this); }
	eScriptOp getOpcode() const { return OP_HASH160; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_HASH256
// Desciption:
//
class TStackOperator_OP_HASH256 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_HASH256"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_HASH256(*this); }
	eScriptOp getOpcode() const { return OP_HASH256; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_CODESEPARATOR
// Desciption:
//
class TStackOperator_OP_CODESEPARATOR : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CODESEPARATOR"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CODESEPARATOR(*this); }
	eScriptOp getOpcode() const { return OP_CODESEPARATOR; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_CHECKSIG
// Desciption:
//
class TStackOperator_OP_CHECKSIG : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CHECKSIG"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CHECKSIG(*this); }
	eScriptOp getOpcode() const { return OP_CHECKSIG; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_CHECKSIGVERIFY
// Desciption:
//
class TStackOperator_OP_CHECKSIGVERIFY : public TStackOperatorFromCompoundOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CHECKSIGVERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CHECKSIGVERIFY(*this); }
	eScriptOp getOpcode() const { return OP_CHECKSIGVERIFY; }

	istream &readAndAppend( TBitcoinScript *, istream & ) const;
};

//
// Class: TStackOperator_OP_CHECKMULTISIG
// Desciption:
//
class TStackOperator_OP_CHECKMULTISIG : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CHECKMULTISIG"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CHECKMULTISIG(*this); }
	eScriptOp getOpcode() const { return OP_CHECKMULTISIG; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_CHECKMULTISIGVERIFY
// Desciption:
//
class TStackOperator_OP_CHECKMULTISIGVERIFY : public TStackOperatorFromCompoundOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_CHECKMULTISIGVERIFY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_CHECKMULTISIGVERIFY(*this); }
	eScriptOp getOpcode() const { return OP_CHECKMULTISIGVERIFY; }

	istream &readAndAppend( TBitcoinScript *, istream & ) const;
};

//
// Class: TStackOperator_OP_PUBKEYHASH
// Desciption:
//
class TStackOperator_OP_PUBKEYHASH : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_PUBKEYHASH"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_PUBKEYHASH(*this); }
	eScriptOp getOpcode() const { return OP_PUBKEYHASH; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_PUBKEY
// Desciption:
//
class TStackOperator_OP_PUBKEY : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_PUBKEY"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_PUBKEY(*this); }
	eScriptOp getOpcode() const { return OP_PUBKEY; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_RESERVED
// Desciption:
//
class TStackOperator_OP_RESERVED : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RESERVED"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RESERVED(*this); }
	eScriptOp getOpcode() const { return OP_RESERVED; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_VER
// Desciption:
//
class TStackOperator_OP_VER : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_VER"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_VER(*this); }
	eScriptOp getOpcode() const { return OP_VER; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_VERIF
// Desciption:
//
class TStackOperator_OP_VERIF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_VERIF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_VERIF(*this); }
	eScriptOp getOpcode() const { return OP_VERIF; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_VERNOTIF
// Desciption:
//
class TStackOperator_OP_VERNOTIF : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_VERNOTIF"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_VERNOTIF(*this); }
	eScriptOp getOpcode() const { return OP_VERNOTIF; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_RESERVED1
// Desciption:
//
class TStackOperator_OP_RESERVED1 : public TStackOperatorFromOpcode
{
  public:
	const char *className() const { return "TStackOperator_OP_RESERVED1"; }
	TStackOperatorFromStream *clone() const { return new TStackOperator_OP_RESERVED1(*this); }
	eScriptOp getOpcode() const { return OP_RESERVED1; }

	void execute( TExecutionStack &Stack ) const;
};

//
// Class: TStackOperator_OP_RESERVED2
// Desciption:
//
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

	void execute( TExecutionStack &Stack ) const;
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

	void execute( TExecutionStack &Stack ) const;

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

// -------------- Template instantiations


// -------------- Class declarations

//
// Function:	TStackElement_t :: printOn
// Description:
//
template <typename t>
ostream &TStackElement_t<t>::printOn( ostream &s ) const
{
	s << Data;
	return s;
}

//
// Function:	TStackElement_t :: printOn
// Description:
//
template <>
ostream &TStackElement_t<string>::printOn( ostream &s ) const
{
	TLog::hexify( s, Data );
	return s;
}

// -----------
//
// Function:	TExecutionStack :: TExecutionStack
// Description:
//
TExecutionStack::TExecutionStack() :
	Invalid( false )
{
}

//
// Function:	TExecutionStack :: printOn
// Description:
//
ostream &TExecutionStack::printOn( ostream &s ) const
{
	list<TStackElement*>::const_iterator it;
	for( it = Stack.begin(); it != Stack.end(); it++ ) {
		s << " - ";
		(*it)->printOn(s);
		s << endl;
	}

	return s;
}

// -----------

//
// Function:	TBitcoinScript :: TBitcoinScript
// Description:
//
TBitcoinScript::TBitcoinScript() :
	Initialised( false )
{
}

//
// Function:	TBitcoinScript :: ~TBitcoinScript
// Description:
//
TBitcoinScript::~TBitcoinScript()
{
	while( !Program.empty() ) {
		delete Program.front();
		Program.erase( Program.begin() );
	}
	while( !Templates.empty() ) {
		delete Templates.front();
		Templates.erase( Templates.begin() );
	}
}

//
// Function:	TBitcoinScript :: init
// Description:
//
void TBitcoinScript::init()
{
	Initialised = true;
}

//
// Function:	TBitcoinScript :: read
// Description:
//
istream &TBitcoinScript::read( istream &is )
{
	eScriptOp Opcode;
	list<const TStackOperatorFromStream *>::const_iterator it;

	// Load the template list
	if( !Initialised )
		init();

	// Peek at each opcode, then ask each template if it will accept
	// that opcode.  Finding one that does, we clone it and let it read
	// itself from the stream.
	while( (Opcode = static_cast<eScriptOp>(is.peek()) ) != ios::traits_type::eof() ) {
		for( it = Templates.begin(); it != Templates.end(); it++ ) {
			if( !(*it)->acceptOpcode( Opcode ) )
				continue;
			break;
		}
		if( it == Templates.end() ) {
			log() << "script opcode (" << Opcode << ") found" << endl;
			throw script_parse_error_not_found();
		}
		log() << "Reading " << (*it)->className() << endl;
		(*it)->readAndAppend( this, is );

		// Should leave us pointing at next character
	}

	return is;
}

// -----------

//
// Function:	TBitcoinScript_0 :: TBitcoinScript_0
// Description:
//
TBitcoinScript_0::TBitcoinScript_0()
{
}

//
// Function:	TBitcoinScript_0 :: getMinimumAcceptedVersion
// Description:
//
uint32_t TBitcoinScript_0::getMinimumAcceptedVersion() const
{
	return 0;
}

//
// Function:	TBitcoinScript_0 :: init
// Description:
//
void TBitcoinScript_0::init()
{
	Templates.push_back( new TStackOperator_OP_FALSE );
	Templates.push_back( new TStackOperator_OP_PUSHDATA1 );
	Templates.push_back( new TStackOperator_OP_PUSHDATA2 );
	Templates.push_back( new TStackOperator_OP_PUSHDATA4 );
	Templates.push_back( new TStackOperator_OP_1NEGATE );
	Templates.push_back( new TStackOperator_OP_TRUE );
	Templates.push_back( new TStackOperator_OP_NOP );
	Templates.push_back( new TStackOperator_OP_IF );
	Templates.push_back( new TStackOperator_OP_NOTIF );
	Templates.push_back( new TStackOperator_OP_ELSE );
	Templates.push_back( new TStackOperator_OP_ENDIF );
	Templates.push_back( new TStackOperator_OP_VERIFY );
	Templates.push_back( new TStackOperator_OP_RETURN );
	Templates.push_back( new TStackOperator_OP_TOALTSTACK );
	Templates.push_back( new TStackOperator_OP_FROMALTSTACK );
	Templates.push_back( new TStackOperator_OP_IFDUP );
	Templates.push_back( new TStackOperator_OP_DEPTH );
	Templates.push_back( new TStackOperator_OP_DROP );
	Templates.push_back( new TStackOperator_OP_DUP );
	Templates.push_back( new TStackOperator_OP_NIP );
	Templates.push_back( new TStackOperator_OP_OVER );
	Templates.push_back( new TStackOperator_OP_PICK );
	Templates.push_back( new TStackOperator_OP_ROLL );
	Templates.push_back( new TStackOperator_OP_ROT );
	Templates.push_back( new TStackOperator_OP_SWAP );
	Templates.push_back( new TStackOperator_OP_TUCK );
	Templates.push_back( new TStackOperator_OP_2DROP );
	Templates.push_back( new TStackOperator_OP_2DUP );
	Templates.push_back( new TStackOperator_OP_3DUP );
	Templates.push_back( new TStackOperator_OP_2OVER );
	Templates.push_back( new TStackOperator_OP_2ROT );
	Templates.push_back( new TStackOperator_OP_2SWAP );
//	Templates.push_back( new TStackOperator_OP_CAT );
//	Templates.push_back( new TStackOperator_OP_SUBSTR );
//	Templates.push_back( new TStackOperator_OP_LEFT );
//	Templates.push_back( new TStackOperator_OP_RIGHT );
	Templates.push_back( new TStackOperator_OP_SIZE );
//	Templates.push_back( new TStackOperator_OP_INVERT );
//	Templates.push_back( new TStackOperator_OP_AND );
//	Templates.push_back( new TStackOperator_OP_OR );
//	Templates.push_back( new TStackOperator_OP_XOR );
	Templates.push_back( new TStackOperator_OP_EQUAL );
	Templates.push_back( new TStackOperator_OP_EQUALVERIFY );
	Templates.push_back( new TStackOperator_OP_1ADD );
	Templates.push_back( new TStackOperator_OP_1SUB );
//	Templates.push_back( new TStackOperator_OP_2MUL );
//	Templates.push_back( new TStackOperator_OP_2DIV );
	Templates.push_back( new TStackOperator_OP_NEGATE );
	Templates.push_back( new TStackOperator_OP_ABS );
	Templates.push_back( new TStackOperator_OP_NOT );
	Templates.push_back( new TStackOperator_OP_0NOTEQUAL );
	Templates.push_back( new TStackOperator_OP_ADD );
	Templates.push_back( new TStackOperator_OP_SUB );
//	Templates.push_back( new TStackOperator_OP_MUL );
//	Templates.push_back( new TStackOperator_OP_DIV );
//	Templates.push_back( new TStackOperator_OP_MOD );
//	Templates.push_back( new TStackOperator_OP_LSHIFT );
//	Templates.push_back( new TStackOperator_OP_RSHIFT );
	Templates.push_back( new TStackOperator_OP_BOOLAND );
	Templates.push_back( new TStackOperator_OP_BOOLOR );
	Templates.push_back( new TStackOperator_OP_NUMEQUAL );
	Templates.push_back( new TStackOperator_OP_NUMEQUALVERIFY );
	Templates.push_back( new TStackOperator_OP_NUMNOTEQUAL );
	Templates.push_back( new TStackOperator_OP_LESSTHAN );
	Templates.push_back( new TStackOperator_OP_GREATERTHAN );
	Templates.push_back( new TStackOperator_OP_LESSTHANOREQUAL );
	Templates.push_back( new TStackOperator_OP_GREATERTHANOREQUAL );
	Templates.push_back( new TStackOperator_OP_MIN );
	Templates.push_back( new TStackOperator_OP_MAX );
	Templates.push_back( new TStackOperator_OP_WITHIN );
	Templates.push_back( new TStackOperator_OP_RIPEMD160 );
	Templates.push_back( new TStackOperator_OP_SHA1 );
	Templates.push_back( new TStackOperator_OP_SHA256 );
	Templates.push_back( new TStackOperator_OP_HASH160 );
	Templates.push_back( new TStackOperator_OP_HASH256 );
	Templates.push_back( new TStackOperator_OP_CODESEPARATOR );
	Templates.push_back( new TStackOperator_OP_CHECKSIG );
	Templates.push_back( new TStackOperator_OP_CHECKSIGVERIFY );
	Templates.push_back( new TStackOperator_OP_CHECKMULTISIG );
	Templates.push_back( new TStackOperator_OP_CHECKMULTISIGVERIFY );
	Templates.push_back( new TStackOperator_OP_PUBKEYHASH );
	Templates.push_back( new TStackOperator_OP_PUBKEY );
	Templates.push_back( new TStackOperator_OP_RESERVED );
	Templates.push_back( new TStackOperator_OP_VER );
	Templates.push_back( new TStackOperator_OP_VERIF );
	Templates.push_back( new TStackOperator_OP_VERNOTIF );
	Templates.push_back( new TStackOperator_OP_RESERVED1 );
	Templates.push_back( new TStackOperator_OP_RESERVED2 );
	Templates.push_back( new TStackOperator_OP_NOP_N );
	Templates.push_back( new TStackOperator_OP_N );
	Templates.push_back( new TStackOperator_PUSH_N );
	// OP_INVALIDOPCODE _must_ be last in the template list, this is
	// because it is a special case and has an acceptOpcode() that
	// always returns true.  Therefore, it will take up any opcode not
	// previously accepted by one of the above.
	Templates.push_back( new TStackOperator_OP_INVALIDOPCODE );

	TBitcoinScript::init();
}

// -----------

//
// Function:	TStackOperatorFromStream :: readAndAppend
// Description:
//
// We are the template, called because our acceptOpcode() returned true
//
// By default we simply clone ourself and get the clone to read itself.
//
istream &TStackOperatorFromStream::readAndAppend( TBitcoinScript *Script, istream &is ) const
{
	TStackOperatorFromStream *Operator = clone();
	Script->append( Operator );
	return Operator->read(is);
}

// -----------

//
// Function:  OP_FALSE
// Input:     Nothing
// Output:    False
// Operation:
//
void TStackOperator_OP_FALSE::execute( TExecutionStack &Stack ) const
{
	Stack().push_back( new TStackElementBoolean(false) );
}

//
// Function:  OP_PUSHDATAN
// Input:     
// Output:    
// Operation:
//
void TStackOperator_OP_PUSHDATAN::execute( TExecutionStack &Stack ) const
{
	Stack().push_back( new TStackElementString(Raw) );
}

//
// Function:  OP_1NEGATE
// Input:     
// Output:    
// Operation:
//
void TStackOperator_OP_1NEGATE::execute( TExecutionStack &Stack ) const
{
	Stack().push_back( new TStackElementInteger(-1) );
}

//
// Function:  OP_TRUE
// Input:     
// Output:    
// Operation:
//
void TStackOperator_OP_TRUE::execute( TExecutionStack &Stack ) const
{
	Stack().push_back( new TStackElementBoolean(true) );
}

//
// Function:  OP_IF
// <expression> if [statements] [else [statements]] endif
//
// Operation: If the top stack value is 1, the statements are executed.
// The top stack value is removed.
//
void TStackOperator_OP_IF::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_NOTIF
// <expression> if [statements] [else [statements]] endif
//
// If the top stack value is 0, the statements are executed. The top
// stack value is removed.
//
void TStackOperator_OP_NOTIF::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_ELSE
// <expression> if [statements] [else [statements]] endif
//
// If the preceding OP_IF or OP_NOTIF was not executed then these
// statements are.
//
void TStackOperator_OP_ELSE::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_ENDIF
// <expression> if [statements] [else [statements]] endif
//
// Ends an if/else block.
//
void TStackOperator_OP_ENDIF::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_RETURN
// Input:     True / false
// Output:    Nothing / False
// Operation: Marks transaction as invalid if top stack value is not
// true. True is removed, but false is not.
//
void TStackOperator_OP_VERIFY::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_RETURN
// Input:     Nothing
// Outpu:     Nothing
// Operation: Marks transaction as invalid.
//
void TStackOperator_OP_RETURN::execute( TExecutionStack &Stack ) const
{
}

// Function:  OP_TOALTSTACK
// Input:     x1
// Output:    (alt)x1
// Operation: Puts the input onto the top of the alt stack. Removes it
// from the main stack.
void TStackOperator_OP_TOALTSTACK::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_FROMALTSTACK
// Input:     (alt)x1
// Output:    x1
// Operation: Puts the input onto the top of the main stack. Removes it
// from the alt stack.
void TStackOperator_OP_FROMALTSTACK::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_IFDUP
// Input:     x
// Output:    x / x x
// Operation: If the input is true or false, duplicate it.
void TStackOperator_OP_IFDUP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_DEPTH
// Input:     Nothing
// Output:    <Stack size>
// Operation: Puts the number of stack items onto the stack.
void TStackOperator_OP_DEPTH::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_DROP
// Input:     x
// Output:    Nothing
// Operation: Removes the top stack item.
void TStackOperator_OP_DROP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_DUP
// Input:     x
// Output:    x x
// Operation: Duplicates the top stack item.
void TStackOperator_OP_DUP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_NIP
// Input:     x1 x2
// Output:    x2
// Operation: Removes the second-to-top stack item.
void TStackOperator_OP_NIP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_OVER
// Input:     x1 x2
// Output:    x1 x2 x1
// Operation: Copies the second-to-top stack item to the top.
void TStackOperator_OP_OVER::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_PICK
// Input:     xn ... x2 x1 x0 <n>
// Output:    xn ... x2 x1 x0 xn
// Operation: The item n back in the stack is copied to the top.
void TStackOperator_OP_PICK::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_ROLL
// Input:     xn ... x2 x1 x0 <n>
// Output:    ... x2 x1 x0 xn
// Operation: The item n back in the stack is moved to the top.
void TStackOperator_OP_ROLL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_ROT
// Input:     x1 x2 x3
// Output:    x2 x3 x1
// Operation: The top three items on the stack are rotated to the left.
void TStackOperator_OP_ROT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_SWAP
// Input:     x1 x2
// Output:    x2 x1
// Operation: The top two items on the stack are swapped.
void TStackOperator_OP_SWAP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_TUCK
// Input:     x1 x2
// Output:    x2 x1 x2
// Operation: The item at the top of the stack is copied and inserted
// before the second-to-top item.
void TStackOperator_OP_TUCK::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2DROP
// Input:     x1 x2
// Output:    Nothing
// Operation: Removes the top two stack items.
void TStackOperator_OP_2DROP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2DUP
// Input:     x1 x2
// Output:    x1 x2 x1 x2
// Operation: Duplicates the top two stack items.
void TStackOperator_OP_2DUP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_3DUP
// Input:     x1 x2 x3
// Output:    x1 x2 x3 x1 x2 x3
// Operation: Duplicates the top three stack items.
void TStackOperator_OP_3DUP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2OVER
// Input:     x1 x2 x3 x4
// Output:    x1 x2 x3 x4 x1 x2
// Operation: Copies the pair of items two spaces back in the stack to
// the front.
void TStackOperator_OP_2OVER::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2ROT
// Input:     x1 x2 x3 x4 x5 x6
// Output:    x3 x4 x5 x6 x1 x2
// Operation: The fifth and sixth items back are moved to the top of the
// stack.
void TStackOperator_OP_2ROT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2SWAP
// Input:     x1 x2 x3 x4
// Output:    x3 x4 x1 x2
// Operation: Swaps the top two pairs of items.
void TStackOperator_OP_2SWAP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_CAT
// Input:     x1 x2
// Output:    out
// Operation: Concatenates two strings. Currently disabled.
void TStackOperator_OP_CAT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_SUBSTR
// Input:     in begin size
// Output:    out
// Operation: Returns a section of a string. Currently disabled.
void TStackOperator_OP_SUBSTR::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_LEFT
// Input:     in size
// Output:    out
// Operation: Keeps only characters left of the specified point in a
// string. Currently disabled.
void TStackOperator_OP_LEFT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_RIGHT
// Input:     in size
// Output:    out
// Operation: Keeps only characters right of the specified point in a
// string. Currently disabled.
void TStackOperator_OP_RIGHT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_SIZE
// Input:     in
// Output:    in size
// Operation: Returns the length of the input string.
void TStackOperator_OP_SIZE::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_INVERT
// Input:     in
// Output:    out
// Operation: Flips all of the bits in the input. Currently disabled.
void TStackOperator_OP_INVERT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_AND
// Input:     x1 x2
// Output:    out
// Operation: Boolean and between each bit in the inputs. Currently
// disabled.
void TStackOperator_OP_AND::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_OR
// Input:     x1 x2
// Output:    out
// Operation: Boolean or between each bit in the inputs. Currently
// disabled.
void TStackOperator_OP_OR::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_XOR
// Input:     x1 x2
// Output:    out
// Operation: Boolean exclusive or between each bit in the inputs.
// Currently disabled.
void TStackOperator_OP_XOR::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_EQUAL
// Input:     x1 x2
// Output:    True / false
// Operation: Returns 1 if the inputs are exactly equal, 0 otherwise.
void TStackOperator_OP_EQUAL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_EQUALVERIFY
// Input:     x1 x2
// Output:    True / false
// Operation: Same as OP_EQUAL, but runs OP_VERIFY afterward.
istream &TStackOperator_OP_EQUALVERIFY::readAndAppend( TBitcoinScript *Script, istream &is ) const
{
	// Read the opcode
	TStackOperatorFromCompoundOpcode::readAndAppend( Script, is );
	Script->append( new TStackOperator_OP_EQUAL );
	Script->append( new TStackOperator_OP_VERIFY );
	return is;
}

//
// Function:  OP_1ADD
// Input:     in
// Output:    out
// Operation: 1 is added to the input.
void TStackOperator_OP_1ADD::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_1SUB
// Input:     in
// Output:    out
// Operation: 1 is subtracted from the input.
void TStackOperator_OP_1SUB::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2MUL
// Input:     in
// Output:    out
// Operation: The input is multiplied by 2. Currently disabled.
void TStackOperator_OP_2MUL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2DIV
// Input:     in
// Output:    out
// Operation: The input is divided by 2. Currently disabled.
void TStackOperator_OP_2DIV::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_NEGATE
// Input:     in
// Output:    out
// Operation: The sign of the input is flipped.
void TStackOperator_OP_NEGATE::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_ABS
// Input:     in
// Output:    out
// Operation: The input is made positive.
void TStackOperator_OP_ABS::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_NOT
// Input:     in
// Output:    out
// Operation: If the input is 0 or 1, it is flipped. Otherwise the
// output will be 0.
void TStackOperator_OP_NOT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_0NOTEQUAL
// Input:     in
// Output:    out
// Operation: Returns 1 if the input is 0. 0 otherwise.
void TStackOperator_OP_0NOTEQUAL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_ADD
// Input:     a b
// Output:    out
// Operation: a is added to b.
void TStackOperator_OP_ADD::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_SUB
// Input:     a b
// Output:    out
// Operation: b is subtracted from a.
void TStackOperator_OP_SUB::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_MUL
// Input:     a b
// Output:    out
// Operation: a is multiplied by b. Currently disabled.
void TStackOperator_OP_MUL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_DIV
// Input:     a b
// Output:    out
// Operation: a is divided by b. Currently disabled.
void TStackOperator_OP_DIV::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_MOD
// Input:     a b
// Output:    out
// Operation: Returns the remainder after dividing a by b. Currently
// disabled.
void TStackOperator_OP_MOD::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_LSHIFT
// Input:     a b
// Output:    out
// Operation: Shifts a left b bits, preserving sign. Currently disabled.
void TStackOperator_OP_LSHIFT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_RSHIFT
// Input:     a b
// Output:    out
// Operation: Shifts a right b bits, preserving sign. Currently disabled.
void TStackOperator_OP_RSHIFT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_BOOLAND
// Input:     a b
// Output:    out
// Operation: If both a and b are not 0, the output is 1. Otherwise 0.
void TStackOperator_OP_BOOLAND::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_BOOLOR
// Input:     a b
// Output:    out
// Operation: If a or b is not 0, the output is 1. Otherwise 0.
void TStackOperator_OP_BOOLOR::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_NUMEQUAL
// Input:     a b
// Output:    out
// Operation: Returns 1 if the numbers are equal, 0 otherwise.
void TStackOperator_OP_NUMEQUAL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_NUMEQUALVERIFY
// Input:     a b
// Output:    out
// Operation: Same as OP_NUMEQUAL, but runs OP_VERIFY afterward.
istream &TStackOperator_OP_NUMEQUALVERIFY::readAndAppend( TBitcoinScript *Script, istream &is ) const
{
	// Read the opcode
	TStackOperatorFromCompoundOpcode::readAndAppend( Script, is );
	Script->append( new TStackOperator_OP_NUMEQUAL );
	Script->append( new TStackOperator_OP_VERIFY );
	return is;
}

//
// Function:  OP_NUMNOTEQUAL
// Input:     a b
// Output:    out
// Operation: Returns 1 if the numbers are not equal, 0 otherwise.
void TStackOperator_OP_NUMNOTEQUAL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_LESSTHAN
// Input:     a b
// Output:    out
// Operation: Returns 1 if a is less than b, 0 otherwise.
void TStackOperator_OP_LESSTHAN::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_GREATERTHAN
// Input:     a b
// Output:    out
// Operation: Returns 1 if a is greater than b, 0 otherwise.
void TStackOperator_OP_GREATERTHAN::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_LESSTHANOREQUAL
// Input:     a b
// Output:    out
// Operation: Returns 1 if a is less than or equal to b, 0 otherwise.
void TStackOperator_OP_LESSTHANOREQUAL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_GREATERTHANOREQUAL
// Input:     a b
// Output:    out
// Operation: Returns 1 if a is greater than or equal to b, 0 otherwise.
void TStackOperator_OP_GREATERTHANOREQUAL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_MIN
// Input:     a b
// Output:    out
// Operation: Returns the smaller of a and b.
void TStackOperator_OP_MIN::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_MAX
// Input:     a b
// Output:    out
// Operation: Returns the larger of a and b.
void TStackOperator_OP_MAX::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_WITHIN
// Input:     x min max
// Output:    out
// Operation: Returns 1 if x is within the specified range
// (left-inclusive), 0 otherwise.
void TStackOperator_OP_WITHIN::execute( TExecutionStack &Stack ) const
{
}


//
// Function:  OP_RIPEMD160
// Input:     in
// Output:    hash
// Operation: The input is hashed using RIPEMD-160.
void TStackOperator_OP_RIPEMD160::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_SHA1
// Input:     in
// Output:    hash
// Operation: The input is hashed using SHA-1.
void TStackOperator_OP_SHA1::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_SHA256
// Input:     in
// Output:    hash
// Operation: The input is hashed using SHA-256.
void TStackOperator_OP_SHA256::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_HASH160
// Input:     in
// Output:    hash
// Operation: The input is hashed twice: first with SHA-256 and then
// with RIPEMD-160.
void TStackOperator_OP_HASH160::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_HASH256
// Input:     in
// Output:    hash
// Operation: The input is hashed two times with SHA-256.
void TStackOperator_OP_HASH256::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_CODESEPARATOR
// Input:     Nothing
// Output:    Nothing
// Operation: All of the signature checking words will only match
// signatures to the data after the most recently-executed
// OP_CODESEPARATOR.
void TStackOperator_OP_CODESEPARATOR::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_CHECKSIG
// Input:     sig pubkey
// Output:    True / false
// Operation: The entire transaction's outputs, inputs, and script (from
// the most recently-executed OP_CODESEPARATOR to the end) are hashed.
// The signature used by OP_CHECKSIG must be a valid signature for this
// hash and public key. If it is, 1 is returned, 0 otherwise.
void TStackOperator_OP_CHECKSIG::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_CHECKSIGVERIFY
// Input:     sig pubkey
// Output:    True / false
// Operation: Same as OP_CHECKSIG, but OP_VERIFY is executed afterward.
istream &TStackOperator_OP_CHECKSIGVERIFY::readAndAppend( TBitcoinScript *Script, istream &is ) const
{
	// Read the opcode
	TStackOperatorFromCompoundOpcode::readAndAppend( Script, is );
	Script->append( new TStackOperator_OP_CHECKSIG );
	Script->append( new TStackOperator_OP_VERIFY );
	return is;
}

//
// Function:  OP_CHECKMULTISIG
// Input:     sig1 sig2 ... <number of signatures> pub1 pub2 <number of public keys>
// Output:    True / False
// Operation: For each signature and public key pair, OP_CHECKSIG is
// executed. If more public keys than signatures are listed, some
// key/sig pairs can fail. All signatures need to match a public key. If
// all signatures are valid, 1 is returned, 0 otherwise.
void TStackOperator_OP_CHECKMULTISIG::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_CHECKMULTISIGVERIFY
// Input:     sig1 sig2 ... <number of signatures> pub1 pub2 ... <number of public keys>
// Output:    True / False
// Operation: Same as OP_CHECKMULTISIG, but OP_VERIFY is executed afterward.
istream &TStackOperator_OP_CHECKMULTISIGVERIFY::readAndAppend( TBitcoinScript *Script, istream &is ) const
{
	// Read the opcode
	TStackOperatorFromCompoundOpcode::readAndAppend( Script, is );
	Script->append( new TStackOperator_OP_CHECKMULTISIGVERIFY );
	Script->append( new TStackOperator_OP_VERIFY );
	return is;
}

//
// Function:  OP_PUBKEYHASH
// Operation: Represents a public key hashed with OP_HASH160.
void TStackOperator_OP_PUBKEYHASH::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_PUBKEY
// Operation: Represents a public key compatible with OP_CHECKSIG.
void TStackOperator_OP_PUBKEY::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_INVALIDOPCODE
// Operation: Matches any opcode that is not yet assigned.
void TStackOperator_OP_INVALIDOPCODE::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_N
void TStackOperator_OP_N::execute( TExecutionStack &Stack ) const
{
	Stack().push_back( new TStackElementInteger( OP-OP_2 ) );
}

//
// Function:  PUSH_N
void TStackOperator_PUSH_N::execute( TExecutionStack &Stack ) const
{
	Stack().push_back( new TStackElementString( Raw ) );
}

//
// Function:  OP_RESERVED
// Operation: Transaction is invalid
void TStackOperator_OP_RESERVED::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_VER
// Operation: Transaction is invalid
void TStackOperator_OP_VER::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_VERIF
// Operation: Transaction is invalid
void TStackOperator_OP_VERIF::execute( TExecutionStack &Stack ) const
{
}

//
// OP_VERNOTIF
// Transaction is invalid
void TStackOperator_OP_VERNOTIF::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_RESERVED1
// Operation: Transaction is invalid
void TStackOperator_OP_RESERVED1::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_RESERVED2
// Operation: Transaction is invalid
void TStackOperator_OP_RESERVED2::execute( TExecutionStack &Stack ) const
{
}


// -------------- Class member definitions


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include <sstream>
#include "logstream.h"
#include "unittest.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
		log() << "--- Testing parser" << endl;

		const string *p = UNITTESTSampleScripts;
		while( !p->empty() ) {
			istringstream iss(*p);
			TBitcoinScript_0 BCP;

			BCP.read(iss);

			p++;
		}
	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

