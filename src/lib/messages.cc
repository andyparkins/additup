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

	// Twelve bytes command code (d4-d15)
	string headerCommandCode = d.substr(4,12);
	// Strip NUL bytes
	headerCommandCode.erase( headerCommandCode.find_last_not_of('\0') + 1 );

	if( headerCommandCode != commandString() )
		return 0;

	// Four bytes length (d16-d19)
	uint32_t x = littleEndian32FromString(d, 16);

	// Include header
	x += 4 + 12 + 4;
	if( headerHasPayloadChecksum() )
		x += 4;

	// Check for overflow
	if( x < (headerHasPayloadChecksum() ? 24 : 20) )
		return 0;

	return x;
}

//
// Function:	TMessage :: parse
// Description:
//
void TMessage::parse( const string &d )
{
	string::size_type pos;

	// Don't try and extract more data than is available
	if( d.size() < 20 )
		throw message_parse_error_underflow();

	MessageHeader.Magic = littleEndian32FromString( d, 0 );
	MessageHeader.Command = d.substr(4,12);
	MessageHeader.PayloadLength = littleEndian32FromString( d, 16 );

	if( headerHasPayloadChecksum() ) {
		pos = 24;
		if( d.size() < pos )
			throw message_parse_error_underflow();
		MessageHeader.Checksum = littleEndian32FromString( d, 20 );
	} else {
		pos = 20;
		if( d.size() < pos )
			throw message_parse_error_underflow();
		MessageHeader.Checksum = 0;
	}

	// Don't try and extract more data than is available
	if( MessageHeader.PayloadLength > d.size() - pos )
		throw message_parse_error_underflow();

	// Pull the payload out
	RawPayload = d.substr(pos, MessageHeader.PayloadLength);
	// TMessage parses none of the payload, so we point at zero
	PayloadAccepted = 0;
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

