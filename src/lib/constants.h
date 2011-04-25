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
// --- Project
// --- Project lib


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
class TOfficialSeedNode
{
  public:
	TOfficialSeedNode( uint32_t );

	ostream &write( ostream & ) const;
	string get() const;

	operator bool() const { return IPv4 != 0; }
	operator uint32_t() const { return IPv4; }

  protected:
	uint32_t IPv4;
};

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
	// BigInteger ProofOfWorkLimit;
	uint16_t DefaultTCPPort;
	uint32_t Magic;
	uint8_t BitcoinAddressPrefix;
	unsigned int DifficultyIncreaseSpacing;
	unsigned int TargetDifficultyIncreaseTime;
};


// -------------- Function pre-class prototypes


// -------------- Class declarations


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)
extern const TOfficialSeedNode SEED_NODES[];
extern const TNetworkParameters *NETWORK_TESTNET;
extern const TNetworkParameters *NETWORK_PRODNET;

// End of conditional compilation
#endif