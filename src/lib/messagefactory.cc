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
#include "peer.h"
#include "bitcoinnetwork.h"


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

	// We should be initialised if we want the templates to be available
	if( !Initialised )
		init();

	streamoff sp = 0;

	if( Peer == NULL )
		throw runtime_error("TMessageFactory::receive() is impossible without a known peer");

	while( !iss.str().empty() && sp < iss.str().size() ) {
		TMessage *WorkingClone = NULL;

		// --- Sync

		string::size_type pos = findNextMagic(iss.str(), sp);
		if( pos == string::npos || pos >= iss.str().size() ) {
			// If there is no magic in the string, then there is no point
			// looking for a message in it.  This buffer is nothing we
			// can read, so we discard all of it.
//			log() << "Discarding " << RXBuffer.size() << endl;
			RXBuffer.clear();
			break;
		} else {
//			log() << "Ignoring " << pos << endl;
			// Anything before the magic has been read or isn't a
			// message
			RXBuffer = RXBuffer.substr( pos, RXBuffer.size() - pos );
			// And now repoint the stringstream at this shortened buffer
			iss.str( RXBuffer );
			iss.seekg( 0, ios::beg );
		}

		// --- Try to read

		// bookmark position
		sp = iss.tellg();

		// Test against each template message
		for( it = Templates.begin(); it != Templates.end(); it++ ) {
			// The clone gets automatically deleted unless we release
			// the auto_ptr
			auto_ptr<TMessage> AutoTidyClone( (*it)->clone() );
			AutoTidyClone->setPeer( Peer );

			// Clear outstanding exceptions
			iss.clear();
			// Restore our bookmark
			iss.seekg( sp, ios::beg );

			try {
				// Attempt read
				AutoTidyClone->read( iss );
			} catch( ios::failure &e ) {
//				log() << "[FACT] Parser at byte " << sp << "/" << iss.str().size();
//				log() << " D: ";
//				TLog::hexify( log(), iss.str() );
//				log() << " - " << e.what() << ", " << (*it)->className() << endl;
				// If we run out of message from the source, then we
				// leave the pointer where it is, while in principle
				// this is identical to the underflow error, it only
				// gets thrown by body reads not header reads.  Fail
				// during a body read means the packet was correctly
				// identified, there was just insufficient data.  We
				// therefore return in the hopes that we'll get some
				// more
				return;

			} catch( message_parse_error_underflow &e ) {
//				log() << "[FACT] Parser at byte " << sp << "/" << iss.str().size();
//				log() << " D: ";
//				TLog::hexify( log(), iss.str() );
//				log() << " - " << e.what() << ", " << (*it)->className() << endl;
				// Same as above, but caught by the parser
				return;

			} catch( message_parse_error_magic &e ) {
				log() << "[FACT] Parser at byte " << sp << "/" << iss.str().size();
				TLog::hexify( log(), iss.str() );
				log() << " - " << e.what() << ", " << (*it)->className() << endl;
				// A magic error applies to all message types, so we
				// break out of this loop, there is no point trying
				// other templates
				break;

			} catch( message_parse_error_version &e ) {
				log() << "[FACT] Parser at byte " << sp << "/" << RXBuffer.size();
				log() << " D: ";
				TLog::hexify( log(), RXBuffer );
				log() << " - " << e.what() << ", " << (*it)->className() << endl;
				// Version errors shouldn't happen after versioning has
				// taken place, it means a TMessage_version_X was used
				// to read a version message that had a lower value than
				// it supported.  When versioning is active (handshaking
				// mode in TBitcoinPeer), this is exactly the same error
				// as a type error: not a problem, and we simply try
				// the next template.
				continue;

			} catch( message_parse_error_type &e ) {
//				log() << "[FACT] Parser at byte " << sp << "/" << RXBuffer.size();
//				log() << " D: ";
//				TLog::hexify( log(), RXBuffer );
//				log() << " - " << e.what() << ", " << (*it)->className() << endl;
				// This incoming stream contains a message that is of a
				// different type than the template; this is perfectly
				// normal, most of the time the template won't be the
				// right type, we simply try next template with the same data
				continue;

			} catch( message_parse_error &e ) {
				log() << "[FACT] Parser at byte " << sp << "/" << RXBuffer.size();
				log() << " D: ";
				TLog::hexify( log(), RXBuffer );
				log() << " - " << e.what() << ", " << (*it)->className() << endl;
				// Any other error from the parser means we can't handle
				// this message
				sp += 1;
				continue;
			}

			// If there were no errors, we can release the auto_ptr, and
			// pass it out of the loop
			WorkingClone = AutoTidyClone.get();
			AutoTidyClone.release();
			break;
		}

		if( WorkingClone != NULL ) {
			// If the read successfully converted bytes into a TMessage
			// then push it onto the receive queue
			Peer->queueIncoming( WorkingClone );
			// The data we've parsed can be removed from the stream
			sp = iss.tellg();
//			log() << "Eaten " << sp << " to make " << *WorkingClone << endl;
			RXBuffer = RXBuffer.substr( sp, RXBuffer.size() - sp );
			// Repoint the stringstream at this shortened buffer
			iss.str( RXBuffer );
			// ... and point at the start
			sp = 0;
			if( !continuousParse() )
				break;
		} else {
			// Not having found a message makes it harder to predict
			// were we should start the next search for the magic.  The
			// best we can do is start one byte along and try again.
			sp += 1;
		}
	}
}

