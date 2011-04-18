// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   crypto.cc
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
#include "crypto.h"

// -------------- Includes
// --- C
// --- C++
// --- Qt
// --- OS
// --- Project libs
// --- Project


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Template instantiations


// -------------- Class declarations




// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>

// -------------- main()

int main( int argc, char *argv[] )
{
	TEllipticCurveKey ECKEY;
	string x("1234567");
	string y;

	y = ECKEY.sign( x );
	for( unsigned int i = 0; i < y.size(); i++ ) {
		cerr << hex
			<< static_cast<unsigned int>(static_cast<unsigned char>(y[i]))
			<< " " << dec;
	}
	cerr << endl;

	if( ECKEY.verify( x, y ) ) {
		cerr << "Verifies" << endl;
	} else {
		cerr << "Does not verify" << endl;
	}

	return 0;
}
#endif

