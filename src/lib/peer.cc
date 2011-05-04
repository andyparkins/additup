// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   peer.cc
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
#include "peer.h"

// -------------- Includes
// --- C
// --- C++
#include <sstream>
// --- Qt
// --- OS
// --- Project libs
// --- Project


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Template instantiations


// -------------- Class declarations

//
// Function:	TNodeInfo :: write
// Description:
//
ostream &TNodeInfo::write( ostream &os ) const
{
	os << ((IPv4 & 0xff000000) >> 24)
		<< "." << ((IPv4 & 0xff0000) >> 16)
		<< "." << ((IPv4 & 0xff00) >> 8)
		<< "." << ((IPv4 & 0xff) >> 0);

	return os;
}

//
// Function:	TNodeInfo :: get
// Description:
//
string TNodeInfo::get() const
{
	ostringstream oss;

	write(oss);

	return oss.str();
}


// -------------- Class member definitions


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include "logstream.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

