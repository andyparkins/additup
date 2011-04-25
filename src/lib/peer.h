// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   peer.h
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
#ifndef PEER_H
#define PEER_H

// -------------- Includes
// --- C
#include <stdint.h>
// --- C++
#include <iostream>
// --- Qt
// --- OS
// --- Project lib
// --- Project


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
class TBitcoinNetwork;
class TMessageFactory;


// -------------- Function pre-class prototypes


// -------------- Class declarations

//
// Class:	TNodeInfo
// Description:
//
class TNodeInfo
{
  public:
	TNodeInfo( uint32_t ip ) { IPv4 = ip; }

	ostream &write( ostream & ) const;
	string get() const;

	operator bool() const { return IPv4 != 0; }
	operator uint32_t() const { return IPv4; }

  protected:
	uint32_t IPv4;
};

//
// Class:	TBitcoinPeer
// Description:
//
class TBitcoinPeer
{
  public:
	TBitcoinPeer( TNodeInfo *, TBitcoinNetwork * );

  protected:
	TBitcoinNetwork *Network;
	TMessageFactory *Factory;

	TNodeInfo *Info;
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
