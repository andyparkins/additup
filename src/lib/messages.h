// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   messages.h
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
#ifndef MESSAGES_H
#define MESSAGES_H

// -------------- Includes
// --- C
#include <stdint.h>
// --- C++
#include <string>
#include <list>
#include <iostream>
#include <stdexcept>
// --- OS
// --- Project
#include "structures.h"


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


// -------------- Function pre-class prototypes


// -------------- Class declarations

//
// Class: message_parse_error
// Description:
//
class message_parse_error : public runtime_error
{
  public:
	explicit message_parse_error( const string &s ) :
		runtime_error(s) {}
};

class message_parse_error_type : public message_parse_error
{
  public:
	message_parse_error_type() :
		message_parse_error("message type wrong") {}
};

class message_parse_error_checksum : public message_parse_error
{
  public:
	message_parse_error_checksum() :
		message_parse_error("checksum") {}
};

class message_parse_error_underflow : public message_parse_error
{
  public:
	message_parse_error_underflow() :
		message_parse_error("not enough data for message to be parsed") {}
};

class message_parse_error_version : public message_parse_error
{
  public:
	message_parse_error_version() :
		message_parse_error("message version too old") {}
};

//
// Class: TMessage
// Description:
//
class TMessage
{
  public:
	TMessage();
	virtual const char *className() const { return "TMessage"; }
	virtual TMessage *clone() const = 0;

	unsigned int queryMessageExtractSize( const string & );

	virtual void take( string & );
	virtual string give() const = 0;

	const sMessageHeader &header() const { return MessageHeader; }

  protected:
	virtual bool headerHasPayloadChecksum() const { return true; }

	virtual const char *commandString() const = 0;

	virtual ostream &printOn( ostream & ) const;
	friend ostream &operator<<( ostream &, const TMessage & );

  protected:
	sMessageHeader MessageHeader;
	string RawPayload;
};

//
// Class:	TMessageAutoSizeField
// Description:
//
// Numeric Value     Data Size Required    Format
// < 253             1 byte                < data >
// <= USHRT_MAX      3 bytes               253 + <data> (as ushort datatype)
// <= UINT_MAX       5 bytes               254 + <data> (as uint datatype)
// size > UINT_MAX   9 bytes               255 + <data>
//
// XXX: WTF? On what planet are we going to send multi-gigabyte sized
// fields (32-bit) let alone 64-bit sized fields.  It's also not like
// the rest of the protocol has tried hard to pack the bits together.
// Wouldn't it have been easier just to make these variable sized count
// fields a single 32 bit number?
//
class TMessageAutoSizeField
{
  public:
	TMessageAutoSizeField();

	unsigned int queryMessageExtractSize( const string & );
	void take( string );
	string give() const;

	uint64_t getSize() const;
	void setSize( uint64_t s ) { Size = s; }

  protected:
	uint64_t Size;
};

//
// Class: TMessage_version
// Description:
// The first packet type transmitted between nodes upon connecting to
// each other shall be the version packet, and all other packet shall be
// rejected by the node from that connection until a successful version
// packet is transmitted. This packet contains information about what
// version of the network and client are being used by a particular node
// and contains information about how nodes will exchanging data. Nodes
// may adjust their communications between each other based upon the
// packet type, or reject all further communications with that node if
// versions are incompatible with each other.
//
// Upon successful reception of a Version packet, the VerAck packet shall
// be sent, to nodes with version >= 209.
//
class TMessage_version : public TMessage
{
  public:
	const char *className() const { return "TMessage_version"; }

  protected:
	bool headerHasPayloadChecksum() const { return false; }
	const char *commandString() const { return "version"; }

  protected:

	struct {
		uint32_t Version;
		uint64_t Services;
		uint64_t Timestamp;
		// XXX: Why does sAddressData have a services field _and_ the
		// version message have a services field?
		sAddressData AddrMe;
		// Version >= 106
		sAddressData AddrFrom;
		uint64_t Nonce;
		string SubVersionNum;
		// Version >= 209
		uint32_t StartingHeight;
	} Payload;
};

