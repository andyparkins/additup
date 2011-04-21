// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   messageelements.h
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
#ifndef MESSAGEELEMENTS_H
#define MESSAGEELEMENTS_H

// -------------- Includes
// --- C
#include <stdint.h>
// --- C++
#include <string>
#include <iostream>
#include <vector>
// --- Qt
// --- OS
// --- Project
// --- Project lib


// -------------- Namespace
	// --- Imported namespaces
	using namespace std;


// -------------- Defines
// General
// Project


// -------------- Constants


// -------------- Typedefs (pre-structure)


// -------------- Enumerations

enum eWords {
	// Flow control
	OP_NOP = 0,
	OP_IF,
	OP_NOTIF,
	OP_ELSE,
	OP_ENDIF,
	OP_VERIFY,
	OP_RETURN,
	// Stack
	OP_TOALTSTACK,
	OP_FROMALTSTACK,
	OP_IFDUP,
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
	OP_2DROP,
	OP_2DUP,
	OP_3DUP,
	OP_2OVER,
	OP_2ROT,
	OP_2SWAP,
	// Splice
	OP_CAT,
	OP_SUBSTR,
	OP_LEFT,
	OP_RIGHT,
	OP_SIZE,
	// Bitwise logic
	OP_INVERT,
	OP_AND,
	OP_OR,
	OP_XOR,
	OP_EQUAL,
	OP_EQUALVERIFY,
	// Arithmetic
	OP_1ADD,
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
	OP_RIPEMD160,
	OP_SHA1,
	OP_SHA256,
	OP_HASH160,
	OP_HASH256,
	OP_CODESEPARATOR,
	OP_CHECKSIG,
	OP_CHECKSIGVERIFY,
	OP_CHECKMULTISIG,
	OP_CHECKMULTISIGVERIFY,
	WORD_COUNT
};


// -------------- Structures/Unions


// -------------- Typedefs (post-structure)


// -------------- Class pre-declarations
class TMessageElement;


// -------------- Function pre-class prototypes
istream &operator>>(istream &s, TMessageElement &E );
ostream &operator<<(ostream &s, const TMessageElement &E );


// -------------- Class declarations

//
// Class:	TMessageElement
// Description:
//
class TMessageElement
{
  public:
	virtual istream &read( istream &is ) { return is; }
	virtual ostream &write( ostream &os ) const { return os; }

	static uint32_t littleEndian16FromString( const string &d, string::size_type p = 0) {
		return static_cast<uint8_t>(d[p]) << 0
		| static_cast<uint8_t>(d[p+1]) << 8; }
	static uint32_t littleEndian32FromString( const string &d, string::size_type p = 0) {
		return static_cast<uint8_t>(d[p]) << 0
		| static_cast<uint8_t>(d[p+1]) << 8
		| static_cast<uint8_t>(d[p+2]) << 16
		| static_cast<uint8_t>(d[p+3]) << 24; }
	static uint64_t littleEndian64FromString( const string &d, string::size_type p = 0) {
		return static_cast<uint64_t>(littleEndian32FromString(d,p)) << 0
		| static_cast<uint64_t>(littleEndian32FromString(d,p+4)) << 32; }
	static string::size_type NULTerminatedString( string &d, const string &s, string::size_type p = 0) {
		d = s.substr(p, s.find_first_of('\0', p) - p );
		return d.size();
	}
};

//
// Class:	TStringBasedElement
// Description:
//
class TStringBasedElement : public TMessageElement
{
  public:
	const string &getValue() const { return Value; }
	TStringBasedElement &operator=( const string &s ) { Value = s; return *this; }
	void clear() { Value.clear(); }

  protected:
	string Value;
};

//
// Class:	TNULTerminatedStringElement
// Description:
//
class TNULTerminatedStringElement : public TStringBasedElement
{
  public:
	istream &read( istream &is ) {
		char ch;
		Value.clear();
		while( !is.eof() && (ch = is.get()) != '\0' && ch != ios::traits_type::eof() ) {
			Value += ch;
		}
		return is;
	}

	using TStringBasedElement::operator=;
};

//
// Class:	TSizedStringElement
// Description:
//
class TSizedStringElement : public TStringBasedElement
{
  public:
	TSizedStringElement(string::size_type n) : N(n) {}

	istream &read( istream &is ) {
		char buffer[N];
		is.read( buffer, N );
		Value.assign( buffer, N );
		return is;
	}

	using TStringBasedElement::operator=;

  protected:
	string::size_type N;
};

//
// Class:	TFixedStringElement
// Description:
//
template<string::size_type NUM>
class TFixedStringElement : public TSizedStringElement
{
  public:
	TFixedStringElement() : TSizedStringElement(NUM) {}

	using TSizedStringElement::operator=;
};

//
// Class:	TByteElement
// Description:
//
class TByteElement : public TMessageElement
{
  public:
	istream &read( istream &is ) {
		Value = is.get();
		return is;
	}

	operator uint8_t() const { return Value; }
	uint8_t getValue() const { return Value; }
	TByteElement &operator=( uint8_t s ) { Value = s; return *this; }

  protected:
	uint8_t Value;
};

//
// Class:	TBigEndian16Element
// Description:
//
class TBigEndian16Element : public TMessageElement
{
  public:
	istream &read( istream &is ) {
		Value = static_cast<uint8_t>(is.get()) << 8
			| static_cast<uint8_t>(is.get()) << 0;
		return is;
	}

	operator uint16_t() const { return Value; }
	uint16_t getValue() const { return Value; }
	TBigEndian16Element &operator=( uint16_t s ) { Value = s; return *this; }

  protected:
	uint16_t Value;
};

