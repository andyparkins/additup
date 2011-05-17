// ----------------------------------------------------------------------------
// Project: library
/// @file   singleton.cc
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
#include "singleton.h"

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


// -------------- Function definitions


#ifdef UNITTEST
#include <stdexcept>
#include "logstream.h"

class Object
{
  public:
	Object() : x(0) {}

	unsigned int x;
};

typedef TSingleton<Object> TSingleObject;

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
		log() << "Singleton exists       : "
			<< (TSingleObject::exists() ? "true" : "false")
			<< endl;
		if( TSingleObject::exists() )
			throw logic_error( "Singleton shouldn't exist until used" );

		log() << "Accessing instance; x  : "
			<< TSingleObject::instance()->x << endl;

		log() << "Singleton exists       : "
			<< (TSingleObject::exists() ? "true" : "false")
			<< endl;
		if( !TSingleObject::exists() )
			throw logic_error( "Singleton should exist after used" );

		log() << "Changing singleton" << endl;
		TSingleObject::instance()->x = 10;

		log() << "Accessing instance; x  : "
			<< TSingleObject::instance()->x << endl;
		if( TSingleObject::instance()->x != 10 )
			throw logic_error( "Instance value didn't change" );

	} catch( exception &e ) {
		cerr << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