//
// Class: TMessage_version_0
// Description:
//
class TMessage_version_0 : public TMessage_version
{
  public:
	const char *className() const { return "TMessage_version_0"; }
	TMessage *clone() const { return new TMessage_version_0(*this); }

  protected:
};

//
// Class: TMessage_version_106
// Description:
//
class TMessage_version_106 : public TMessage_version_0
{
  public:
	const char *className() const { return "TMessage_version_106"; }
	TMessage *clone() const { return new TMessage_version_106(*this); }

  protected:
};

//
// Class: TMessage_version_209
// Description:
//
class TMessage_version_209 : public TMessage_version_106
{
  public:
	const char *className() const { return "TMessage_version_209"; }
	TMessage *clone() const { return new TMessage_version_209(*this); }

  protected:
};

//
// Class: TMessage_verack
// Description:
// The verack message is sent in response to a version message,
// confirming that the sending node has been accepted as a peer and that
// the Bitcoin version is compatible with what that node is expecting in
// terms of protocol.
//
// No data is expected with this packet, but it gives information that a
// version packet has been successfully transmitted to a node.
//
//
class TMessage_verack : public TMessage
{
  public:
	const char *className() const { return "TMessage_verack"; }
	TMessage *clone() const { return new TMessage_verack(*this); }

  protected:
	bool headerHasPayloadChecksum() const { return false; }
	const char *commandString() const { return "verack"; }

  protected:
};

//
// Class: TMessage_addr
// Description:
// For nodes implementing protocol versions prior to 209 (as contained
// in the Version data listed above) all further data is ignored and
// only one data packet is scanned.
//
// If the size of all of the address data is greater than 1000 bytes, an
// error condition is assumed.
//
class TMessage_addr : public TMessage
{
  public:
	const char *className() const { return "TMessage_addr"; }
	TMessage *clone() const { return new TMessage_addr(*this); }

  protected:
	const char *commandString() const { return "addr"; }

  protected:
	struct {
		list<sAddressData> AddressData;
	} Payload;
};

//
// Class: TMessage_InventoryBase
// Description:
//
class TMessage_InventoryBase : public TMessage
{
  public:
	const char *className() const { return "TMessage_InventoryBase"; }

  protected:
	struct {
		TMessageAutoSizeField Count;
		list<sInventoryVector> InventoryVector;
	} Payload;
};

//
// Class: TMessage_inv
// Description:
//
class TMessage_inv : public TMessage_InventoryBase
{
  public:
	const char *className() const { return "TMessage_inv"; }
	TMessage *clone() const { return new TMessage_inv(*this); }

  protected:
	const char *commandString() const { return "inv"; }

  protected:
};

//
// Class: TMessage_getdata
// Description:
//
class TMessage_getdata : public TMessage_InventoryBase
{
  public:
	const char *className() const { return "TMessage_getdata"; }
	TMessage *clone() const { return new TMessage_getdata(*this); }

  protected:
	const char *commandString() const { return "getdata"; }

  protected:
};

//
// Class: TMessage_getblocks
// Description:
//
class TMessage_getblocks : public TMessage
{
  public:
	const char *className() const { return "TMessage_getblocks"; }
	TMessage *clone() const { return new TMessage_getblocks(*this); }

  protected:
	const char *commandString() const { return "getblocks"; }

  protected:
	struct {
		uint32_t Version;
		TMessageAutoSizeField HashStartCount;
		list<sHash> HashStart;
		sHash HashStop;
	} Payload;
};

//
// Class: TMessage_tx
// Description:
// The tx message is sent in response to a getdata message which
// requests transaction information from a transaction hash.
//
class TMessage_tx : public TMessage
{
  public:
	const char *className() const { return "TMessage_tx"; }
	TMessage *clone() const { return new TMessage_tx(*this); }

  protected:
	const char *commandString() const { return "tx"; }

