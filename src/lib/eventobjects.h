// ----------------------------------------------------------------------------
// Project: additup
/// @file   eventobjects.h
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
#ifndef EVENTOBJECTS_H
#define EVENTOBJECTS_H

// -------------- Includes
// --- C
// --- C++
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
class TMessage;
class TInventoryElement;


// -------------- Function pre-class prototypes


// -------------- Class declarations

//
// Class:	TBitcoinEventObject
// Description:
//
class TBitcoinEventObject
{
  public:
	TBitcoinEventObject() {}
	virtual ~TBitcoinEventObject() {}

	virtual void messageReceived( const TMessage * ) const {}
	virtual void inventoryHashError( const TInventoryElement * ) const {}
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
