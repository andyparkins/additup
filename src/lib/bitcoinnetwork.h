// ----------------------------------------------------------------------------
// Project: additup
/// @file   bitcoinnetwork.h
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

// Catch multiple includes
#ifndef BITCOINNETWORK_H
#define BITCOINNETWORK_H

// -------------- Includes
// --- C
// --- C++
#include <list>
// --- Qt
// --- OS
// --- Project lib
// --- Project
#include "peer.h"
#include "hashtypes.h"


// -------------- Namespace
	// --- Imported namespaces
	using namespace std;


// -------------- Defines
// General
// Project


// -------------- Constants


// -------------- Typedefs (pre-structure)


// -------------- Enumerations


// -------------- Structures/Unions


// -------------- Typedefs (post-structure)


// -------------- Class pre-declarations
class TTransactionPool;
class TBlockPool;
class TBlock;


// -------------- Function pre-class prototypes


// -------------- Class declarations

//
// Class:	TNetworkParameters
// Description:
//
class TNetworkParameters
{
  public:
	TNetworkParameters();
	virtual const char *className() const { return "TNetworkParameters"; }

	uint32_t ProtocolVersion;
	TBlock *GenesisBlock;
	TBitcoinHash ProofOfWorkLimit;
	uint16_t DefaultTCPPort;
	uint32_t Magic;
	uint8_t BitcoinAddressPrefix;
	unsigned int DifficultyIncreaseSpacing;
	unsigned int TargetDifficultyIncreaseTime;

	// The official client defines these as constants, I think they're
	// better as network parameters, as they are pretty much arbitrarily
	// chosen.  They take up no more space and allow flexibility.  I've
	// kept the names to allow ease of understanding for those familiar
	// with the offical constants.
	unsigned int COINBASE_MATURITY;
	unsigned int MAX_BLOCK_SIZE;
};

//
// Class:	TBitcoinNetwork
// Description:
/// Object representing the entire bitcoin network.
//
/// There is a directory of bitcoin nodes; a list of bitcoin peers we're
/// connected to, blockchains, transaction chains.  All of that
/// information is public.  This class is the top level object for the
/// rest of the objects, and initial connection point.
//
class TBitcoinNetwork
{
  public:
	TBitcoinNetwork();

	void connect();
	void connect( TNodeInfo * );

	const TNetworkParameters *getNetworkParameters() const { return Parameters; }
	void setNetworkParameters( const TNetworkParameters *p ) { Parameters = p; }

  protected:
	const TNetworkParameters *Parameters;

	list<TBitcoinPeer* > Peers;
	TBitcoinPeer *Self;

	list<TNodeInfo *> Directory;

	TTransactionPool *TransactionPool;
	TBlockPool *BlockPool;
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