//
// Class:	TLittleEndian16Element
// Description:
//
class TLittleEndian16Element : public TMessageElement
{
  public:
	istream &read( istream &is ) {
		Value = static_cast<uint8_t>(is.get()) << 0
			| static_cast<uint8_t>(is.get()) << 8;
		return is;
	}

	operator uint16_t() const { return Value; }
	uint16_t getValue() const { return Value; }
	TLittleEndian16Element &operator=( uint16_t s ) { Value = s; return *this; }

  protected:
	uint16_t Value;
};

//
// Class:	TLittleEndian32Element
// Description:
//
class TLittleEndian32Element : public TMessageElement
{
  public:
	istream &read( istream &is ) {
		Value = static_cast<uint8_t>(is.get()) << 0
			| static_cast<uint8_t>(is.get()) << 8
			| static_cast<uint8_t>(is.get()) << 16
			| static_cast<uint8_t>(is.get()) << 24;
		return is;
	}

	operator uint32_t() const { return Value; }
	uint32_t getValue() const { return Value; }
	TLittleEndian32Element &operator=( uint32_t s ) { Value = s; return *this; }

  protected:
	uint32_t Value;
};

//
// Class:	TLittleEndian64Element
// Description:
//
class TLittleEndian64Element : public TMessageElement
{
  public:
	istream &read( istream &is ) {
		TLittleEndian32Element a, b;
		is >> a >> b;
		Value = static_cast<uint64_t>(a) << 0
			| static_cast<uint64_t>(b) << 32;
		return is;
	}

	operator uint64_t() const { return Value; }
	uint64_t getValue() const { return Value; }
	TLittleEndian64Element &operator=( uint64_t s ) { Value = s; return *this; }

  protected:
	uint64_t Value;
};

//
// Class:	TAutoSizeIntegerElement
// Description:
//
class TAutoSizeIntegerElement : public TMessageElement
{
  public:
	istream &read( istream & );
	ostream &write( ostream & ) const;

	uint64_t getValue() const { return Value; }
	TAutoSizeIntegerElement &operator=( uint64_t s ) { Value = s; return *this; }

	operator uint64_t() const { return Value; }

  protected:
	uint64_t Value;
};

//
// Class:	TVariableSizedStringElement
// Description:
//
class TVariableSizedStringElement : public TSizedStringElement
{
  public:
	TVariableSizedStringElement() : TSizedStringElement(0) {}

	istream &read( istream &is ) {
		TAutoSizeIntegerElement VarInt;
		is >> VarInt;
		N = VarInt.getValue();
		return TSizedStringElement::read(is);
	}
};

//
// Typedef:    THashElement
// Description:
//
typedef TFixedStringElement<32> THashElement;

//
// Typedef:	TTimestampElement
// Description:
//
typedef TLittleEndian32Element TTimestampElement;

//
// Class:	TMessageHeaderElement
// Description:
//
/// All Network communications packets have the following initial bytes
/// that form a 'header' to identify a valid packet.
///
/// Data is appended after the Checksum depending upon the data type,
/// with verification using the packet size for the validity of the
/// packet as well.
///
/// (from net.h)
//
class TMessageHeaderElement : public TMessageElement
{
  public:
	istream &read( istream &is ) {
		is >> Magic >> Command >> PayloadLength;
		return is;
	}

  public:
	TLittleEndian32Element Magic;
	TFixedStringElement<12> Command;
	TLittleEndian32Element PayloadLength;
	TLittleEndian32Element Checksum;
};

//
// Class:	TAddressDataElement
// Description:
/// Address data is used in several places in the packets.
//
/// (from CAddress in net.h)
//
class TAddressDataElement : public TMessageElement
{
  public:
	enum eServices {
		// This node can be asked for full blocks instead of just headers.
		NODE_NETWORK = 1
	};

  public:
	istream &read( istream &is ) {
		is >> Services >> Address >> PortNumber;
		return is;
	}

  public:
	TLittleEndian64Element Services;
	TFixedStringElement<16> Address;
	TBigEndian16Element PortNumber;
};

//
// Class:	TTimedAddressDataElement
// Description:
//
class TTimedAddressDataElement : public TAddressDataElement
{
  public:
	istream &read( istream &is ) {
		is >> Time;
		return TAddressDataElement::read(is);
	}

  public:
	TTimestampElement Time;
};

//
// Class:	TNElementsElement
// Description:
//
template <typename Element>
class TNElementsElement : public TMessageElement
{
  public:
	istream &read( istream &is ) {
		TAutoSizeIntegerElement N;
		is >> N;
		for( unsigned int i = 0; i < N; i++ ) {
			Array.push_back( Element() );
			is >> Array.back();
		}
		return is;
	}

	unsigned int size() const { return Array.size(); }
	Element &operator[]( unsigned int i ) { return Array[i]; }
	const Element &operator[]( unsigned int i ) const { return Array[i]; }

  protected:
	vector<Element> Array;
};

////
//// Struct:	sInventoryVector
//// Description:
//// Inventory vectors are used for notifying other nodes about data they
//// may have, and data which is being requested.
////
//// (from net.h)
////
//struct sInventoryVector
//{
//	enum eObjectType {
//		ERROR = 0,
//		MSG_TX,
//		MSG_BLOCK,
//		// Other Data Type values are considered reserved for future
//		// implementations.
//		OBJECT_TYPE_COUNT
//	};
//
//	eObjectType ObjectType;
//	sHash Hash;
//};

// -------------- Constants


// -------------- Inline Functions
inline istream &operator>>(istream &s, TMessageElement &E ) { return E.read(s); }
inline ostream &operator<<(ostream &s, const TMessageElement &E ) { return E.write(s); }


// -------------- Function prototypes


// -------------- Template instantiations



// -------------- World globals ("extern"s only)

// End of conditional compilation
#endif
