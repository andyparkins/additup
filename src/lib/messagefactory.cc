// ----------------------------------------------------------------------------
// Project: additup
/// @file   messagefactory.cc
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

// Module include
#include "messagefactory.h"

// -------------- Includes
// --- C
// --- C++
#include <sstream>
#include <memory>
// --- Qt
// --- OS
// --- Project libs
// --- Project
#include "messages.h"
#include "script.h"
#include "logstream.h"


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Template instantiations


// -------------- Class declarations

//
// Function:	TMessageFactory :: TMessageFactory
// Description:
//
TMessageFactory::TMessageFactory() :
	Initialised( false ),
	Peer( NULL )
{
}

//
// Function:	TMessageFactory :: ~TMessageFactory
// Description:
//
TMessageFactory::~TMessageFactory()
{
	// Tidy up the template messages we made
	while( !Templates.empty() ) {
		delete Templates.front();
		Templates.erase( Templates.begin() );
	}
}

//
// Function:	TMessageFactory :: answer
// Description:
//
TMessage *TMessageFactory::answer( TMessage *Message )
{
	if( dynamic_cast<TMessageUnimplemented*>( Message ) != NULL ) {
		// No response needed
	} else if( dynamic_cast<TMessage_version_20900*>( Message ) != NULL ) {
		// RX< version209
		// TX> verack
		return new TMessage_verack();
	} else if( dynamic_cast<TMessage_version*>( Message ) != NULL ) {
		// No response needed
	} else if( dynamic_cast<TMessage_inv*>( Message ) != NULL ) {
		// RX< inv
		// TX> getdata
		// RX< block
		//  or
		// RX< inv
		// TX> getdata
		// RX< tx
	} else if( dynamic_cast<TMessage_getdata*>( Message ) != NULL ) {
		// RX< getdata
		// TX> block
		//  or
		// RX< getdata
		// TX> tx
	} else if( dynamic_cast<TMessage_getblocks*>( Message ) != NULL ) {
		// RX< getblocks
		// TX> inv
	} else if( dynamic_cast<TMessage_getheaders*>( Message ) != NULL ) {
		// RX< getheaders
		// TX> headers
	} else if( dynamic_cast<TMessage_getaddr*>( Message ) != NULL ) {
		// "The getaddr message sends a request to a node asking for
		// information about known active peers to help with identifying
		// potential nodes in the network. The response to receiving
		// this message is to transmit an addr message with one or more
		// peers from a database of known active peers. The typical
		// presumption is that a node is likely to be active if it has
		// been sending a message within the last three hours."
		return new TMessage_addr();
	} else if( dynamic_cast<TMessage_submitorder*>( Message ) != NULL ) {
		// RX< submitorder
		// TX> reply
	} else if( dynamic_cast<TMessage_checkorder*>( Message ) != NULL ) {
		// RX< checkorder
		// TX> reply
	} else if( dynamic_cast<TMessage_tx*>( Message ) != NULL ) {
		// No response needed
	} else if( dynamic_cast<TMessage_block*>( Message ) != NULL ) {
		// No response needed
	} else if( dynamic_cast<TMessage_headers*>( Message ) != NULL ) {
		// No response needed
	} else if( dynamic_cast<TMessage_verack*>( Message ) != NULL ) {
		// No response needed
	} else if( dynamic_cast<TMessage_addr*>( Message ) != NULL ) {
		// No response needed
	} else if( dynamic_cast<TMessage_reply*>( Message ) != NULL ) {
		// No response needed
	} else if( dynamic_cast<TMessage_ping*>( Message ) != NULL ) {
		// No response needed
	} else if( dynamic_cast<TMessage_alert*>( Message ) != NULL ) {
		// No response needed
	}

	return NULL;
}

