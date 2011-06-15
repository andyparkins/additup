// ----------------------------------------------------------------------------
// Project: additup
/// @file   messageelements.cc
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
#include "messageelements.h"

// -------------- Includes
// --- C
#include <string.h>
// --- C++
#include <stdexcept>
#include <limits>
#include <sstream>
// --- Qt
// --- OS
// --- Project libs
// --- Project
#include "script.h"
#include "crypto.h"
#include "logstream.h"
#include "bitcoinnetwork.h"


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Template instantiations


// -------------- Class declarations


// -------------- Class member definitions

//
// Function:	TNULTerminatedStringElement :: write
// Description:
//
ostream &TNULTerminatedStringElement::write( ostream &os ) const
{
	string::size_type pos = Value.find_first_of('\0');

	if( pos == string::npos ) {
		// String contains no NULs; so we'll send it literally, and
		// terminate ourselves
		os << Value << '\0';
	} else {
		// The string contains at least one NUL; send up to and
		// including that NUL.  Anything after that will be lost --
		// tough, shouldn't have put NULs in a NUL terminated string
		// then should you?
		os << Value.substr( 0, Value.find_first_of('\0') );
	}
	return os;
}

//
// Function:	TSizedStringElement :: write
// Description:
//
ostream &TSizedStringElement::write( ostream &os ) const
{
	if( N <= Value.size() ) {
		// Truncate
		os.write( Value.data(), N );
	} else {
		// Pad
		string::size_type n = N - Value.size();
		os << Value;
		while( n-- )
			os.put( 0 );
	}
	return os;
}

//
// Function:	TAutoSizeIntegerElement :: read
// Description:
//
// Numeric Value     Data Size Required    Format
// < 253             1 byte                < data >
// <= USHRT_MAX      3 bytes               253 + <data> (as ushort datatype)
// <= UINT_MAX       5 bytes               254 + <data> (as uint datatype)
// size > UINT_MAX   9 bytes               255 + <data>
//
istream &TAutoSizeIntegerElement::read( istream &is )
{
	unsigned char ch;

	ch = is.get();

	switch( ch ) {
		case 255: {
			TLittleEndian64Element i64;
			is >> i64;
			Value = i64.getValue();
			break;
		}
		case 254: {
			TLittleEndian32Element i32;
			is >> i32;
			Value = i32.getValue();
			break;
		}
		case 253: {
			TLittleEndian16Element i16;
			is >> i16;
			Value = i16.getValue();
			break;
		}
		default:
			// Less than 253, means use the value literally
			Value = ch;
			break;
	}

	return is;
}

//
// Function:	TAutoSizeIntegerElement :: write
// Description:
//
// Numeric Value     Data Size Required    Format
// < 253             1 byte                < data >
// <= USHRT_MAX      3 bytes               253 + <data> (as ushort datatype)
// <= UINT_MAX       5 bytes               254 + <data> (as uint datatype)
// size > UINT_MAX   9 bytes               255 + <data>
//
ostream &TAutoSizeIntegerElement::write( ostream &os ) const
{
	char buffer[9];
	unsigned int n;

	memset( buffer, 0, sizeof(buffer) );

	if( Value < 253 ) {
		n = 1;
		buffer[0] = Value;
	} else if( Value <= numeric_limits<uint16_t>::max() ) {
		n = 3;
		buffer[0] = 253;
		buffer[1] = (Value & 0xffULL);
		buffer[2] = (Value & 0xff00ULL) >> 8;
	} else if( Value <= numeric_limits<uint32_t>::max() ) {
		n = 5;
		buffer[0] = 254;
		buffer[1] = (Value & 0xffULL);
		buffer[2] = (Value & 0xff00ULL) >> 8;
		buffer[3] = (Value & 0xff0000ULL) >> 16;
		buffer[4] = (Value & 0xff000000ULL) >> 24;
	} else {
		// Value > UINT_MAX
		n = 9;
		buffer[0] = 255;
		buffer[1] = (Value & 0xffULL);
		buffer[2] = (Value & 0xff00ULL) >> 8;
		buffer[3] = (Value & 0xff0000ULL) >> 16;
		buffer[4] = (Value & 0xff000000ULL) >> 24;
		buffer[5] = (Value & 0xff00000000ULL) >> 32;
		buffer[6] = (Value & 0xff0000000000ULL) >> 40;
		buffer[7] = (Value & 0xff000000000000ULL) >> 48;
		buffer[8] = (Value & 0xff00000000000000ULL) >> 56;
	}

	os.write( buffer, n );

	return os;
}

