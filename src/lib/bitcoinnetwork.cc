// ----------------------------------------------------------------------------
// Project: additup
/// @file   bitcoinnetwork.cc
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
#include "bitcoinnetwork.h"

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


// -------------- Class member definitions

//
// Function:	TNetworkParameters
// Description:
//
TNetworkParameters::TNetworkParameters()
{
}


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include "logstream.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
	} catch( std::exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