//
// Function:	TMessageFactory :: findNextMagic
// Description:
//
string::size_type TMessageFactory::findNextMagic( const string &s, string::size_type start ) const
{
	if( Peer == NULL || Peer->getNetworkParameters() == NULL ) {
		// If we have no peer, then we have no magic available,
		// which makes it hard to synchronise.  We'll have
		// to fall back to moving one byte at a time
//		log() << "Magic search impossible" << endl;
		return start;
	}

	// Convert the magic to a string
	TLittleEndian32Element Magic;
	Magic = Peer->getNetworkParameters()->Magic;
	ostringstream oss;
	oss << Magic;

//	log() << "Next magic ";
//	TLog::hexify( log(), oss.str() );
//	log() << " from " << start;
	start = s.find( oss.str(), start );

	if( start == string::npos ) {
//		log() << " not found" << endl;
		return s.size();
	}
//	log() << " found at buffer position " << start << endl;

	return start;
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
	Templates.push_back( new TMessage_version_31402() );
	Templates.push_back( new TMessage_version_20900() );
	Templates.push_back( new TMessage_version_10600() );
	Templates.push_back( new TMessage_version_1() );

	TMessageFactory::init();
}

// ---------

//
// Function:	TMessageFactory_1 :: init
// Description:
//
void TMessageFactory_1::init()
{
//	Templates.push_back( new TMessage_version_1() );
	Templates.push_back( new TMessage_verack() );
	Templates.push_back( new TMessage_addr_1() );
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
// Function:	TMessageFactory_1 :: createVersionedBitcoinScript
// Description:
//
TBitcoinScript *TMessageFactory_1::createVersionedBitcoinScript() const
{
	return new TBitcoinScript_1;
}

// ---------

//
// Function:	TMessageFactory_10600 :: init
// Description:
//
void TMessageFactory_10600::init()
{
//	Templates.push_back( new TMessage_version_10600() );
	Templates.push_back( new TMessage_verack() );
	Templates.push_back( new TMessage_addr_1() );
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
	return new TBitcoinScript_1;
}

// ---------

//
// Function:	TMessageFactory_20900 :: init
// Description:
//
void TMessageFactory_20900::init()
{
//	Templates.push_back( new TMessage_version_20900() );
	Templates.push_back( new TMessage_verack() );
	Templates.push_back( new TMessage_addr_1() );
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
	return new TBitcoinScript_1;
}

// ---------

//
// Function:	TMessageFactory_31402 :: init
// Description:
//
void TMessageFactory_31402::init()
{
//	Templates.push_back( new TMessage_version_20900() );
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
	return new TBitcoinScript_1;
}


// -------------- Class member definitions


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include "unittest.h"
#include "logstream.h"
#include "peer.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
		TBitcoinPeer Peer;
		TVersioningMessageFactory PF;
		PF.setPeer( &Peer );

		log() << "--- " << PF.className() << endl;

		const string *p = UNITTESTSampleMessages;
		while( !p->empty() ) {
			PF.receive( *p );
			if( Peer.newestIncoming() != NULL ) {
				log() << "PF.queue() = " << *Peer.newestIncoming() << endl;
			} else {
				log() << "no packet yet" << endl;
			}
			p++;
		}

	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	try {
		TBitcoinPeer Peer;
		TMessageFactory_31402 PF;
		PF.setPeer( &Peer );

		log() << "--- " << PF.className() << endl;

		const string *p = UNITTESTSampleMessages;
		while( !p->empty() ) {
			PF.receive( *p );
			if( Peer.newestIncoming() != NULL ) {
				log() << "PF.queue() = " << *Peer.newestIncoming() << endl;
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

