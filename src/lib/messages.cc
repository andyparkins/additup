// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   messages.cc
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
#include "messages.h"

// -------------- Includes
// --- C
// --- C++
#include <list>
// --- Qt
// --- OS
// --- Project libs
// --- Project


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Template instantiations


// -------------- Class declarations

//
// Class: TMessageTemplates
// Description:
// Master message list.
//
class TMessageTemplates
{
  public:
	TMessageTemplates();
	~TMessageTemplates();

	typedef list<const TMessage *> container;

	container Templates;

	static TMessageTemplates t;
};

TMessageTemplates TMessageTemplates::t;


// -------------- Class member definitions

//
// Function:	TMessageTemplates :: TMessageTemplates
// Description:
//
TMessageTemplates::TMessageTemplates()
{
	static const TMessage *ModuleTemplates[] = {
		// Note: version messages have to be in reverse order so that the
		// highest matching version will be tried first.  This is
		// necessary because version_0 will happilly accept a
		// version_209 message, being that it is backwards compatible
		new TMessage_version_209(),
		new TMessage_version_106(),
		new TMessage_version_0(),
		new TMessage_verack(),
		new TMessage_addr(),
		new TMessage_inv(),
		new TMessage_getdata(),
		new TMessage_getblocks(),
		new TMessage_tx(),
		new TMessage_block(),
		new TMessage_getaddr(),
		new TMessage_checkorder(),
		new TMessage_submitorder(),
		new TMessage_reply(),
		new TMessage_ping(),
		new TMessage_alert(),
		// Note: The unimplemented type should always be tried last
		new TMessageUnimplemented(),
		NULL
	};

	// ---
	const TMessage **p = ModuleTemplates;

	// Insert each template message into the master list
	while( *p != NULL ) {
		Templates.push_back( *p );
		p++;
	}
}

//
// Function:	TMessageTemplates :: ~TMessageTemplates
// Description:
//
TMessageTemplates::~TMessageTemplates()
{
}

// --------

//
// Function:	TMessageAutoSizeInteger :: TMessageAutoSizeInteger
// Description:
//
TMessageAutoSizeInteger::TMessageAutoSizeInteger()
{
}

// --------

//
// Function:	TMessage :: TMessage
// Description:
//
TMessage::TMessage() :
	PayloadAccepted(0)
{
}

//
// Function:	TMessage :: queryMessageExtractSize
// Description:
//
uint32_t TMessage::queryMessageExtractSize( const string &d ) const
{
	if( d.size() < 20 )
		return 0;

	// Four bytes little endian magic (d0-d3)

	if( !acceptCommandCode( d.substr(4,12) ) )
		return 0;

	// Four bytes length (d16-d19)
	return littleEndian32FromString(d, 16) + 20;
}

//
// Function:	TMessage :: acceptCommandCode
// Description:
//
bool TMessage::acceptCommandCode( const string &d ) const
{
	// Twelve bytes command code (d4-d15)
	string headerCommandCode = d;
	// Strip NUL bytes
	headerCommandCode.erase( headerCommandCode.find_last_not_of('\0') + 1 );

	if( headerCommandCode != commandString() )
		return false;

	return true;
}

//
// Function:	TMessage :: parse
// Description:
//
void TMessage::parse( const string &d )
{
	// Don't try and extract more data than is available
	if( d.size() < 20 )
		throw message_parse_error_underflow();

	MessageHeader.Magic = littleEndian32FromString( d, 0 );
	MessageHeader.Command = d.substr(4,12);
	MessageHeader.PayloadLength = littleEndian32FromString( d, 16 );
}

//
// Function:	TMessage :: printOn
// Description:
//
ostream &TMessage::printOn( ostream &s ) const
{
	s << className();
	return s;
}

// --------

//
// Function:	TMessageWithChecksum :: queryMessageExtractSize
// Description:
//
uint32_t TMessageWithChecksum::queryMessageExtractSize( const string &d ) const
{
	uint32_t x = TMessage::queryMessageExtractSize(d);

	// We have a checksum, so add it on
	x += 4;

	// Check for overflow (this will also check for a zero returned from
	// the base class)
	if( x < 24 )
		return 0;

	return x;
}

