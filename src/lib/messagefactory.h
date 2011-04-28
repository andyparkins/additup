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
class TBitcoinPeer;


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
	virtual const char *className() { return "TMessageFactory"; }

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

	void setPeer( TBitcoinPeer *p ) { Peer = p; }

  protected:
	virtual void init();

  protected:
	string RXBuffer;
	list<TMessage*> IncomingQueue;
	list<TMessage*> OutgoingQueue;

	bool Initialised;

	list<const TMessage *> Templates;

	TBitcoinPeer *Peer;
};

//
// Class:	TVersioningMessageFactory
// Description:
//
class TVersioningMessageFactory : public TMessageFactory
{
  public:
	const char *className() { return "TVersioningMessageFactory"; }

  protected:
	void init();

};

//
// Class:	TMessageFactory
// Description:
//
class TVersionedMessageFactory : public TMessageFactory
{
  public:
	const char *className() { return "TVersionedMessageFactory"; }

  protected:
//	void init();

  protected:
	virtual uint32_t minimumAcceptedVersion() const = 0;
};

//
// Class:	TMessageFactory_0
// Description:
//
class TMessageFactory_0 : public TVersionedMessageFactory
{
  public:
	const char *className() { return "TMessageFactory_0"; }

  protected:
	void init();

	uint32_t minimumAcceptedVersion() const { return 0; }
};

//
// Class:	TMessageFactory_0
// Description:
//
class TMessageFactory_10600 : public TVersionedMessageFactory
{
  public:
	const char *className() { return "TMessageFactory_10600"; }

  protected:
	void init();

	uint32_t minimumAcceptedVersion() const { return 10600; }
};

//
// Class:	TMessageFactory_0
// Description:
//
class TMessageFactory_20900 : public TVersionedMessageFactory
{
  public:
	const char *className() { return "TMessageFactory_20900"; }

  protected:
	void init();

	uint32_t minimumAcceptedVersion() const { return 20900; }
};

//
// Class:	TMessageFactory_0
// Description:
//
class TMessageFactory_31402 : public TVersionedMessageFactory
{
  public:
	const char *className() { return "TMessageFactory_31402"; }

  protected:
	void init();

	uint32_t minimumAcceptedVersion() const { return 31402; }
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
