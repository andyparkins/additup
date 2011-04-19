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
// Function:	TMessageAutoSizeInteger :: read
// Description:
//
// Numeric Value     Data Size Required    Format
// < 253             1 byte                < data >
// <= USHRT_MAX      3 bytes               253 + <data> (as ushort datatype)
// <= UINT_MAX       5 bytes               254 + <data> (as uint datatype)
// size > UINT_MAX   9 bytes               255 + <data>
//
unsigned int TMessageAutoSizeInteger::read( const string &s )
{
	if( s.empty() )
		throw runtime_error("TMessageAutoSizeInteger's can't be read from empty strings" );

	switch( static_cast<uint8_t>(s[0]) ) {
		case 255:
			Value = littleEndian64FromString( s, 1 );
			return 9;
		case 254:
			Value = littleEndian32FromString( s, 1 );
			return 5;
		case 253:
			Value = littleEndian16FromString( s, 1 );
			return 3;
		default:
			// Less than 253, means use the value literally
			Value = static_cast<uint64_t>(static_cast<unsigned char>(s[0]));
			return 1;
	}
}

//
// Function:	TMessageAutoSizeInteger :: write
// Description:
//
// Numeric Value     Data Size Required    Format
// < 253             1 byte                < data >
// <= USHRT_MAX      3 bytes               253 + <data> (as ushort datatype)
// <= UINT_MAX       5 bytes               254 + <data> (as uint datatype)
// size > UINT_MAX   9 bytes               255 + <data>
//
string TMessageAutoSizeInteger::write() const
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

	return string(buffer, n);
}


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>

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
		TMessageAutoSizeInteger x;
		const string *p = SampleMessages;
		while( !p->empty() ) {

			cerr << p->size() << " input -> " << x.read( *p );
			cerr << " output = " << x.getValue() << endl;

			string y = x.write();

			if( y != *p )
				throw runtime_error( "Mismatch" );

			p++;
		}

	} catch( exception &e ) {
		cerr << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