//
// Function:	TMessageWithChecksum :: parse
// Description:
//
void TMessageWithChecksum::parse( const string &d )
{
	TMessage::parse(d);

	if( d.size() < 24 )
		throw message_parse_error_underflow();
	MessageHeader.Checksum = littleEndian32FromString( d, 20 );

	// Don't try and extract more data than is available
	if( MessageHeader.PayloadLength > d.size() - 24 )
		throw message_parse_error_underflow();

	// Pull the payload out
	RawPayload = d.substr(24, MessageHeader.PayloadLength);
	// TMessage parses none of the payload, so we point at zero
	PayloadAccepted = 0;
}

//
// Function:	TMessageWithoutChecksum :: queryMessageExtractSize
// Description:
//
uint32_t TMessageWithoutChecksum::queryMessageExtractSize( const string &d ) const
{
	uint32_t x = TMessage::queryMessageExtractSize(d);

	// Check for overflow (this will also check for a zero returned from
	// the base class)
	if( x < 20 )
		return 0;

	return x;
}

//
// Function:	TMessageWithoutChecksum :: parse
// Description:
//
void TMessageWithoutChecksum::parse( const string &d )
{
	TMessage::parse(d);

	if( d.size() < 20 )
		throw message_parse_error_underflow();
	MessageHeader.Checksum = 0;

	// Don't try and extract more data than is available
	if( MessageHeader.PayloadLength > d.size() - 20 )
		throw message_parse_error_underflow();

	// Pull the payload out
	RawPayload = d.substr(20, MessageHeader.PayloadLength);
	// TMessage parses none of the payload, so we point at zero
	PayloadAccepted = 0;
}

// --------

//
// Function:	TMessage_version :: parse
// Description:
//
ostream &TMessage_version::printOn( ostream &s ) const
{
	TMessage::printOn(s);
	s << " { Version = " << Payload.Version
		<< "; Services = ["
		<< (Payload.Services & sAddressData::NODE_NETWORK ? " NODE_NETWORK" : "" )
		<< " ]; Time = " << Payload.Timestamp
		<< "; SenderAddress = ";
	if( Payload.Version >= 106 ) {
		s << "; ReceiverAddress = "
			<< "; Nonce = " << Payload.Nonce
			<< "; SubVersion = \"" << Payload.SubVersionNum << "\"";
	}
	if( Payload.Version >= 209 ) {
		s << "; Height = " << Payload.StartingHeight;
	}

	s << " }";
	return s;
}

//
// Function:	TMessage_version :: parse
// Description:
//
void TMessage_version::parse( const string &d )
{
	TMessageWithoutChecksum::parse(d);

	// Clear everything
	Payload.Version = 0;
	Payload.Services = 0;
	Payload.Timestamp = 0;
//	Payload.AddrMe.clear();
//	Payload.AddrFrom.clear();
	Payload.Nonce = 0;
	Payload.SubVersionNum.clear();
	Payload.StartingHeight = 0;
}

//
// Function:	TMessage_version_0 :: parse
// Description:
//
void TMessage_version_0::parse( const string &d )
{
	TMessage_version::parse(d);

	if( RawPayload.size() < 46 )
		throw message_parse_error_underflow();

	// d0
	Payload.Version = littleEndian32FromString(RawPayload,0);

	if( Payload.Version < minimumAcceptedVersion() )
		throw message_parse_error_version();

	Payload.Services = littleEndian64FromString(RawPayload,4);
	Payload.Timestamp = littleEndian64FromString(RawPayload,12);

	// d20
	Payload.AddrMe;
	PayloadAccepted = 46;
}

