// ----------------------------------------------------------------------------
// Project: additup
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


// -------------- Structures/Unions


// -------------- Typedefs (post-structure)


// -------------- Class pre-declarations
class TMessageElement;
class TMessageDigest;
class TBitcoinScript;


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
	virtual istream &read( istream &is ) = 0;
	virtual ostream &write( ostream &os ) const = 0;

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
	ostream &write( ostream &os ) const;

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
	ostream &write( ostream &os ) const;

	using TStringBasedElement::operator=;

  protected:
	mutable string::size_type N;
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
// Class:	TIntegerElement_t
// Description:
//
template <typename integer_t>
class TIntegerElement_t : public TMessageElement
{
  public:
	operator integer_t() const { return Value; }
	integer_t getValue() const { return Value; }
	TIntegerElement_t &operator=( integer_t s ) { Value = s; return *this; }

	integer_t operator++(int) { return Value++; }
	integer_t operator--(int) { return Value--; }

  protected:
	integer_t Value;
};

//
// Class:	TByteElement
// Description:
//
class TByteElement : public TIntegerElement_t<uint8_t>
{
  public:
	istream &read( istream &is ) {
		Value = is.get();
		return is;
	}
	ostream &write( ostream &os ) const { return os.put( Value ); }

	using TIntegerElement_t<uint8_t>::operator=;
};

//
// Class:	TBigEndian16Element
// Description:
//
class TBigEndian16Element : public TIntegerElement_t<uint16_t>
{
  public:
	istream &read( istream &is ) {
		Value = static_cast<uint8_t>(is.get()) << 8
			| static_cast<uint8_t>(is.get()) << 0;
		return is;
	}
	ostream &write( ostream &os ) const {
		os.put( (Value & 0xff00) >> 8 );
		return os.put( (Value & 0xff) >> 0 );
	}

	using TIntegerElement_t<uint16_t>::operator=;
};

//
// Class:	TLittleEndian16Element
// Description:
//
class TLittleEndian16Element : public TIntegerElement_t<uint16_t>
{
  public:
	istream &read( istream &is ) {
		Value = static_cast<uint8_t>(is.get()) << 0
			| static_cast<uint8_t>(is.get()) << 8;
		return is;
	}
	ostream &write( ostream &os ) const {
		os.put( (Value & 0xff) >> 0 );
		return os.put( (Value & 0xff00) >> 8 );
	}

	using TIntegerElement_t<uint16_t>::operator=;
};

//
// Class:	TLittleEndian24Element
// Description:
//
class TLittleEndian24Element : public TIntegerElement_t<uint32_t>
{
  public:
	istream &read( istream &is ) {
		Value = static_cast<uint8_t>(is.get()) << 0
			| static_cast<uint8_t>(is.get()) << 8
			| static_cast<uint8_t>(is.get()) << 16;
		return is;
	}
	ostream &write( ostream &os ) const {
		os.put( (Value & 0xff) >> 0 );
		os.put( (Value & 0xff00) >> 8 );
		return os.put( (Value & 0xff0000) >> 16 );
	}

	using TIntegerElement_t<uint32_t>::operator=;
};

//
// Class:	TLittleEndian32Element
// Description:
//
class TLittleEndian32Element : public TIntegerElement_t<uint32_t>
{
  public:
	istream &read( istream &is ) {
		Value = static_cast<uint8_t>(is.get()) << 0
			| static_cast<uint8_t>(is.get()) << 8
			| static_cast<uint8_t>(is.get()) << 16
			| static_cast<uint8_t>(is.get()) << 24;
		return is;
	}
	ostream &write( ostream &os ) const {
		os.put( (Value & 0xff) >> 0 );
		os.put( (Value & 0xff00) >> 8 );
		os.put( (Value & 0xff0000) >> 16 );
		return os.put( (Value & 0xff000000) >> 24 );
	}

	using TIntegerElement_t<uint32_t>::operator=;
};