//
// Function:	TMessageFactory :: receive
// Description:
// receive() handles the conversion of raw bytes from the peer to a
// TMessage child.
//
void TMessageFactory::receive( const string &s )
{
	list<const TMessage*>::const_iterator it;
	RXBuffer += s;
	istringstream iss( RXBuffer );
	iss.exceptions( ios::eofbit | ios::failbit | ios::badbit );

	TMessage *potential = NULL;

	// We should be initialised if we want the templates to be available
	if( !Initialised )
		init();

	streamoff sp;

	if( Peer == NULL )
		throw logic_error("TMessageFactory::receive() is impossible without a known peer");

	do {
		// Test against each template message
		for( it = Templates.begin(); it != Templates.end(); it++ ) {

			// Clone the template
			auto_ptr<TMessage> p( (*it)->clone() );

			// TMessages neeed to know details of where they came from
			p->setPeer( Peer );

//			log() << "Trying " << RXBuffer.size() << " bytes with "
//				<< p->className();

			try {
				// Store the current position
				sp = iss.tellg();

				p->read( iss );
//				log() << "*" << endl;
				potential = p.get();
				p.release();
				break;

			} catch( ios::failure &e ) {
//				log() << " - " << e.what() << endl;
				// If we run out of message from the source, then leave,
				// hoping for more
				break;

			} catch( message_parse_error_underflow &e ) {
//				log() << " - " << e.what() << endl;
				// If we run out of message from the source, then leave,
				// hoping for more
				break;

			} catch( message_parse_error_magic &e ) {
				// Skip
			} catch( message_parse_error_version &e ) {
//				log() << " - " << e.what() << endl;
				// Try next template with the same data
				iss.seekg( sp, ios::beg );

			} catch( message_parse_error_type &e ) {
//				log() << " - " << e.what() << endl;
				// Try next template with the same data
				iss.seekg( sp, ios::beg );

			} catch( message_parse_error &e ) {
//				log() << " - " << e.what() << endl;

				// Anything else, chuck the packet away
				sp = iss.tellg();
				RXBuffer = RXBuffer.substr( sp, RXBuffer.size() - sp );
				break;
			}
		}

		if( potential != NULL ) {
//			log() << *potential << endl;
			// Push it onto the receive queue
			IncomingQueue.push_back( potential );
			// Remove the bytes from the RX buffer
			sp = potential->getMessageSize();
			RXBuffer = RXBuffer.substr( sp, RXBuffer.size() - sp );

			return;
		} else {
			try {
				iss.seekg(sp + 1);
			} catch( ... ) {
				break;
			}
		}
	} while( true );
}

//
// Function:	TMessageFactory :: init
// Description:
//
void TMessageFactory::init()
{
	list<const TMessage*>::const_iterator it;

	// Set the template flag on any messages the child class created
	for( it = Templates.begin(); it != Templates.end(); it++ ) {
		(*it)->setTemplate( true );
	}

	Initialised = true;
}

// ---------

//
// Function:	TVersioningMessageFactory :: init
// Description:
//
void TVersioningMessageFactory::init()
{
	// Must be in reverse order of version so that the highest matches
	// first
	Templates.push_back( new TMessage_version_20900() );
	Templates.push_back( new TMessage_version_10600() );
	Templates.push_back( new TMessage_version_0() );

	TMessageFactory::init();
}

// ---------

//
// Function:	TMessageFactory_0 :: init
// Description:
//
void TMessageFactory_0::init()
{
	Templates.push_back( new TMessage_version_0() );
	Templates.push_back( new TMessage_verack() );
	Templates.push_back( new TMessage_addr_0() );
	Templates.push_back( new TMessage_inv() );
	Templates.push_back( new TMessage_getdata() );
	Templates.push_back( new TMessage_getblocks() );
	Templates.push_back( new TMessage_tx() );
	Templates.push_back( new TMessage_block() );
	Templates.push_back( new TMessage_getaddr() );
	Templates.push_back( new TMessage_checkorder() );
	Templates.push_back( new TMessage_submitorder() );
	Templates.push_back( new TMessage_reply() );
	Templates.push_back( new TMessage_ping() );
	Templates.push_back( new TMessage_alert() );

	TMessageFactory::init();
}

//
// Function:	TMessageFactory_0 :: createVersionedBitcoinScript
// Description:
//
TBitcoinScript *TMessageFactory_0::createVersionedBitcoinScript() const
{
	return new TBitcoinScript_0;
}

