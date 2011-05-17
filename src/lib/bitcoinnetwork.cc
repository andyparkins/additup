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
	BitcoinAddressPrefix(0)
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
	DIFFICULTY_TIMESPAN = 14 * 24 * 60 * 60;
	NEW_BLOCK_PERIOD = 10 * 60;
//	DifficultyIncreaseSpacing = DIFFICULTY_TIMESPAN / NEW_BLOCK_PERIOD
}

//
// Function:	TNetworkParameters :: convertTargetToDifficulty
// Description:
// Calculate ProofOfWorkLimit / Target as a floating point number.
//
// The official client cheats a fair bit and assumes the two numbers are
// within one uint of each other.  It would be nicer if we could do a
// proper job, but as the official client does this, so will I.
//
double TNetworkParameters::convertTargetToDifficulty( const TBitcoinHash &Target ) const
{
	TBitcoinHash q(Target), r(ProofOfWorkLimit);
	double Difficulty;

//	r.divideWithRemainder(Target, q);
//
//	Difficulty = r;
//
//	// XXX: Fractional part is r/Target

	// Get the highest of the highest bits
	unsigned int hb1, hb2;
	hb1 = q.highestBit();
	hb2 = r.highestBit();
	if( hb2 > hb1 )
		hb1 = hb2;

	// Preserve the top N bits (with N being the storage unit of the big
	// number)
	hb1 -= TBitcoinHash::bitsPerBlock;
	q >>= hb1;
	r >>= hb1;

	// We can now be sure that the two number fill one block only of the
	// big numbers.  We'll let the compiler do the conversion to
	// floating point for us.
	Difficulty = static_cast<double>( r.getBlock(0) )
		/ static_cast<double>( q.getBlock(0) );

	return Difficulty;
}

//
// Function:	TNetworkParameters :: convertDifficultyToTarget
// Description:
//
TBitcoinHash TNetworkParameters::convertDifficultyToTarget( double ) const
{
	// Yuck.
	throw logic_error( "Don't call TNetworkParameters::convertDifficultyToTarget() until I've written it" );
}

//
// Function:	TNetworkParameters :: expectedGHashesPerBlock
// Description:
//
unsigned int TNetworkParameters::expectedGHashesPerBlock( const TBitcoinHash &Target ) const
{
	TBitcoinHash Hashes;
	// To generate a block, the nonce must be selected such that the
	// hash of the block must be less or equal to the current target.
	//
	// There are 2^256 possible hashes, and the current target divides
	// those into two -- above and below it.  Assuming that every hash
	// is equally likely, then the probability of finding an acceptable
	// hash will be
	//
	//    P = target / 2^256
	//
	// Imagine that target was 2^256; the probability would be 100% that
	// any given hash was acceptable -- we would need only 1.  Imagine
	// it was 2^255, half of all hashes would be above and half below;
	// 50% probability -- we would need two hashes.  2^254 would be 25%
	// and we would need 4 hashes.  And so on and so on.  In other
	// words:
	//
	//   N = 2^256 / target
	//
	// Where N is the expected number of hashes performed to find an
	// acceptable block.
	//
	Hashes = (TBitcoinHash(1) << 256);
	Hashes /= Target;

	// NB: Difficulty = MaxTarget / CurrentTarget
	//              N = 2^256 / CurrentTarget
	//              N = 2^256 * Difficulty / MaxTarget

	// NB: We also know that the expected time between blocks is
	// NEW_BLOCK_PERIOD (which we might call SECONDS_PER_BLOCK); we have
	// calculated the HASHES_PER_BLOCK_PERIOD, therefore, the computing
	// power of the network is approximately:
	//
	//   HASHES_PER_SECOND = HASHES_PER_BLOCK_PERIOD / NEW_BLOCK_PERIOD
	//

	// The proof of work limit for the production network is (2^224)-1,
	// which we can use to tell us the sort of scale we are talking
	// about.
	//
	//   2^256 / (2^224-1) = 2^32 (as near as makes no difference)
	//
	// So 4 gigahashes is the minimum expected number of hashes needed
	// to create a block.  The running network will presumably exceed this,
	// and we want space to express that.  Therefore we'll return our
	// answer in gigahashes, which gives plenty of room for expansion.

	Hashes /= 1000000000;

	return Hashes.getBlock(0);
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