//
// Class:	TLittleEndian64Element
// Description:
//
class TLittleEndian64Element : public TIntegerElement_t<uint64_t>
{
  public:
	istream &read( istream &is ) {
		TLittleEndian32Element a, b;
		is >> a >> b;
		Value = static_cast<uint64_t>(a) << 0
			| static_cast<uint64_t>(b) << 32;
		return is;
	}
	ostream &write( ostream &os ) const {
		os.put( (Value & 0xffULL) >> 0 );
		os.put( (Value & 0xff00ULL) >> 8 );
		os.put( (Value & 0xff0000ULL) >> 16 );
		os.put( (Value & 0xff000000ULL) >> 24 );
		os.put( (Value & 0xff00000000ULL) >> 32 );
		os.put( (Value & 0xff0000000000ULL) >> 40 );
		os.put( (Value & 0xff000000000000ULL) >> 48 );
		return os.put( (Value & 0xff00000000000000ULL) >> 56 );
	}

	using TIntegerElement_t<uint64_t>::operator=;
};

//
// Class:	TAutoSizeIntegerElement
// Description:
//
class TAutoSizeIntegerElement : public TIntegerElement_t<uint64_t>
{
  public:
	istream &read( istream & );
	ostream &write( ostream & ) const;

	using TIntegerElement_t<uint64_t>::operator=;
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
	ostream &write( ostream &os ) const {
		TAutoSizeIntegerElement VarInt;
		// Ensure the base class writes the whole string
		N = Value.size();
		VarInt = N;
		// Write the size field
		os << VarInt;
		// Write the string
		return TSizedStringElement::write(os);
	}

	using TSizedStringElement::operator=;
};

//
// Class:	THashElement
// Description:
//
class THashElement : public TMessageElement
{
  public:
	THashElement() { Hash.invalidate(); }

	istream &read( istream &is ) {
		char buffer[32];
		is.read( buffer, sizeof(buffer) );
		string s( buffer, sizeof(buffer) );
		Hash.fromBytes(s);
		Hash = Hash.reversedBytes();
		return is;
	}
	ostream &write( ostream &os ) const {
		// Hashes are written little endian
		os.write( Hash.reversedBytes().toBytes(32).data(), 32 );
		return os;
	}

	operator TBitcoinHash() const { return Hash; }
	operator const TBitcoinHash&() const { return Hash; }
	TBitcoinHash &get() { return Hash; }
	const TBitcoinHash &get() const { return Hash; }
	THashElement &operator=( const TBitcoinHash &s ) { Hash = s; return *this; }
	bool operator==( const TBitcoinHash &s ) const { return Hash == s; }
	bool operator!=( const TBitcoinHash &s ) const { return Hash != s; }

  protected:
	TBitcoinHash Hash;
};

//
// Class:	TSignatureElement
// Description:
//  <30><len><02><len><r bytes><02><len><s bytes><hashtype>
//
class TSignatureElement : public TMessageElement
{
  public:
	TSignatureElement();

	istream &read( istream &is );
	ostream &write( ostream &os ) const;

  protected:
	TBigUnsignedInteger r;
	TBigUnsignedInteger s;
	TByteElement HashType;
};

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
	ostream &write( ostream &os ) const {
		os << Magic << Command << PayloadLength;
		return os;
	}

  public:
	TLittleEndian32Element Magic;
	TFixedStringElement<12> Command;
	TLittleEndian32Element PayloadLength;
	bool hasChecksum;
	TLittleEndian32Element Checksum;
};

//
// Class:	TNetworkAddressElement
// Description:
/// Address data is used in several places in the packets.
//
/// (from CAddress in net.h)
//
class TNetworkAddressElement : public TMessageElement
{
  public:
	enum eServices {
		// This node can be asked for full blocks instead of just headers.
		NODE_NETWORK = 1
	};

  public:
	TNetworkAddressElement() {
		Services = 0;
		PortNumber = 0;
	}

	istream &read( istream &is ) {
		is >> Services >> Address >> PortNumber;
		return is;
	}
	ostream &write( ostream &os ) const {
		os << Services << Address << PortNumber;
		return os;
	}

	void clear() { Services = 0; Address = ""; PortNumber = 0; }

  public:
	TLittleEndian64Element Services;
	TFixedStringElement<16> Address;
	TBigEndian16Element PortNumber;
};

