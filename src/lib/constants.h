// ----------------------------------------------------------------------------
// Project: bitcoin
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
// --- Qt
// --- OS
// --- Project lib
// --- Project
#include "peer.h"
#include "bitcoinnetwork.h"


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


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)
extern const TOfficialSeedNode SEED_NODES[];

extern const TNetworkParameters *KNOWN_NETWORKS[];
extern const TNetworkParameters *NETWORK_TESTNET;
extern const TNetworkParameters *NETWORK_PRODNET;


// End of conditional compilation
#endif
