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

	return 0;
}
#endif

