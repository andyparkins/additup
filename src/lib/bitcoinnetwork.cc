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
#include <time.h>
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
// Static:	TNetworkParameters :: COINBASE_REFERENCE_HASH
// Description:
//
const TBitcoinHash TNetworkParameters::COINBASE_REFERENCE_HASH = 0;

//
// Static:	TNetworkParameters :: COINBASE_REFERENCE_INDEX
// Description:
//
const unsigned int TNetworkParameters::COINBASE_REFERENCE_INDEX = static_cast<unsigned int>(-1);

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

	// Default to something sensible (from official client)
	COINBASE_MATURITY = 100;

	MAX_BLOCK_SIZE = 1000000;
	MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE/2;
	MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE/50;

	MINIMUM_TRANSACTION_SIZE = 100;

	MIN_TX_FEE.setValue(0,50000);
	MIN_MONEY.setValue(0,0);
	MAX_MONEY.setValue(21000000,0);

	BLOCK_TIMESTAMP_WINDOW = 2 * 60 * 60;
}

// -----------

//
// Function:	TBitcoinNetwork :: TBitcoinNetwork
// Description:
//
TBitcoinNetwork::TBitcoinNetwork() :
	Self( NULL ),
	TransactionPool( NULL ),
	BlockPool( NULL ),
	NetworkTimeOffset( 0 )
{
}

//
// Function:	TBitcoinNetwork :: getNetworkTime
// Description:
//
time_t TBitcoinNetwork::getNetworkTime() const
{
	return time(NULL) + NetworkTimeOffset;
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