//
// Function:	TDifficultyTargetElement :: setTarget
// Description:
//
void TDifficultyTargetElement::setTarget( const TBitcoinHash &Target )
{
	unsigned int highbit = Target.highestBit();

//	cerr << "highbit = " << highbit << " -> ";

	// The compact representation bizarrely uses a byte-based exponent,
	// which means we can't address bits, only bytes.  We round up to
	// the nearest byte.
	highbit /= 8;
	highbit += 1;
	highbit *= 8;
	// We're keeping the top three bytes, so reduce highbit by 24 bits
	highbit -= 8*3;

//	cerr << highbit << endl;

//	cerr << Target << " -> " << (Target >> highbit) << endl;

	// Extract those most significant three bytes
	Mantissa = (Target >> highbit).getBlock(0);
	// The exponent is stored as byte shifts offset by three.
	Exponent = (highbit / 8) + 3;

//	cerr << "Mantissa = " << Mantissa.getValue() << " comp " << CompMantissa << endl;
//	cerr << "Exponent = " << (unsigned int)(Exponent.getValue()) << " comp " << CompExponent << endl;
}

//
// Function:	TDifficultyTargetElement :: operator==
// Description:
//
bool TDifficultyTargetElement::operator==( const TBitcoinHash &Target ) const
{
	TDifficultyTargetElement x;

	x.setTarget( Target );

	return (*this == x);
}

//
// Function:	TSignatureElement :: read
// Description:
//
// http://forum.bitcoin.org/index.php?topic=8392.msg123728#msg123728
//
// "To encode a positive integer in DER, you convert it to a big endian
// sequence of octets, using the minimum necessary, and if the sign bit
// on the leading one is set, you prepend a zero octet so it won't be
// interpreted as a negative number.  You then prepend an "02", which
// indicates "integer", and an octet count which is a single byte for
// lengths up to 127, and 0x80 plus an octet count followed by the
// length octets for lengths greater than 127.  This maps every integer
// onto a unique octet sequence.
//
// A signature is a pair of bignum integers, (r,s), so it consists of
// 0x30, which indicates a sequence of one or more things, an octet
// count of what follows, followed by two encoded integers.  In
// addition, bitcoin appends the hashtype, which is always "1", to the
// end."
//
istream &TSignatureElement::read( istream &is )
{
	TByteElement B;

	// Type indicator (should be 0x30 == sequence)
	is >> B;
	if( B.getValue() != 0x30 )
		throw runtime_error("First byte of TSignatureElement should be 0x30");

	// Sequence count (should be two)
	// XXX: http://msdn.microsoft.com/en-us/library/bb648643%28v=vs.85%29.aspx
	// says that this is a byte count not an element count
	is >> B;
	if( B.getValue() != 2 )
		throw runtime_error("TSignatureElements should be made of two items only");

	// Type indicator (should be 0x02 == integer)
	is >> B;
	if( B.getValue() != 0x02 )
		throw runtime_error("TSignatureElements are made of two integers");

	// Size of bignum (R)
	is >> B;
	// Bignum
	TSizedStringElement R(B.getValue());
	is >> R;
	r.fromBytes( R.getValue() );

	// Size of bignum (S)
	is >> B;
	// Bignum
	TSizedStringElement S(B.getValue());
	is >> S;
	s.fromBytes( S.getValue() );

	// Hashtype is just appended by bitcoin
	is >> HashType;

	return is;
}