//
// Class:	TTimedNetworkAddressElement
// Description:
//
class TTimedNetworkAddressElement : public TNetworkAddressElement
{
  public:
	istream &read( istream &is ) {
		is >> Time;
		return TNetworkAddressElement::read(is);
	}
	ostream &write( ostream &os ) const {
		os << Time;
		return TNetworkAddressElement::write(os);
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
	ostream &write( ostream &os ) const {
		typename vector<Element>::const_iterator it;
		TAutoSizeIntegerElement N;
		N = Array.size();
		os << N;
		for( it = Array.begin(); it != Array.end(); it++ ) {
			os << *it;
		}
		return os;
	}

	void append( const Element &e ) { Array.push_back( e ); }
	Element &back() { return Array.back(); }

	unsigned int size() const { return Array.size(); }
	Element &operator[]( unsigned int i ) { return Array[i]; }
	const Element &operator[]( unsigned int i ) const { return Array[i]; }

  protected:
	vector<Element> Array;
};

//
// Class:	TDifficultyTargetElement
// Description:
//
class TDifficultyTargetElement : public TMessageElement
{
  public:
	istream &read( istream &is ) {
		is >> Mantissa >> Exponent;
		return is;
	}
	ostream &write( ostream &os ) const {
		os << Mantissa << Exponent;
		return os;
	}

	TBitcoinHash getTarget() const {
		TBitcoinHash t( Mantissa.getValue() );
		t <<= (Exponent - 3) * 8;
		return t;
	}
	void setTarget( uint32_t m, uint8_t e ) { Mantissa = m; Exponent = e; }
	void setTarget( const TBitcoinHash & );

	bool operator==( const TDifficultyTargetElement &dt ) const {
		return dt.Mantissa.getValue() == Mantissa.getValue()
			&& dt.Exponent.getValue() == Exponent.getValue();
	}
	bool operator==( const TBitcoinHash & ) const;
	bool operator!=( const TBitcoinHash &h ) const { return !operator==(h); }

  public:
	TLittleEndian24Element Mantissa;
	TByteElement Exponent;
};

//
// Class:	TBlockHeaderElement
// Description:
//
class TBlockHeaderElement : public TMessageElement
{
  public:
	istream &read( istream &is ) {
		is >> Version
			>> PreviousBlock >> MerkleRoot >> Timestamp
			>> DifficultyBits >> Nonce;
		return is;
	}
	ostream &write( ostream &os ) const {
		os << Version << PreviousBlock << MerkleRoot << Timestamp
			<< DifficultyBits << Nonce;
		return os;
	}

  public:
	TLittleEndian32Element Version;
	THashElement PreviousBlock;
	THashElement MerkleRoot;
	TTimestampElement Timestamp;
	TDifficultyTargetElement DifficultyBits;
	TLittleEndian32Element Nonce;
};

//
// Class:	TPaddedBlockHeaderElement
// Description:
//
class TPaddedBlockHeaderElement : public TBlockHeaderElement
{
  public:
	istream &read( istream &is ) {
		TBlockHeaderElement::read(is);
		TAutoSizeIntegerElement ignore;
		is >> ignore;
		return is;
	}
	ostream &write( ostream &os ) const {
		TBlockHeaderElement::write(os);
		return os.put(0);
	}
};

//
// Class:	TWalletTxElement
// Description:
//
class TWalletTxElement : public TMessageElement
{
  public:
	istream &read( istream &is ) {
		return is;
	}
	ostream &write( ostream &os ) const {
		return os;
	}

  public:
	// Not enough information in protocol to implement
};

//
// Class:	TInventoryElement
// Description:
// Inventory vectors are used for notifying other nodes about data they
// may have, and data which is being requested.
//
// (from net.h)
//
class TInventoryElement : public TMessageElement
{
  public:
	enum eObjectType {
		ERROR = 0,
		MSG_TX,
		MSG_BLOCK,
		// Other Data Type values are considered reserved for future
		// implementations.
		OBJECT_TYPE_COUNT
	};

	istream &read( istream &is ) {
		is >> ObjectType >> Hash;
		return is;
	}
	ostream &write( ostream &os ) const {
		os << ObjectType << Hash;
		return os;
	}

