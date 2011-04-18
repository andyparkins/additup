// ----------------------------------------------------------------------------
// Project: library
/// @file   autoversion.cc
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
#include "autoversion.h"

// -------------- Includes
// --- C
// --- C++
// --- Qt
// --- OS
// --- Project libs
// --- Project


// -------------- Namespace
using namespace std;


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)
// This module has one purpose - to be rebuilt at every build
// with the latest VCSID from the Makefile
#ifdef VCSID
const char *VCSID_CONST = VCSID;
#else
const char *VCSID_CONST = "VCSID-unavailable";
#endif


// -------------- Namespace


// -------------- Class member definitions




// -------------- Function definitions


#ifdef UNITTEST

#include <iostream>
#include <cstring>

// -------------- main()

//
// Function:	main
// Description:
/// Unit test to try the URL parser with a few different URLs
//
int main( int argc, char *argv[] )
{
	cerr << "Version: " << VCSID_CONST << endl;
	if( strcmp( VCSID_CONST, "VCSID-unavailable" ) == 0 )
		return 1;
	return 0;
}

#endif


// End of namespace
//}
