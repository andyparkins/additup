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
	State( Unconnected )
{
}

//
// Function:	TBitcoinPeer :: ~TBitcoinPeer
// Description:
//
TBitcoinPeer::~TBitcoinPeer()
{
	delete Factory;
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
// Function:	TBitcoinPeer :: nextOutgoing
// Description:
//
TMessage *TBitcoinPeer::nextOutgoing()
{
	if( Factory == NULL )
		return NULL;
	return Factory->nextOutgoing();
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
void TBitcoinPeer::receive( const string &s )
{
	if( State == Unconnected ) {
		log() << "[PEER] State: Unconnected" << endl;
		// Can't receive anything until we at least have comms
		return;
	}

	if( State == Connecting ) {
		log() << "[PEER] State: Connecting" << endl;
		if( Factory != NULL )
			throw logic_error( "TBitcoinPeer::receive() can't have a factory while unconnected" );

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

			// We will make the assumption that the accidental transmission
			// of bytes matching a network magic number is impossible.
			// Therefore we'll look through a window until we see a match.

			// Try reading from each byte in turn
			istringstream iss(s);
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

		// Spontaneous queue is answer(NULL);
		Factory->answer(NULL);
		Factory->receive(s);

		auto_ptr<TMessage> Message( Factory->nextIncoming() );
		Factory->answer( Message.get() );

		if( dynamic_cast<TMessage_version*>( Message.get() ) != NULL ) {
			// We don't care exactly what version message, the
			// TMessage_version base class is more than enough for us to
			// query the message
			TMessage_version *VersionMessage = dynamic_cast<TMessage_version*>( Message.get() );

			log() << "[PEER] Version message received, " << *VersionMessage << endl;

			delete Factory;
			Factory = VersionMessage->createMessageFactory();

			State = Connected;

			log() << "[PEER] Factory is now " << Factory->className() << endl;
		} else if( dynamic_cast<TMessage_verack*>( Message.get() ) != NULL ) {
			// Not sure we care...  If we don't get a verack, then
			// presumably the remote will just hang up on us -- what
			// else can it do?
		} else {
			// Odd, we shouldn't get anything but a version message from
			// a newly connected peer.
			if( Message.get() == NULL ) {
				log() << "[PEER] Ignoring " << s.size() << " junk bytes" << endl;
			} else {
				log() << "[PEER] Ignoring " << *Message.get() << endl;
			}
		}
	}

	// If we've been handshaking, then the version message is still in
	// the string, which is just what we want, as it will initialise
	// the factory, and create the verack response

	if( State == Connected ) {
		log() << "[PEER] State: Connected" << endl;
		if( Factory == NULL )
			throw logic_error( "TBitcoinPeer::receive() must have factory in connected mode" );

		// Spontaneous queue is answer(NULL);
		Factory->answer(NULL);
		Factory->receive(s);

		auto_ptr<TMessage> Message;

		do {
			Message.reset( Factory->nextIncoming() );
			if( Message.get() == NULL )
				break;
			Factory->answer( Message.get() );

			log() << "[PEER] Got message " << *Message << endl;

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
			log() << "[TEST] RX< " << p->size() << " bytes" << endl;
			Peer.receive( *p );
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

