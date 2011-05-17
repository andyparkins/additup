// ----------------------------------------------------------------------------
// Project: additup
/// @file   constants.h
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
#ifndef CONSTANTS_H
#define CONSTANTS_H

// -------------- Includes
// --- C
#include <stdint.h>
// --- C++
#include <iostream>
#include <string>
#include <set>
// --- Qt
// --- OS
// --- Project lib
// --- Project
#include "peer.h"
#include "bitcoinnetwork.h"
#include "singleton.h"


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

//
// Class:	TOfficialSeedNode
// Description:
// A simple wrapper around the 32-bit seed node integer obtained from
// the official bitcoin client.
//
class TOfficialSeedNode : public TNodeInfo
{
  public:
	TOfficialSeedNode( uint32_t );
};



// -------------- Function pre-class prototypes


// -------------- Class declarations

//
// Class:	KNOWN_NETWORKS
// Description:
// Singleton to represent the array of known networks.  This is a
// singleton so that initialisation is deferred to the first use, rather
// than left to being at the whim of the compiler and linker.
//
class KNOWN_NETWORKS
{
  public:
	typedef set<const TNetworkParameters *>::iterator iterator;
	typedef set<const TNetworkParameters *>::const_iterator const_iterator;

  public:
	KNOWN_NETWORKS();
	~KNOWN_NETWORKS();

	iterator begin() { return KnownNetworks.begin(); }
	iterator end() { return KnownNetworks.end(); }
	const_iterator begin() const { return KnownNetworks.begin(); }
	const_iterator end() const { return KnownNetworks.end(); }

  protected:
	set<const TNetworkParameters *> KnownNetworks;
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)
extern const TOfficialSeedNode SEED_NODES[];

extern template class TSingleton<KNOWN_NETWORKS>;
extern const TNetworkParameters *NETWORK_TESTNET;
extern const TNetworkParameters *NETWORK_PRODNET;


// End of conditional compilation
#endif
