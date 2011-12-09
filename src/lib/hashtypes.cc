// ----------------------------------------------------------------------------
// Project: library
/// @file   base58.cc
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
#include "hashtypes.h"

// -------------- Includes
// --- C
#include <stdint.h>
// --- C++
#include <iostream>
// --- Qt
// --- OS
// --- Project libs
// --- Project
#include "crypto.h"


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Template instantiations


// -------------- Class member definitions

//
// Function:	TBitcoinBase58 :: fromCharacter
// Description:
//
unsigned int TBitcoinBase58::fromCharacter( unsigned int ch, unsigned int Base ) const
{
	static const uint8_t ASCIIToBase58[] = {
		// Output (0-57)                                 // Input (ASCII)
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01,  // + . . . / 0 1 2
		0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0xff,  // 3 4 5 6 7 8 9 .
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x09, 0x0a,  // . . . . . . A B
		0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0xff, 0x11,  // C D E F G H I J
		0x12, 0x13, 0x14, 0x15, 0xff, 0x16, 0x17, 0x18,  // K L M N O P Q R
		0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,  // S T U V W X Y Z
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x21, 0x22,  // . . . . . . a b
		0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,  // c d e f g h i j
		0x2b, 0xff, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31,  // k l m n o p q r
		0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39   // s t u v w x y z
	};
	static const unsigned int INVALID = static_cast<unsigned int>(-1);

	if( Base != 58 )
		return TBigUnsignedInteger::fromCharacter( ch, Base );

	if( ch >= '+' && ch-'+' <= sizeof(ASCIIToBase58) ) {
		ch = ASCIIToBase58[ch-'+'];
		if( ch == 0xff )
			ch = INVALID;
	} else {
		ch = INVALID;
	}

	return ch;
}

//
// Function:	TBitcoinBase58 :: toCharacter
// Description:
//
unsigned int TBitcoinBase58::toCharacter( unsigned int ch, unsigned int Base ) const
{
	static const char Base58ToASCII[59] =
		"123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
	static const unsigned int INVALID = static_cast<unsigned int>(-1);

	if( Base != 58 )
		return TBigUnsignedInteger::toCharacter( ch, Base );

	if( ch <= sizeof(Base58ToASCII) ) {
		ch = Base58ToASCII[ch];
	} else {
		ch = INVALID;
	}

	return ch;
}

//
// Function:	TBitcoinBase58 :: printOn
// Description:
//
ostream &TBitcoinBase58::printOn( ostream &s ) const
{
	if( !isValid() ) {
		s << "!INVALID!";
		return s;
	}

	if( s.flags() & ostream::hex ) {
		TBigUnsignedInteger::printOn(s);
	} else {
		s << toString();
	}

	return s;
}

// ------

//
// Function:	TBitcoinAddress :: TBitcoinAddress
// Description:
//
TBitcoinAddress::TBitcoinAddress( const TEllipticCurveKey &K, unsigned char v ) :
	AddressClass(v)
{
	fromKey(K);
}

//
// Function:	TBitcoinAddress :: fromKey
// Description:
// A Bitcoin address, truely, is an ECDSA public key.  However, it's
// usually presented as a hash calculated as follows:
// \code
//   AddressClassByte = 0 on prodnet; 111 on testnet
//   Keyhash = ripemd( sha256( ECDSA_Public_Key )
//   Checksum = left( sha256( sha256( AddressClassByte CONCAT Keyhash ) ), 4 )
//   Bitcoin_Address = Base58Encode( AddressClassByte CONCAT KeyHash CONCAT Checksum );
// \endcode
//
void TBitcoinAddress::fromKey( const TEllipticCurveKey &K )
{
	TByteArray CSBuffer;

	invalidate();

	// Hashers
	THash_sha256 SHA256;
	THash_ripemd160 RIPEMD160;
	TDoubleHash SHASHA256( &SHA256, &SHA256 );
	TDoubleHash RIPESHA256( &RIPEMD160, &SHA256 );

	// Core key hash
	KeyHash = RIPESHA256.transform( K.getPublicKey() );

	// Prepend the address class
	CSBuffer.resize( KeyHash.size() + 1 );
	*(CSBuffer.ptr()) = AddressClass;
	memcpy( CSBuffer.ptr(1), KeyHash, KeyHash.size() );

	// Checksum of class and keyhash
	Checksum = SHASHA256.transform( CSBuffer );

	// Append four bytes of checksum
	CSBuffer.resize( CSBuffer.size() + 4 );
	memcpy( CSBuffer.ptr(CSBuffer.size()-4), Checksum, 4 );

	// Base class converts this to bignumber
	fromBytes( CSBuffer );
}

