// ----------------------------------------------------------------------------
// Project: bitcoin
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
	Initialised( false )
{
}

//
// Function:	TMessageFactory :: init
// Description:
//
void TMessageFactory::init()
{
	Templates.push_back( new TMessage_version_20900() );
	Templates.push_back( new TMessage_version_10600() );
	Templates.push_back( new TMessage_version_0() );
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
//
void TMessageFactory::receive( const string &s )
{
	list<const TMessage*>::const_iterator it;
	RXBuffer += s;
	istringstream iss( RXBuffer );
	iss.exceptions( ios::eofbit | ios::failbit | ios::badbit );

	TMessage *potential = NULL;

	ios::streampos sp;

	// Test against each template message
	for( it = Templates.begin(); it != Templates.end(); it++ ) {

		// Clone the template
		auto_ptr<TMessage> p( (*it)->clone() );

//		cerr << "Trying " << RXBuffer.size() << " bytes with "
//			<< potential->className();

		try {
			// Store the current position
			sp = iss.tellg();

			p->read( iss );
//			cerr << "*" << endl;
			potential = p.get();
			p.release();
			break;

		} catch( ios::failure &e ) {
//			cerr << " - " << e.what() << endl;
			// If we run out of message from the source, then leave,
			// hoping for more
			break;

		} catch( message_parse_error_underflow &e ) {
//			cerr << " - " << e.what() << endl;
			// If we run out of message from the source, then leave,
			// hoping for more
			break;

		} catch( message_parse_error_version &e ) {
//			cerr << " - " << e.what() << endl;
			// Try next template with the same data
			iss.seekg( sp, ios::beg );

		} catch( message_parse_error_type &e ) {
//			cerr << " - " << e.what() << endl;
			// Try next template with the same data
			iss.seekg( sp, ios::beg );

		} catch( message_parse_error &e ) {
//			cerr << " - " << e.what() << endl;

			// Anything else, chuck the packet away
			sp = iss.tellg();
			RXBuffer = RXBuffer.substr( sp, RXBuffer.size() - sp );
			break;
		}
	}

	if( potential != NULL ) {
//		cerr << *potential << endl;
		// Push it onto the receive queue
		IncomingQueue.push_back( potential );
		// Remove the bytes from the RX buffer
		sp = potential->getMessageSize();
		RXBuffer = RXBuffer.substr( sp, RXBuffer.size() - sp );
	}
}


////
//// Function:	TMessageFactory :: answer
//// Description:
////
//TMessage *TMessageFactory::answer( TMessage *Message )
//{
//	if( dynamic_cast<TMessageUnimplemented>( Message ) != NULL ) {
//		// No response
//	} else if( dynamic_cast<TMessage_version_209>( Message ) != NULL ) {
//	} else if( dynamic_cast<TMessage_version_106>( Message ) != NULL ) {
//	} else if( dynamic_cast<TMessage_version_0>( Message ) != NULL ) {
//	}
//}


// -------------- Class member definitions


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
		static const string SampleMessages[] = {
			// Short invalid message
//			string("\xf9\xbe\xb4\xd9"    // Magic
//					"unimplement\0"      // Command
//					, 16 ),
//			// TMessageUnimplemented
//			string("\xf9\xbe\xb4\xd9"    // Magic
//					"unimplement\0"      // Command
//					"\0\0\0\0"         // Length
//					, 20 ),
			// TMessage_version_20900
			string("\xf9\xbe\xb4\xd9"    // Magic
					"version\0\0\0\0\0"  // Command
					"\x55\0\0\0"         // Length
					"\xa4\x51\x00\x00"   // Version
					"\x01\0\0\0\0\0\0\0" // Services
					"\0\0\0\x80\0\0\0\0" // Timestamp in seconds
					"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" // Address Me
					"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" // Address You
					"\0\0\0\0\0\0\0\0"   // Nonce
					"\0"                 // Sub version information (NUL terminated)
					"\0\0\0\0"           // Start height
					, 105 ),
			// TMessage_version_10600
			string("\xf9\xbe\xb4\xd9"    // Magic
					"version\0\0\0\0\0"  // Command
					"\x51\0\0\0"         // Length
					"\x68\x29\x00\x00"   // Version
					"\x01\0\0\0\0\0\0\0" // Services
					"\0\0\0\x80\0\0\0\0" // Timestamp in seconds
					"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" // Address Me
					"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" // Address You
					"\0\0\0\0\0\0\0\0"   // Nonce
					"\0"                 // Sub version information (NUL terminated)
					, 101 ),
			// TMessage_version_0
			string("\xf9\xbe\xb4\xd9"    // Magic
					"version\0\0\0\0\0"  // Command
					"\x2e\0\0\0"         // Length
					"\x00\x00\x00\x00"   // Version
					"\x01\0\0\0\0\0\0\0" // Services
					"\0\0\0\x80\0\0\0\0" // Timestamp in seconds
					"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" // Address Me
					, 66 ),
			// --- From https://en.bitcoin.it/wiki/Protocol_specification
			// version
			string(
				"\xf9\xbe\xb4\xd9\x76\x65\x72\x73\x69\x6f\x6e\x00\x00\x00\x00\x00"
				"\x55\x00\x00\x00\x9c\x7c\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
				"\xe6\x15\x10\x4d\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
				"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\x0a\x00\x00\x01"
				"\xda\xf6\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"\x00\x00\x00\x00\xff\xff\x0a\x00\x00\x02\x20\x8d\xdd\x9d\x20\x2c"
				"\x3a\xb4\x57\x13\x00\x55\x81\x01\x00"
				, 105 ),
			// verack
			string(
				"\xf9\xbe\xb4\xd9\x76\x65\x72\x61\x63\x6b\x00\x00\x00\x00\x00\x00"
				"\x00\x00\x00\x00"
				, 20 ),
			// addr (with checksum corrected from 0xc239857f to 0x9b3952ed)
			string(
				"\xF9\xBE\xB4\xD9\x61\x64\x64\x72\x00\x00\x00\x00\x00\x00\x00\x00"
				"\x1F\x00\x00\x00\xed\x52\x39\x9b\x01\xE2\x15\x10\x4D\x01\x00\x00"
				"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xFF"
				"\xFF\x0A\x00\x00\x01\x20\x8D"
				, 55 ),
			// tx
			string(
				"\xF9\xBE\xB4\xD9\x74\x78\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"\x02\x01\x00\x00\xE2\x93\xCD\xBE\x01\x00\x00\x00\x01\x6D\xBD\xDB"
				"\x08\x5B\x1D\x8A\xF7\x51\x84\xF0\xBC\x01\xFA\xD5\x8D\x12\x66\xE9"
				"\xB6\x3B\x50\x88\x19\x90\xE4\xB4\x0D\x6A\xEE\x36\x29\x00\x00\x00"
				"\x00\x8B\x48\x30\x45\x02\x21\x00\xF3\x58\x1E\x19\x72\xAE\x8A\xC7"
				"\xC7\x36\x7A\x7A\x25\x3B\xC1\x13\x52\x23\xAD\xB9\xA4\x68\xBB\x3A"
				"\x59\x23\x3F\x45\xBC\x57\x83\x80\x02\x20\x59\xAF\x01\xCA\x17\xD0"
				"\x0E\x41\x83\x7A\x1D\x58\xE9\x7A\xA3\x1B\xAE\x58\x4E\xDE\xC2\x8D"
				"\x35\xBD\x96\x92\x36\x90\x91\x3B\xAE\x9A\x01\x41\x04\x9C\x02\xBF"
				"\xC9\x7E\xF2\x36\xCE\x6D\x8F\xE5\xD9\x40\x13\xC7\x21\xE9\x15\x98"
				"\x2A\xCD\x2B\x12\xB6\x5D\x9B\x7D\x59\xE2\x0A\x84\x20\x05\xF8\xFC"
				"\x4E\x02\x53\x2E\x87\x3D\x37\xB9\x6F\x09\xD6\xD4\x51\x1A\xDA\x8F"
				"\x14\x04\x2F\x46\x61\x4A\x4C\x70\xC0\xF1\x4B\xEF\xF5\xFF\xFF\xFF"
				"\xFF\x02\x40\x4B\x4C\x00\x00\x00\x00\x00\x19\x76\xA9\x14\x1A\xA0"
				"\xCD\x1C\xBE\xA6\xE7\x45\x8A\x7A\xBA\xD5\x12\xA9\xD9\xEA\x1A\xFB"
				"\x22\x5E\x88\xAC\x80\xFA\xE9\xC7\x00\x00\x00\x00\x19\x76\xA9\x14"
				"\x0E\xAB\x5B\xEA\x43\x6A\x04\x84\xCF\xAB\x12\x48\x5E\xFD\xA0\xB7"
				"\x8B\x4E\xCC\x52\x88\xAC\x00\x00\x00\x00"
				, 282 ),
			string()
		};
		TMessageFactory PF;
		PF.init();


		const string *p = SampleMessages;
		while( !p->empty() ) {
			PF.receive( *p );
			if( PF.newestIncoming() != NULL ) {
				cerr << "PF.queue() = " << *PF.newestIncoming() << endl;
			} else {
				cerr << "no packet yet" << endl;
			}
			p++;
		}

	} catch( exception &e ) {
		cerr << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

