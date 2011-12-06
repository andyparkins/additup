// ----------------------------------------------------------------------------
// Project: additup
/// @file   main.cc
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
#include "main.h"

// -------------- Includes
// --- C
#include <stdlib.h>
// --- C++
#include <stdexcept>
#include <typeinfo>
// --- OS
// --- Project libs
#include <logstream.h>
// --- Project


// -------------- Module functions


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Class member definitions


// ----- Main

int main( int argc, char *argv[] )
{
	try {

	} catch( exception &e ) {
		log(TLog::Error) << typeid(e).name() << " " << e.what() << endl;
		// Core dump, in case it's useful for analysis
		abort();
	}

	return 0;
}

