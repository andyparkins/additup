// ----------------------------------------------------------------------------
// Project: bitcoin
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
// --- Qt
// --- OS
// --- Project libs
// --- Project


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
	if( N >= Value.size() ) {
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

	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

