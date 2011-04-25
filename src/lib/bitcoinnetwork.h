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
#include "constants.h"
#include "peer.h"


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


// -------------- Function pre-class prototypes


// -------------- Class declarations

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

  protected:
	TNetworkParameters *Parameters;

	list<shared_ptr<TBitcoinPeer> > Peers;
	weak_ptr<TBitcoinPeer> Self;

	list<shared_ptr<TNodeInfo> > Directory;
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