//
// Function:	TSignatureElement :: write
// Description:
//
ostream &TSignatureElement::write( ostream &os ) const
{
	// Type indicator (sequence)
	os.put( 0x30 );
	// Sequence count
	os.put( 0x02 );

	// Type indicator (integer)
	os.put( 0x02 );
	string R;
	R = r.toBytes();

	// If the first byte would contain a "sign bit", we need to mask it
	// by prepending another zero
	if( (R[0] & 0x80) != 0 )
		R = string('\0') + R;

	// Size of bignum
	os.put( R.size() );
	os.write( R.data(), R.size() );

	// Type indicator (integer)
	os.put( 0x02 );
	R = s.toBytes();

	// If the first byte would contain a "sign bit", we need to mask it
	// by prepending another zero
	if( (R[0] & 0x80) != 0 )
		R = string('\0') + R;

	// Size of bignum
	os.put( R.size() );
	os.write( R.data(), R.size() );

	os << HashType;

	return os;
}

//
// Function:	TInputSplitElement :: encodeSignatureScript
// Description:
//
void TInputSplitElement::encodeSignatureScript( const TBitcoinScript &Script )
{
	ostringstream oss;
	Script.write(oss);
	SignatureScript = oss.str();
}

//
// Function:	TInputSplitElement :: isCoinBase
// Description:
//
bool TInputSplitElement::isCoinBase() const
{
	return OutPoint.TransactionHash == TNetworkParameters::NULL_REFERENCE_HASH
		&& OutPoint.OutputIndex == TNetworkParameters::NULL_REFERENCE_INDEX;
}

//
// Function:	TOutputSplitElement :: encodePubKeyScript
// Description:
//
void TOutputSplitElement::encodePubKeyScript( const TBitcoinScript &lScript )
{
	ostringstream oss;
	lScript.write(oss);
	Script = oss.str();
}

// --------

TMessageDigest *TTransactionElement::Hasher = new TDoubleHash( new THash_sha256, new THash_sha256 );

//
// Function:	TTransactionElement :: getHash
// Description:
//
const TBitcoinHash &TTransactionElement::getHash() const
{
	if( !cachedHash.isZero() )
		return cachedHash;

	ostringstream oss;
	write(oss);

//	log() << __PRETTY_FUNCTION__ << " transaction bytes ";
//	TLog::hexify( log(), oss.str() );
//	log() << endl;
//
//	log() << "  for transaction {V=" << Version.getValue()
//		<< "; Ni=" << Inputs.size()
//		<< "; {";
//	for(unsigned int i = 0; i < Inputs.size(); i++ ) {
//		log() << i << ":" << Inputs[i].OutPoint.TransactionHash.get()
//			<< "." << (int)(Inputs[i].OutPoint.Index)
//			<< " ";
//	}
//	log() << "}"
//		<< "; No=" << Outputs.size()
//		<< "; {";
//	for(unsigned int i = 0; i < Outputs.size(); i++ ) {
//		log() << "value" << i << ":" << hex << Outputs[i].getValue() << dec << " ";
//	}
//	log() << "}; Locktime=" << LockTime.getValue() << "}" << endl;

	// Version             = 01 00 00 00
	// TX_In.Count         = 01
	// TX_In[0].Outhash    = a1 a0 99 98 97 96 95 94 93 92 91 90 89 88 87 86 85 84 83 82 81 80 79 78 77 76 75 74 73 72 71 70
	// TX_In[0].Outref     = 00 00 00 00
	// TX_In[0].ScriptLen  = 4d
	// TX_In[0].Script     = 04 ff ff 00 1d 01 04 45 54 68 65 20 54 69 6d 65 73 20 30 33 2f 4a 61 6e 2f 32 30 30 39 20 43 68 61 6e 63 65 6c 6c 6f 72 20 6f 6e 20 62 72 69 6e 6b 20 6f 66 20 73 65 63 6f 6e 64 20 62 61 69 6c 6f 75 74 20 66 6f 72 20 62 61 6e 6b 73
	// TX_In[0].Sequence   = ff ff ff ff
	// TX_Out.Count        = 01
	// TX_Out[0].Value     = 00 f2 05 2a 01 00 00 00
	// TX_Out[0].ScriptLen = 43
	// TX_Out[0].Script    = 41 04 67 8a fd b0 fe 55 48 27 19 67 f1 a6 71 30 b7 10 5c d6 a8 28 e0 39 09 a6 79 62 e0 ea 1f 61 de b6 49 f6 bc 3f 4c ef 38 c4 f3 55 04 e5 1e c1 12 de 5c 38 4d f7 ba 0b 8d 57 8a 4c 70 2b 6b f1 1d 5f ac
	// LockTime            = 00 00 00 00

	string digest = Hasher->transform(oss.str());

	// e.g. SHA256(SHA256()) is 32 bytes
	// 96 3f a3 b0 8b 82 37 07
	// 32 37 83 88 99 a1 3f 91
	// 94 eb 17 4e 84 c4 7e b5
	// 52 67 2f 76 6d 9d 82 36
//	log() << __PRETTY_FUNCTION__ << " digest bytes ";
//	TLog::hexify( log(), digest );
//	log() << endl;

	cachedHash.fromBytes( digest );

	// For an unknown reason, bitcoin calculates the hash, then reverses
	// the byte order, and that reversed form is then treated as the
	// hash
	cachedHash = cachedHash.reversedBytes();

	// e.g. as a number, should stay the same
	// 9f3fa3b08b823707
	// 3237838899a13f91
	// 94eb174e84c47eb5
	// 52672f766d9d8236
//	log() << __PRETTY_FUNCTION__ << " big integer ";
//	log() << cachedHash;
//	log() << endl;

	return cachedHash;
}


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include <sstream>
#include "logstream.h"

