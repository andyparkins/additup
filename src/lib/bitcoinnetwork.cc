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
TNetworkParameters::TNetworkParameters() :
	ProtocolVersion(0),
	DefaultTCPPort(0),
	Magic(0),
	BitcoinAddressPrefix(0),
	DifficultyIncreaseSpacing(0),
	TargetDifficultyIncreaseTime(0)
{
	// Zero for proof of work limit is actually the hardest possible
	// difficulty (if not impossible, as SHA256 won't produce a zero
	// hash).  Therefore we'll default the proof of work limit to the
	// easiest, which is 256 bits of ones.
	ProofOfWorkLimit = (TBitcoinHash(1) << (256)) - 1;

	// Default to something sensible
	COINBASE_MATURITY = 1;
	MAX_BLOCK_SIZE = 1 << 31;
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