  public:
	TLittleEndian32Element ObjectType;
	THashElement Hash;
};

//
// Class:	TCoinsElement
// Description:
//
//    5000000  =  0.05 Coins
// 3354000000  = 33.54 Coins
//  100000000x =  x    Coins
//
// Annoyingly the java bitcoin library refers to these as NanoCoins --
// they are manifestly not.  The bitcoin community calls them,
// colloquially, Satoshis.
//
class TCoinsElement : public TLittleEndian64Element
{
  public:
	void setValue( uint64_t Coins, unsigned int Cents = 0 ) {
		Value = Coins * COIN + Cents * CENT;
	}
	double getValue() const {
		return (Value / 1.0 / COIN);
	}

	static const uint64_t COIN = 100000000LL;
	static const uint64_t CENT = 1000000LL;
};

//
// Class:	TOutputTransactionReferenceElement
// Description:
//
class TOutputTransactionReferenceElement : public TMessageElement
{
  public:
	istream &read( istream &is ) {
		is >> TransactionHash >> OutputIndex;
		return is;
	}
	ostream &write( ostream &os ) const {
		os << TransactionHash << OutputIndex;
		return os;
	}

  public:
	THashElement TransactionHash;
	TLittleEndian32Element OutputIndex;
};

//
// Class:	TInputSplitElement
// Description:
//
class TInputSplitElement : public TMessageElement
{
  public:
	TInputSplitElement() { Sequence = 0xffffffff; }

	istream &read( istream &is ) {
		is >> OutPoint >> SignatureScript >> Sequence;
		return is;
	}
	ostream &write( ostream &os ) const {
		os << OutPoint << SignatureScript << Sequence;
		return os;
	}

	bool isCoinBase() const;

	void encodeSignatureScript( const TBitcoinScript & );

  public:
	TOutputTransactionReferenceElement OutPoint;
	TVariableSizedStringElement SignatureScript;
	TLittleEndian32Element Sequence;
};

//
// Class:	TOutputSplitElement
// Description:
//
class TOutputSplitElement : public TMessageElement
{
  public:
	istream &read( istream &is ) {
		is >> Coins >> Script;
		return is;
	}
	ostream &write( ostream &os ) const {
		os << Coins << Script;
		return os;
	}

	void setValue( uint64_t a, unsigned int b = 0 ) { Coins.setValue(a,b); }
	double getValue() const { return Coins.getValue(); }

	void encodePubKeyScript( const TBitcoinScript & );

  protected:
	TCoinsElement Coins;
	TVariableSizedStringElement Script;
};

//
// Class:	TTransactionElement
// Description:
//
class TTransactionElement : public TMessageElement
{
  public:
	TTransactionElement() { Version = 1; LockTime = 0; cachedHash = 0; }

	istream &read( istream &is ) {
		is >> Version
			>> Inputs
			>> Outputs
			>> LockTime;
		return is;
	}
	ostream &write( ostream &os ) const {
		os << Version
			<< Inputs
			<< Outputs
			<< LockTime;
		return os;
	}

	TInputSplitElement &createInput() { cachedHash = 0; Inputs.append(TInputSplitElement()); return Inputs.back(); }
	TOutputSplitElement &createOutput() { cachedHash = 0; Outputs.append(TOutputSplitElement()); return Outputs.back(); }

	const TBitcoinHash &getHash() const;

  public:
	TLittleEndian32Element Version;
	TNElementsElement<TInputSplitElement> Inputs;
	TNElementsElement<TOutputSplitElement> Outputs;
	TLittleEndian32Element LockTime;

	mutable TBitcoinHash cachedHash;

  protected:
	static TMessageDigest *Hasher;
};


// -------------- Constants


// -------------- Inline Functions
inline istream &operator>>(istream &s, TMessageElement &E ) { return E.read(s); }
inline ostream &operator<<(ostream &s, const TMessageElement &E ) { return E.write(s); }


// -------------- Function prototypes


// -------------- Template instantiations



// -------------- World globals ("extern"s only)

// End of conditional compilation
#endif
