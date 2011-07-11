// ----------------------------------------------------------------------------
// Project: additup
/// @file   peer.cc
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
#include "peer.h"

// -------------- Includes
// --- C
// --- C++
#include <sstream>
#include <memory>
#include <stdexcept>
// --- Qt
// --- OS
// --- Project libs
// --- Project
#include "messagefactory.h"
#include "messages.h"
#include "messageelements.h"
#include "logstream.h"
#include "constants.h"


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Template instantiations


// -------------- Class declarations

//
// Function:	TNodeInfo :: write
// Description:
//
TNodeInfo::TNodeInfo( uint32_t ip ) :
	IPv4( ip ),
	LastConnectAttempt(0),
	LastConnectSuccess(0)
{
}

//
// Function:	TNodeInfo :: write
// Description:
//
ostream &TNodeInfo::write( ostream &os ) const
{
	// Big endian
	os.put((IPv4 & 0xff000000) >> 24);
	os.put((IPv4 & 0xff0000) >> 16);
	os.put((IPv4 & 0xff00) >> 8);
	os.put((IPv4 & 0xff) >> 0);

	return os;
}

//
// Function:	TNodeInfo :: printOn
// Description:
//
ostream &TNodeInfo::printOn( ostream &os ) const
{
	os << ((IPv4 & 0xff000000) >> 24)
		<< "." << ((IPv4 & 0xff0000) >> 16)
		<< "." << ((IPv4 & 0xff00) >> 8)
		<< "." << ((IPv4 & 0xff) >> 0)
		<< " " << LastConnectAttempt << ", " << LastConnectSuccess;

	return os;
}

//
// Function:	TNodeInfo :: get
// Description:
//
string TNodeInfo::get() const
{
	ostringstream oss;

	write(oss);

	return oss.str();
}

// --------

//
// Function:	TBitcoinPeer :: TBitcoinPeer
// Description:
//
TBitcoinPeer::TBitcoinPeer( const TNodeInfo *info, TBitcoinNetwork *network ) :
	Info( info ),
	Network( network ),
	Factory( NULL ),
	State( Unconnected ),
	VersionSent( false ),
	VerackReceived( false )
{
}

//
// Function:	TBitcoinPeer :: ~TBitcoinPeer
// Description:
//
TBitcoinPeer::~TBitcoinPeer()
{
	delete Factory;

	// Tidy up anything left on the queues
	while( !IncomingQueue.empty() ) {
		delete IncomingQueue.front();
		IncomingQueue.erase( IncomingQueue.begin() );
	}
	while( !OutgoingQueue.empty() ) {
		delete OutgoingQueue.front();
		OutgoingQueue.erase( OutgoingQueue.begin() );
	}
}

//
// Function:	TBitcoinPeer :: getNetworkParameters
// Description:
//
const TNetworkParameters *TBitcoinPeer::getNetworkParameters() const
{
	if( Network == NULL )
		return NULL;
	return Network->getNetworkParameters();
}

//
// Function:	TBitcoinPeer :: oldestIncoming
// Description:
//
TMessage *TBitcoinPeer::oldestIncoming() const
{
	if( IncomingQueue.empty() )
		return NULL;
	return IncomingQueue.front();
}

//
// Function:	TBitcoinPeer :: nextIncoming
// Description:
//
TMessage *TBitcoinPeer::newestIncoming() const
{
	if( IncomingQueue.empty() )
		return NULL;
	return IncomingQueue.back();
}

//
// Function:	TBitcoinPeer :: nextIncoming
// Description:
//
TMessage *TBitcoinPeer::nextIncoming()
{
	if( IncomingQueue.empty() )
		return NULL;
	TMessage *x = IncomingQueue.front();
	IncomingQueue.pop_front();
	return x;
}

//
// Function:	TBitcoinPeer :: nextOutgoing
// Description:
//
TMessage *TBitcoinPeer::nextOutgoing()
{
	if( OutgoingQueue.empty() )
		return NULL;
	TMessage *x = OutgoingQueue.front();
	OutgoingQueue.pop_front();
	return x;
}

