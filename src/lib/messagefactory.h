// ----------------------------------------------------------------------------
// Project: additup
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
class TBitcoinScript;


// -------------- Function pre-class prototypes


// -------------- Class declarations

//
// Class:	TMessageFactory
// Description:
// TMessageFactory builds TMessages from incoming bytes.
//
// TMessageFactorys are owned (typically) by a TBitcoinPeer, which uses
// TVersioningMessageFactory to read initial version messages, and then
// TMessage_version::createMessageFactory() to create a
// TVersionedMessageFactory to read messages.
//
class TMessageFactory
{
  public:
	TMessageFactory();
	virtual ~TMessageFactory();
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
	TMessage *nextIncoming() {
		if( IncomingQueue.size() == 0 )
			return NULL;
		TMessage *x = IncomingQueue.front();
		IncomingQueue.pop_front();
		return x;
	}

	void setPeer( TBitcoinPeer *p ) { Peer = p; }

	string::size_type findNextMagic( const string &, string::size_type = 0 ) const;

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
// TVersioningMessageFactory builds TMessage_version children only.
//
// The special property of TMessage_version children is that they supply
// the createMessageFactory() member function, which creates a
// TVersionedMessageFactory child.  That is to say, that the unversioned
// version message creates the appropriate factory for the version
// specified in that version message.
//
class TVersioningMessageFactory : public TMessageFactory
{
  public:
	TVersioningMessageFactory() : VerackSent( false ), VerackReceived( false ) {}

	const char *className() { return "TVersioningMessageFactory"; }

	TMessage *answer( TMessage * );

	bool getReady() const { return VerackSent && VerackReceived; }

  protected:
	void init();

	bool VerackSent;
	bool VerackReceived;
};

//
// Class:	TVersionedMessageFactory
// Description:
// TVersionedMessageFactory builds coherently versioned TMessages
//
// Not all versions of the protocol support all TMessages; and some
// messages have different implementations in different protocol
// versions, TVersionedMessageFactory supplies the appropriate set for a
// given protocol version.
//
// Or rather, it's children do.  By implementing minimumAcceptedVersion()
// and supplying an init() that creates the correct TMessage templates.
//
// If a new message type is added by the bitcoin developers, then it
// gets a TVersionedMessageFactory child to define where it goes, and a
// new TVersion_message child to create that factory.
//
class TVersionedMessageFactory : public TMessageFactory
{
  public:
	const char *className() { return "TVersionedMessageFactory"; }

	virtual TBitcoinScript *createVersionedBitcoinScript() const = 0;

  protected:
	void init() = 0;

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

	TBitcoinScript *createVersionedBitcoinScript() const;

  protected:
	void init();

	uint32_t minimumAcceptedVersion() const { return 0; }
};

//
// Class:	TMessageFactory_10600
// Description:
//
class TMessageFactory_10600 : public TVersionedMessageFactory
{
  public:
	const char *className() { return "TMessageFactory_10600"; }

	TBitcoinScript *createVersionedBitcoinScript() const;

  protected:
	void init();

	uint32_t minimumAcceptedVersion() const { return 10600; }
};

//
// Class:	TMessageFactory_20900
// Description:
//
class TMessageFactory_20900 : public TVersionedMessageFactory
{
  public:
	const char *className() { return "TMessageFactory_20900"; }

	TBitcoinScript *createVersionedBitcoinScript() const;

  protected:
	void init();

	uint32_t minimumAcceptedVersion() const { return 20900; }
};

//
// Class:	TMessageFactory_31402
// Description:
//
class TMessageFactory_31402 : public TVersionedMessageFactory
{
  public:
	const char *className() { return "TMessageFactory_31402"; }

	TBitcoinScript *createVersionedBitcoinScript() const;

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
