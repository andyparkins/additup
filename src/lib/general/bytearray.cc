// ----------------------------------------------------------------------------
// Project: library
/// @file   bytearray.cc
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
#include "bytearray.h"

// -------------- Includes
// --- C
// --- C++
#include <iomanip>
// --- Qt
// --- OS
// --- Project libs
// --- Project


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Class member definitions

TBufferDescription::TBufferDescription( void *p, size_t l ) :
	Pointer(p),
	Capacity(l),
	Used(0)
{
}

TBufferDescription::~TBufferDescription()
{
}


// -------------- Explicit template instantiations
template class TByteArray_t<allocator<unsigned char> >;
template class TByteArray_t<TAutoClearAllocator<unsigned char> >;


// -------------- Function definitions

ostream &dumpArray( ostream &s, const TByteArray &B )
{
	unsigned int i;
	const unsigned char *p = B.ptr();

	s << setfill('0') << hex;
	for( i = 0; i < B.size(); i++ ) {
		if( i != 0 )
			s << " ";

		s << setw(2) << (unsigned int)*p;
		p++;
	}
	s << setfill(' ') << dec;

	return s;
}


#ifdef UNITTEST

#include "logstream.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	TLog::instance().setLogLevel( TLog::Debug );

	try {
		TSecureByteArray ba;
	} catch( exception &e ) {
		log(TLog::Error) << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