// -------------- main()

int main( int argc, char *argv[] )
{

	try {
		static const string SampleMessages[] = {
			string("\x00", 1),
			string("\xfc", 1),
			string("\xfd\xff\xff", 3),
			string("\xfe\xff\xff\xff\xff", 5),
			string("\xff\xff\xff\xff\xff\xff\xff\xff\xff", 9),
			string()
		};
		const string *p = SampleMessages;

		while( !p->empty() ) {
			istringstream iss( *p );
			TAutoSizeIntegerElement x;
			iss >> x;

			log() << p->size() << " bytes input; output = " << x.getValue() << endl;

			ostringstream oss;
			oss << x;

			if( oss.str() != *p )
				throw runtime_error( "Mismatch" );

			// Confirm we're at the "EOF"; that means that our read
			// position has been correctly maintained.
			if( iss.get() != ios::traits_type::eof() )
				throw runtime_error( "Underflow" );

			p++;
		}

	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	try {
		static const string SampleMessages[] = {
			string("abcde\0fghijklm\0", 15),
			string()
		};
		const string *p = SampleMessages;

		while( !p->empty() ) {
			istringstream iss( *p );
			TNULTerminatedStringElement x;

			while( !iss.eof() ) {
				iss >> x;
				log() << "Got string \"" << x.getValue() << "\" from "
					<< p->size() << " byte input string" << endl;
			}

			p++;
		}

		TSizedStringElement S(12);
		S = "short";
		ostringstream oss;
		S.write( oss );
		log() << "TSizedStringElement(12) = ";
		TLog::hexify( log(), oss.str() );
		log() << endl;

	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	try {
		TNElementsElement<TInventoryElement> inv;

		inv.append( TInventoryElement() );

		ostringstream oss;
		inv.write( oss );

		log() << "TNElements = ";
		TLog::hexify( log(), oss.str() );
		log() << endl;

	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	try {
		TDifficultyTargetElement dt;
		TBitcoinHash target("00000000000404CB123456789012345678901234567890123456789012345678");
		TBitcoinHash nottarget("000000000000504CB12345678901234567890123456789012345678901234567");

		dt.setTarget(0x0404cb, 0x1b);

		if( dt != target )
			throw logic_error( "Compact target representation wrong (!=)" );
		if( dt == nottarget )
			throw logic_error( "Compact target representation wrong (==)" );

	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	try {
		TMessageHeaderElement header;
		header.Magic = 0;
		header.Command = "TESTING";
		header.PayloadLength = 0;

		ostringstream oss;
		header.write( oss );

		log() << "TMessageHeaderElement = ";
		TLog::hexify( log(), oss.str() );
		log() << endl;

	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