  protected:
	struct sTransactionOutputReference {
		sHash Hash;
		uint32_t Index;
	};
	struct sScriptElement {
	};
	struct sInputTransaction {
		sTransactionOutputReference PreviousOut;
		list<sScriptElement> Signature;
		uint32_t Sequence;
	};
	struct sOutputTransaction {
		uint64_t Value;
		list<sScriptElement> PublicKeyScript;
	};

	struct {
		uint8_t Version;
		list<sInputTransaction> TxIn;
		list<sOutputTransaction> TxOut;
		uint32_t LockTime;
	} Payload;
};

//
// Class: TMessage_block
// Description:
// The block message is sent in response to a getdata message which
// requests transaction information from a block hash.
//
class TMessage_block : public TMessage
{
  public:
	const char *className() const { return "TMessage_block"; }
	TMessage *clone() const { return new TMessage_block(*this); }

  protected:
	const char *commandString() const { return "block"; }

  protected:
	struct {
		uint32_t Version;
		sHash PreviousBlock;
		sHash MerkleRoot;
		uint32_t Time;
		uint32_t DifficultyBits;
		uint32_t Nonce;
	} Payload;
};

//
// Class: TMessage_getaddr
// Description:
// The getaddr message sends a request to a node asking for information
// about known active peers to help with identifying potential nodes in
// the network. The response to receiving this message is to transmit an
// addr message with one or more peers from a database of known active
// peers. The typical presumption is that a node is likely to be active
// if it has been sending a message within the last three hours.
//
// No additional data is transmitted with this message.
//
//
class TMessage_getaddr : public TMessage
{
  public:
	const char *className() const { return "TMessage_getaddr"; }
	TMessage *clone() const { return new TMessage_getaddr(*this); }

  protected:
	const char *commandString() const { return "getaddr"; }

  protected:
};

//
// Class: TMessage_checkorder
// Description:
//
class TMessage_checkorder : public TMessage
{
  public:
	const char *className() const { return "TMessage_checkorder"; }
	TMessage *clone() const { return new TMessage_checkorder(*this); }

  protected:
	const char *commandString() const { return "checkorder"; }

  protected:
};

//
// Class: TMessage_submitorder
// Description:
//
class TMessage_submitorder : public TMessage
{
  public:
	const char *className() const { return "TMessage_submitorder"; }
	TMessage *clone() const { return new TMessage_submitorder(*this); }

  protected:
	const char *commandString() const { return "submitorder"; }

  protected:
};

//
// Class: TMessage_reply
// Description:
//
class TMessage_reply : public TMessage
{
  public:
	const char *className() const { return "TMessage_reply"; }
	TMessage *clone() const { return new TMessage_reply(*this); }

  protected:
	const char *commandString() const { return "reply"; }

  protected:
};

//
// Class: TMessage_ping
// Description:
//
class TMessage_ping : public TMessage
{
  public:
	const char *className() const { return "TMessage_ping"; }
	TMessage *clone() const { return new TMessage_ping(*this); }

  protected:
	const char *commandString() const { return "ping"; }

  protected:
};

//
// Class: TMessage_alert
// Description:
// An alert is sent between nodes to send a general notification message
// throughout the network. If the alert can be confirmed with the
// signature as having come from the the core development group of the
// Bitcoin software, the message is suggested to be displayed for
// end-users. Attempts to perform transactions, particularly automated
// transactions through the client, are suggested to be halted. The text
// in the Message string should be relayed to log files and any user
// interfaces.
//
// (see also http://www.bitcoin.org/smf/index.php?topic=898.0 )
//
class TMessage_alert : public TMessage
{
  public:
	const char *className() const { return "TMessage_alert"; }
	TMessage *clone() const { return new TMessage_alert(*this); }

  protected:
	const char *commandString() const { return "alert"; }

  protected:
	struct {
		string Message;
		string Signature;
	} Payload;

	static const string ALERT_VERIFICATION_KEYS[];
};


// -------------- Constants


// -------------- Inline Functions
inline ostream &operator<<(ostream &s, const TMessage &M ) { return M.printOn(s); }


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)

// End of conditional compilation
#endif
