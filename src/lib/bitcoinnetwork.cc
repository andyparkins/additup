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
	// 14 days
	static const unsigned int DIFFICULTY_TIMESPAN = 14 * 24 * 60 * 60;
	// 10 minutes
	static const unsigned int NEW_BLOCK_PERIOD = 10 * 60;

	// Defined by whatever we support -- not sure this should be here,
	// would like to support multiple versions in the same client
	ProtocolVersion = 31800;

	// If we expect new blocks every NEW_BLOCK_PERIOD seconds, and we
	// expect the difficulty to increase every DIFFICULTY_TIMESPAN then
	// the number of blocks in DIFFICULTY_TIMESPAN is given by:
	DifficultyIncreaseSpacing = DIFFICULTY_TIMESPAN / NEW_BLOCK_PERIOD;
	// And we note the DIFFICULTY_TIMESPAN...
	TargetDifficultyIncreaseTime = DIFFICULTY_TIMESPAN;
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