// ---------

//
// Function:	TMessageFactory_10600 :: init
// Description:
//
void TMessageFactory_10600::init()
{
	Templates.push_back( new TMessage_version_10600() );
	Templates.push_back( new TMessage_verack() );
	Templates.push_back( new TMessage_addr_0() );
	Templates.push_back( new TMessage_inv() );
	Templates.push_back( new TMessage_getdata() );
	Templates.push_back( new TMessage_getblocks() );
	Templates.push_back( new TMessage_tx() );
	Templates.push_back( new TMessage_block() );
	Templates.push_back( new TMessage_getaddr() );
	Templates.push_back( new TMessage_checkorder() );
	Templates.push_back( new TMessage_submitorder() );
	Templates.push_back( new TMessage_reply() );
	Templates.push_back( new TMessage_ping() );
	Templates.push_back( new TMessage_alert() );

	TMessageFactory::init();
}

//
// Function:	TMessageFactory_10600 :: createVersionedBitcoinScript
// Description:
//
TBitcoinScript *TMessageFactory_10600::createVersionedBitcoinScript() const
{
	return new TBitcoinScript_0;
}

// ---------

//
// Function:	TMessageFactory_20900 :: init
// Description:
//
void TMessageFactory_20900::init()
{
	Templates.push_back( new TMessage_version_20900() );
	Templates.push_back( new TMessage_verack() );
	Templates.push_back( new TMessage_addr_0() );
	Templates.push_back( new TMessage_inv() );
	Templates.push_back( new TMessage_getdata() );
	Templates.push_back( new TMessage_getblocks() );
	Templates.push_back( new TMessage_tx() );
	Templates.push_back( new TMessage_block() );
	Templates.push_back( new TMessage_getaddr() );
	Templates.push_back( new TMessage_checkorder() );
	Templates.push_back( new TMessage_submitorder() );
	Templates.push_back( new TMessage_reply() );
	Templates.push_back( new TMessage_ping() );
	Templates.push_back( new TMessage_alert() );

	TMessageFactory::init();
}

//
// Function:	TMessageFactory_20900 :: createVersionedBitcoinScript
// Description:
//
TBitcoinScript *TMessageFactory_20900::createVersionedBitcoinScript() const
{
	return new TBitcoinScript_0;
}

// ---------

//
// Function:	TMessageFactory_31402 :: init
// Description:
//
void TMessageFactory_31402::init()
{
	Templates.push_back( new TMessage_version_20900() );
	Templates.push_back( new TMessage_verack() );
	Templates.push_back( new TMessage_addr_31402() );
	Templates.push_back( new TMessage_inv() );
	Templates.push_back( new TMessage_getdata() );
	Templates.push_back( new TMessage_getblocks() );
	Templates.push_back( new TMessage_getheaders() );
	Templates.push_back( new TMessage_tx() );
	Templates.push_back( new TMessage_block() );
	Templates.push_back( new TMessage_headers() );
	Templates.push_back( new TMessage_getaddr() );
	Templates.push_back( new TMessage_checkorder() );
	Templates.push_back( new TMessage_submitorder() );
	Templates.push_back( new TMessage_reply() );
	Templates.push_back( new TMessage_ping() );
	Templates.push_back( new TMessage_alert() );

	TMessageFactory::init();
}

//
// Function:	TMessageFactory_31402 :: createVersionedBitcoinScript
// Description:
//
TBitcoinScript *TMessageFactory_31402::createVersionedBitcoinScript() const
{
	return new TBitcoinScript_0;
}


// -------------- Class member definitions


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include "unittest.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
		TVersioningMessageFactory PF;

		const string *p = UNITTESTSampleMessages;
		while( !p->empty() ) {
			PF.receive( *p );
			if( PF.newestIncoming() != NULL ) {
				log() << "PF.queue() = " << *PF.newestIncoming() << endl;
			} else {
				log() << "no packet yet" << endl;
			}
			p++;
		}

	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

