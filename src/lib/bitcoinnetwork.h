// ----------------------------------------------------------------------------
// Project: bitcoin
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
#include <boost/smart_ptr.hpp>
#include <list>
// --- Qt
// --- OS
// --- Project lib
// --- Project
#include "peer.h"
#include "extraint.h"


// -------------- Namespace
	// --- Imported namespaces
	using namespace std;
	using namespace boost;


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

	uint32_t ProtocolVersion;
	// Block GenesisBlock;
	TBigInteger ProofOfWorkLimit;
	uint16_t DefaultTCPPort;
	uint32_t Magic;
	uint8_t BitcoinAddressPrefix;
	unsigned int DifficultyIncreaseSpacing;
	unsigned int TargetDifficultyIncreaseTime;
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

  protected:
	const TNetworkParameters *Parameters;

	list<shared_ptr<TBitcoinPeer> > Peers;
	weak_ptr<TBitcoinPeer> Self;

	list<shared_ptr<TNodeInfo> > Directory;

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