//
// Function:	TMessage_version_106 :: parse
// Description:
//
void TMessage_version_106::parse( const string &d )
{
	TMessage_version_0::parse(d);

	if( RawPayload.size() < PayloadAccepted + 34 + 1 )
		throw message_parse_error_underflow();

	// d46
	Payload.AddrFrom;

	// d72
	Payload.Nonce = littleEndian64FromString(RawPayload,72);
	PayloadAccepted = 80;

	// d80: Variable sized NUL-terminated string
	PayloadAccepted += NULTerminatedString(Payload.SubVersionNum, RawPayload, 80);
}

//
// Function:	TMessage_version_209 :: parse
// Description:
//
void TMessage_version_209::parse( const string &d )
{
	TMessage_version_106::parse(d);

	if( RawPayload.size() < PayloadAccepted + 4 )
		throw message_parse_error_underflow();

	// Version >= 209
	Payload.StartingHeight = littleEndian32FromString(RawPayload,PayloadAccepted);
	PayloadAccepted += 4;
}

// --------

const string TMessage_alert::ALERT_VERIFICATION_KEYS[] = {
	string("04fc9702847840aaf195de8442ebeced"
	"f5b095cdbb9bc716bda9110971b28a49"
	"e0ead8564ff0db22209e0374782c093b"
	"b899692d524e9d6a6956e7c5ecbcd68284"),
	string("")
	};



// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
		TMessageTemplates::container::iterator it;

		cerr << "--- Available TMessage templates" << endl;

		for( it = TMessageTemplates::t.Templates.begin();
				it != TMessageTemplates::t.Templates.end(); it++ ) {
			cerr << (*it)->className() << endl;
		}
	} catch( exception &e ) {
		cerr << e.what() << endl;
		return 255;
	}

	try {
		static const string SampleMessages[] = {
			// TMessage_version_0
			string("\xf9\xbe\xb4\xd9"    // Magic
					"unimplement\0"      // Command
					"\0\0\0\0"         // Length
					, 20 ),
			// TMessage_version_209
			string("\xf9\xbe\xb4\xd9"    // Magic
					"version\0\0\0\0\0"  // Command
					"\x55\0\0\0"         // Length
					"\xd1\x00\x00\x00"   // Version
					"\x01\0\0\0\0\0\0\0" // Services
					"\0\0\0\x80\0\0\0\0" // Timestamp in seconds
					"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" // Address Me
					"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" // Address You
					"\0\0\0\0\0\0\0\0"   // Nonce
					"\0"                 // Sub version information (NUL terminated)
					"\0\0\0\0"           // Start height
					, 105 ),
			// TMessage_version_106
			string("\xf9\xbe\xb4\xd9"    // Magic
					"version\0\0\0\0\0"  // Command
					"\x51\0\0\0"         // Length
					"\x6a\x00\x00\x00"   // Version
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
			// addr
			string(
				"\xF9\xBE\xB4\xD9\x61\x64\x64\x72\x00\x00\x00\x00\x00\x00\x00\x00"
				"\x1F\x00\x00\x00\x7F\x85\x39\xC2\x01\xE2\x15\x10\x4D\x01\x00\x00"
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
		TMessageTemplates::container::iterator it;

		cerr << "--- Testing parser" << endl;

		const string *p = SampleMessages;
		while( !p->empty() ) {
			TMessage *potential = NULL;

			for( it = TMessageTemplates::t.Templates.begin();
					it != TMessageTemplates::t.Templates.end(); it++ ) {
				if( (*it)->queryMessageExtractSize( *p ) == 0 ) {
					continue;
				}
				potential = (*it)->clone();
				try {
					potential->parse( *p );
				} catch( exception &e ) {
					cerr << p << " message parse by " << potential->className()
						<< " failed, " << e.what() << endl;
					delete potential;
					potential = NULL;
					continue;
				}
				break;
			}
			if( it != TMessageTemplates::t.Templates.end() ) {
				cerr << "Sample message " << p << " is a " << *potential << endl;
			} else {
				cerr << "Sample message " << p << " is not understood" << endl;
			}
			delete potential;

			p++;
		}
	} catch( exception &e ) {
		cerr << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