//
// Function:	TBitcoinAddress :: isValid
// Description:
//
bool TBitcoinAddress::isValid() const
{
	// Hashers
	THash_sha256 SHA256;
	TDoubleHash SHASHA256( &SHA256, &SHA256 );

	// We assume all the constituent parts have been set (call parse()
	// if they haven't); we're now checking for consistency

	// The keyhash is supplied hashed, there is nothing to do with that,
	// instead we prepend the address class byte and calculate a
	// checksum

	TByteArray CSBuffer;

	// Prepend the address class
	CSBuffer.resize( KeyHash.size() + 1 );
	CSBuffer[0] = AddressClass;
	memcpy( CSBuffer.ptr(1), KeyHash, KeyHash.size() );

	// Checksum of class and keyhash
	CSBuffer = SHASHA256.transform( CSBuffer );

	// Compare the first four bytes of the calculated checksum against
	// the parsed checksum

	if( Checksum.size() < 4 || CSBuffer.size() < 4 ) {
		return false;
	}

	if( memcmp( Checksum.ptr(), CSBuffer.ptr(), 4 ) != 0 ) {
		return false;
	}

	return true;
}

//
// Function:	TBitcoinAddress :: fromKey
// Description:
//
void TBitcoinAddress::fromString( const string &s, unsigned int b )
{
	// The base class can handle the conversion to bytes
	TBitcoinBase58::fromString(s, b);

	parse();
}

//
// Function:	TBitcoinAddress :: parse
// Description:
//
void TBitcoinAddress::parse()
{
	// We can now extract the constituent parts
	TByteArray Buffer( toBytes(1 + 20 + 4) );

	KeyHash.clear();
	Checksum.clear();

	if( Buffer.size() < 5 )
		throw runtime_error( "Can't parse an undersized address" );

	// Enough space for a RIPE160 hash
	KeyHash.resize( Buffer.size() - 1 - 4 );
	// Enough space for the four checksum bytes
	Checksum.resize( 4 );

	AddressClass = Buffer[0];
	memcpy( KeyHash.ptr(), Buffer.ptr(1), Buffer.size() - 4 - 1 );
	memcpy( Checksum.ptr(), Buffer.ptr( Buffer.size() - 4 ), 4 );
}

//
// Function:	TBitcoinAddress :: stringPad
// Description:
//
string TBitcoinAddress::stringPad( const string &s, unsigned int Base ) const
{
	string output;

	if( Base == 58 ) {
		output = s;
		// Pad to minimum length for a bitcoin address (34)
		while( output.length() < 34 )
			output = string() + static_cast<char>(toCharacter(0,58)) + output;
	} else {
		return s;
	}

	return output;
}

// ------

//
// Static:	TBitcoinHash :: HASH_BYTES
// Description:
//
const unsigned int TBitcoinHash::HASH_BYTES = 32;

//
// Function:	TBitcoinHash :: stringPad
// Description:
//
string TBitcoinHash::stringPad( const string &s, unsigned int Base ) const
{
	if( s.size() < HASH_BYTES*2 ) {
		string pad;
		pad.assign( HASH_BYTES*2 - s.size(), toCharacter(0,Base) );
		return pad + s;
	} else if( s.size() > HASH_BYTES*2 ) {
		throw logic_error( "TBitcoinHashes can't be more than HASH_BYTES bytes" );
	} else {
		return s;
	}
}

//
// Function:	TBitcoinHash :: printOn
// Description:
//
ostream &TBitcoinHash::printOn( ostream &s ) const
{
	if( !isValid() ) {
		s << "!INVALID!";
		return s;
	}

	// Force the use of string pad by calling it even for hex (which the
	// base class doesn't do)
	string x = toString(16);
	s << x.substr(0,20) << "..." << x.substr(x.size()-4, x.size()-1);

	return s;
}

