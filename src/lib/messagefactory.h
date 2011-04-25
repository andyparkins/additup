// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   messagefactory.h
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
#ifndef MESSAGEFACTORY_H
#define MESSAGEFACTORY_H

// -------------- Includes
// --- C
#include <stdint.h>
// --- C++
#include <string>
#include <list>
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


// -------------- Function pre-class prototypes


// -------------- Class declarations

//
// Class:	TMessageFactory
// Description:
//
class TMessageFactory
{
  public:
	TMessageFactory();
	virtual ~TMessageFactory() {}

	void receive( const string & );
	void transmit( TMessage * );

	virtual TMessage *answer( TMessage * );

	TMessage *oldestIncoming() const {
		if( IncomingQueue.size() == 0 )
			return NULL;
		return IncomingQueue.front();
	}
	TMessage *newestIncoming() const {
		if( IncomingQueue.size() == 0 )
			return NULL;
		return IncomingQueue.back();
	}

	virtual void init();

  protected:
	string RXBuffer;
	list<TMessage*> IncomingQueue;
	list<TMessage*> OutgoingQueue;

	bool Initialised;

	list<const TMessage *> Templates;
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
