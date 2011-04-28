// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   logstream.cc
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
#include "logstream.h"

// -------------- Includes
// --- C
// --- C++
#include <iostream>
#include <iomanip>
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
// Static:	TLog :: Singleton
// Description:
//
TLog *TLog::Singleton = NULL;

//
// Function:	TLog :: TLog
// Description:
//
TLog::TLog()
{
}

//
// Function:	TLog :: ~TLog
// Description:
//
TLog::~TLog()
{
}

//
// Function:	TLog :: instance
// Description:
//
TLog &TLog::instance()
{
	if( Singleton == NULL )
		Singleton = new TLog;

	return *Singleton;
}

//
// Function:	TLog :: hexify
// Description:
//
ostream &TLog::hexify(ostream &s, const string &Source)
{
	unsigned int i;
	const unsigned char *p = (unsigned char *) Source.data();

	s << setfill('0') << hex;
	for( i = 0; i < Source.size(); i++ ) {
		if( i != 0 )
			s << " ";

		s << setw(2) << (unsigned int)*p;
		p++;
	}
	s << setfill(' ') << dec;

	return s;
}


// -------------- Function definitions

//
// Function:	log
// Description:
//
ostream &log()
{
	return TLog::instance().getStream();
}


#ifdef UNITTEST

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
		log() << "Testing" << endl;

		TLog::hexify( log(), string("\x01\x02\x03\x04\x05",5) );
		log() << endl;

	} catch( exception &e ) {
		cerr << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