//
// Function:	TBitcoinHash :: reversedBytes
// Description:
//
TBitcoinHash TBitcoinHash::reversedBytes() const
{
	tIndex OldHighestBit = highestBit();
	TBitcoinHash X( TBigUnsignedInteger::reversedBytes() );

	// Consider these blocks  (highest bit 75)
	//   XXXX0908 07060504 03020100
	// After base-class reversal these will be
	//   00000001 02030405 06070809
	// Lets say we want a 18 byte hash, then our left shift will be
	// (18-(75/8))*8 = 9
	//   0102 03040506 07080900 00000000 00000000
	// Final size 18 bytes, flipped version of original.

	// The bytes are now reversed, but that's not enough, we have to
	// reverse to a fixed width for a hash.  For that we need to know
	// how many bytes we were short of 32 originally.
	X <<= (HASH_BYTES - 1 - (OldHighestBit/8)) * 8;

	return X;
}

// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include <stdexcept>
#include "logstream.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
		log() << "--- Testing constructors and initialisation (base58)" << endl;

		TBitcoinBase58 i("1Eym7pyJcaambv8FG4ZoU8A4xsiL9us2zz");
		TBitcoinBase58 j("1111111111111111111114oLvT2"); // 0x94a00911
		// Bitcoin addresses are 1 version byte, 20 bytes of hashed
		// ECDSA key followed by four bytes of checksum (which is itself
		// a hash of the hashed key and version bytes).  The largest
		// possible address is therefore 25 bytes of 0xff
		TBitcoinBase58 k("ffffffffffffffffffffffffffffffffffffffffffffffffff", 16);
		// The above gives a 34 character base58 string.  What's the
		// biggest 34 digit base58 number though?
		TBitcoinBase58 l("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz");

		log() << "i = " << i << "; 0x" << hex << i << dec << endl;
//		if( i.getBlock(2) != 0x5 || i.getBlock(1) != 0x6bc75e2d || i.getBlock(0) != 0x630fffff )
//			throw logic_error("Assignment from decimal-representing string incorrect");
		log() << "j = " << j << "; 0x" << hex << j << dec << endl;
		log() << "k = " << k << "; 0x" << hex << k << dec << endl;
		k++;
		log() << "k = " << k << "; 0x" << hex << k << dec << endl;
		log() << "l = " << l << "; 0x" << hex << l << dec << endl;
	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	try {
		log() << "--- Testing constructors and initialisation (hashes)" << endl;

		TBitcoinHash i("0"); // 0x94a00911
		TBitcoinHash j("ffffffffffffffffffffffffffffffffffffffffffffffffff");

		log() << "i = " << i << endl;
		log() << "j = " << j << endl;
	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	try {
		log() << "--- Testing key to address conversion" << endl;

		// https://en.bitcoin.it/wiki/Technical_background_of_Bitcoin_addresses
		TByteArray PubKey("\x04"
				"\x67\x8a\xfd\xb0\xfe\x55\x48\x27\x19\x67\xf1\xa6\x71\x30\xb7\x10"
				"\x5c\xd6\xa8\x28\xe0\x39\x09\xa6\x79\x62\xe0\xea\x1f\x61\xde\xb6"
				"\x49\xf6\xbc\x3f\x4c\xef\x38\xc4\xf3\x55\x04\xe5\x1e\xc1\x12\xde"
				"\x5c\x38\x4d\xf7\xba\x0b\x8d\x57\x8a\x4c\x70\x2b\x6b\xf1\x1d\x5f", 65 );
		TEllipticCurveKey ECKEY;
		ECKEY.setPublicKey(PubKey);

		TBitcoinAddress BA( ECKEY );

		log() << "AC: address = " << BA
			<< "; " << hex << BA << dec << endl;

		if( BA.toString() != "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa" )
			throw runtime_error("Public key conversion failed");

		if( !BA.isValid() )
			throw runtime_error("Expected address to be valid");
		if( BA.getClass() != 0 )
			throw runtime_error("Expected address to be class 0 (prodnet)");

		log() << "AC: Copying address by string" << endl;

		TBitcoinAddress BA2;
		BA2.fromString( BA.toString() );

		if( BA2.toString() != "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa" )
			throw runtime_error("Public key copy by string failed");

	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