//
// Function:	TBitcoinPeer :: queueOutgoing
// Description:
//
void TBitcoinPeer::queueOutgoing( TMessage *m )
{
	// If it's on our queue it's going to this peer
	m->setPeer( this );
	// Make sure the message is valid
	m->setFields();
	OutgoingQueue.push_back( m );
}

//
// Function:	TBitcoinPeer :: queueIncoming
// Description:
//
void TBitcoinPeer::queueIncoming( TMessage *m )
{
	// If it's on our queue it came from our peer
	m->setPeer( this );
	IncomingQueue.push_back( m );
}

//
// Function:	TBitcoinPeer :: receive
// Description:
// receive() passes incoming bytes to the appropriate factory, creating
// factories as necessary.
//
// When unconnected there is no factory.  Once connected, we could be
// talking to any version of node, so we use TVersioningMessageFactory
// to read whatever TMessage_version is sent to us.  TMessage_versions
// are special because they know how to create a factory appropriate to
// the version the message specifies.  Once a TMessage_version is
// received, that factory replaced our TVersioningMessageFactory, and we
// are in full communication.
//
//
void TBitcoinPeer::receive( const TByteArray &s )
{
	TByteArray incoming(s);

	if( State == Unconnected ) {
		log() << "[PEER] State: Unconnected" << endl;
		// Can't receive anything until we at least have comms
		return;
	}

	if( State == Connecting ) {
		log() << "[PEER] State: Connecting" << endl;
		if( Factory != NULL )
			throw logic_error( "TBitcoinPeer::receive() can't have a factory while unconnected" );

		VersionSent = false;
		VerackReceived = false;

		// The versioning factory only understands version messages
		delete Factory;
		Factory = new TVersioningMessageFactory;

		// The TVersioningMessageFactory is told we are the peer it
		// should use; TVersioningMessageFactory tells its created
		// TMessage_versions that peer; TMessage_versions tell their
		// created TVersionedMessageFactorys that peer.  Hence we don't
		// need to do anything other than this initial setPeer() call.
		Factory->setPeer( this );

		// We're now in handshaking mode
		State = Parameters;
	}

	if( State == Parameters ) {
		log() << "[PEER] State: Parameters" << endl;
		if( Network != NULL && getNetworkParameters() != NULL ) {
			log() << "[PEER] Network parameters already available, "
				<< getNetworkParameters()->className() << endl;
			State = Handshaking;
		} else {
			log() << "[PEER] Network parameters not available" << endl;
			// If we don't already know our network, then the first thing
			// we're looking for is a magic number to tell us what network
			// we're connected to.

			// We will make the assumption that the accidental
			// transmission of bytes matching a network magic number is
			// impossible (or at least 2^32 to 1 against).  Therefore
			// we'll look through a window until we see a match.

			// Try reading four bytes starting from each byte in turn
			istringstream iss(incoming);
			while( iss.good() ) {
				TLittleEndian32Element PotentialMagic;
				streamoff pos;
				iss.exceptions( ios::eofbit | ios::failbit | ios::badbit );

				// Bookmark
				pos = iss.tellg();

				// Attempt read
				try {
					PotentialMagic.read(iss);
				} catch( ... ) {
					break;
				}

				// Restore
				iss.seekg(static_cast<streamoff>(pos+1));

				// Compare magic against all known networks
				log() << "[PEER]  - Testing potential network magic " << hex << PotentialMagic.getValue() << dec << endl;

				set<const TNetworkParameters *>::const_iterator p = TSingleton<KNOWN_NETWORKS>::O().begin();
				while( p != TSingleton<KNOWN_NETWORKS>::O().end() ) {
					if( (*p)->Magic == PotentialMagic.getValue() )
						break;
					p++;
				}
				if( p != TSingleton<KNOWN_NETWORKS>::O().end() ) {
					if( Network != NULL ) {
						log() << "[PEER]  - Network magic found, using " << (*p)->className() << endl;
						Network->setNetworkParameters( *p );
					} else {
						log() << "[PEER]  - Network magic for " << (*p)->className() << " will not be used" << endl;
					}
					State = Handshaking;
					break;
				}
			}
		}
	}

	if( State == Handshaking ) {
		log() << "[PEER] State: Handshaking" << endl;
		if( Factory == NULL )
			throw logic_error( "TBitcoinPeer::receive() must have factory in handshaking mode" );

		// Send our version
		if( !VersionSent ) {
			VersionSent = true;
			queueOutgoing( new TMessage_version_31402 );
		}

		// Convert stream to messages
		try {
			Factory->receive(incoming);
			incoming.clear();
		} catch( exception &e ) {
			log() << "[PEER] Error parsing message, " << e.what() << endl;
			return;
		}

		auto_ptr<TMessage> Message( nextIncoming() );

		if( dynamic_cast<TMessage_version*>( Message.get() ) != NULL ) {
			// We don't care exactly what version message, the
			// TMessage_version base class is more than enough for us to
			// query the message
			TMessage_version *VersionMessage = reinterpret_cast<TMessage_version*>( Message.get() );

			log() << "[PEER] Version message received, " << *VersionMessage << endl;
			TMessageFactory *newFactory = VersionMessage->createMessageFactory();
			log() << "[PEER] Factory is now " << newFactory->className() << endl;
			// Any bytes left over in the factory we're about to delete
			// must be forwarded to the new factory
			newFactory->receive( Factory->getRXBuffer() );
			delete Factory;
			Factory = newFactory;

			// Acknowledge every version received, even if the remote
			// chooses to send more than one (which it shouldn't)
			queueOutgoing( new TMessage_verack() );
		} else if( dynamic_cast<TMessage_verack*>( Message.get() ) != NULL ) {
			// Not sure we care...  If we don't get a verack, then
			// presumably the remote will just hang up on us -- what
			// else can it do?
			VerackReceived = true;
		} else {
			// Odd, we shouldn't get anything but a version message from
			// a newly connected peer.
			if( Message.get() == NULL ) {
				log() << "[PEER] " << incoming.size() << " bytes left pending" << endl;
			} else {
				log() << "[PEER] Ignoring " << *Message.get() << endl;
			}
		}

		if( VerackReceived && VersionSent ) {
			log() << "[PEER] Handshake complete, setting state to 'Connected'" << endl;
			State = Connected;
		}
	}

	if( State == Connected ) {
//		log() << "[PEER] State: Connected" << endl;
		if( Factory == NULL )
			throw logic_error( "TBitcoinPeer::receive() must have factory in connected mode" );

		try {
			Factory->receive(incoming);
		} catch( exception &e ) {
			log() << "[PEER] Error ";
			TLog::hexify( log(), incoming );
			log() << endl;
			log() << "[PEER] Error parsing message, " << e.what() << endl;
			return;
		}

		auto_ptr<TMessage> Message;

		do {
			Message.reset( nextIncoming() );
			if( Message.get() == NULL )
				break;

			// The network gets to process the packets once we're
			// connected
			Network->process( Message.get() );

		} while( true );
	}

}


// -------------- Class member definitions


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include <sstream>
#include "unittest.h"
#include "logstream.h"
#include "bitcoinnetwork.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
		TBitcoinNetwork Network;
		TBitcoinPeer Peer( NULL, &Network );
		Peer.setState( TBitcoinPeer::Connecting );

		const string *p = UNITTESTSampleMessages;
		while( !p->empty() ) {
			// Fake reception of partial messages
			for( unsigned int i = 0; i < p->size(); i += 1000 ) {
				log() << "[TEST] RX< " << p->size() << " bytes @ " << i << endl;
				Peer.receive( p->size() - i < 200
						? *p
						: p->substr(i,200) );
			}
			p++;

			ostringstream oss;
			TMessage *out = Peer.nextOutgoing();
			if( out != NULL ) {
				out->write( oss );
				log() << "[TEST] TX> " << oss.str().size() << endl;
			}
			delete out;
		}

	} catch( std::exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

