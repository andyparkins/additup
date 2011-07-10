// ----------------------------------------------------------------------------
// Project: additup
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
#include "messageelements.h"


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
class TMessageDigest;
class TVersionedMessageFactory;
class TBitcoinPeer;


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
		message_parse_error("data too short for message") {}
};

class message_parse_error_version : public message_parse_error
{
  public:
	message_parse_error_version() :
		message_parse_error("message version too old") {}
};

class message_parse_error_magic : public message_parse_error
{
  public:
	message_parse_error_magic() :
		message_parse_error("message magic is wrong") {}
};

//
// Class: TMessage
// Description:
// BitCoin network message encapsulation.
//
// https://en.bitcoin.it/wiki/Protocol_specification
//
//
class TMessage
{
  public:
	TMessage();
	virtual const char *className() const { return "TMessage"; }
	virtual TMessage *clone() const = 0;

	virtual istream &read( istream & );
	virtual ostream &write( ostream & ) const;

	const TMessageHeaderElement &header() const { return MessageHeader; }

	virtual unsigned int getMessageSize() { return 4 + 12 + 4 + MessageHeader.PayloadLength; }

	void setTemplate( bool b ) const { TemplateMessage = b; }
	void setPeer( TBitcoinPeer *p ) { Peer = p; }

  protected:
	virtual bool acceptCommandCode( const string & ) const;
	virtual const char *commandString() const = 0;

	virtual ostream &printOn( ostream & ) const;
	friend ostream &operator<<( ostream &, const TMessage & );

	virtual uint32_t minimumAcceptedVersion() const { return 0; }

  protected:
	TMessageHeaderElement MessageHeader;
	string RawPayload;

	TBitcoinPeer *Peer;

	mutable bool TemplateMessage;
};

//
// Class: TMessageUnimplemented
// Description:
/// TMessage child representing unknown message types
//
class TMessageUnimplemented : public TMessage
{
  public:
	const char *className() const { return "TMessageUnimplemented"; }
	TMessage *clone() const { return new TMessageUnimplemented(*this); }

  protected:
	bool acceptCommandCode( const string & ) const { return true; }
	const char *commandString() const { return NULL; }
};

//
// Class: TMessageWithChecksum
// Description:
//
class TMessageWithChecksum : public TMessage
{
  public:
	const char *className() const { return "TMessageWithChecksum"; }

	istream &read( istream & );
	ostream &write( ostream & ) const;

	unsigned int getMessageSize() { return TMessage::getMessageSize() + 4; }

  protected:
	void verifyPayloadChecksum() const;
	void generatePayloadChecksum();
	ostream &printOn( ostream & ) const;

  protected:
	static TMessageDigest *PayloadHasher;
};

//
// Class: TMessageWithoutChecksum
// Description:
//
class TMessageWithoutChecksum : public TMessage
{
  public:
	const char *className() const { return "TMessageWithoutChecksum"; }

	istream &read( istream & );
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
class TMessage_version : public TMessageWithoutChecksum
{
  public:
	const char *className() const { return "TMessage_version"; }

	istream &read( istream & );

	virtual TVersionedMessageFactory *createMessageFactory() const = 0;

  protected:
	const char *commandString() const { return "version"; }

	ostream &printOn( ostream & ) const;
  protected:
	TLittleEndian32Element Version;
	TLittleEndian64Element Services;
	TLittleEndian64Element Timestamp;
	// XXX: Why does TNetworkAddressElement have a services
	// field _and_ the version message have a services field?  The
	// one in the version message is redundant.
	TNetworkAddressElement AddrMe;
	// Version >= 106
	TNetworkAddressElement AddrFrom;
	TLittleEndian64Element Nonce;
	TNULTerminatedStringElement SubVersionNum;
	// Version >= 209
	TLittleEndian32Element StartingHeight;
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

	istream &read( istream & );
	ostream &write( ostream & ) const;

	TVersionedMessageFactory *createMessageFactory() const;
};

//
// Class: TMessage_version_10600
// Description:
//
class TMessage_version_10600 : public TMessage_version_0
{
  public:
	const char *className() const { return "TMessage_version_10600"; }
	TMessage *clone() const { return new TMessage_version_10600(*this); }

	istream &read( istream & );
	ostream &write( ostream & ) const;

	TVersionedMessageFactory *createMessageFactory() const;

  protected:
	uint32_t minimumAcceptedVersion() const { return 10600; }
};

//
// Class: TMessage_version_20900
// Description:
//
class TMessage_version_20900 : public TMessage_version_10600
{
  public:
	const char *className() const { return "TMessage_version_20900"; }
	TMessage *clone() const { return new TMessage_version_20900(*this); }

	istream &read( istream & );
	ostream &write( ostream & ) const;

	TVersionedMessageFactory *createMessageFactory() const;

  protected:
	uint32_t minimumAcceptedVersion() const { return 20900; }
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
class TMessage_verack : public TMessageWithoutChecksum
{
  public:
	const char *className() const { return "TMessage_verack"; }
	TMessage *clone() const { return new TMessage_verack(*this); }

  protected:
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
class TMessage_addr : public TMessageWithChecksum
{
  public:
	const char *className() const { return "TMessage_addr"; }
	TMessage *clone() const { return new TMessage_addr(*this); }

  protected:
	const char *commandString() const { return "addr"; }
};

//
// Class: TMessage_addr_0
// Description:
//
class TMessage_addr_0 : public TMessage_addr
{
  public:
	const char *className() const { return "TMessage_addr_0"; }
	TMessage *clone() const { return new TMessage_addr_0(*this); }

	istream &read( istream &is ) {
		TMessageWithChecksum::read(is);
		is >> AddressData;
		return is;
	}
	ostream &write( ostream &os ) const {
		TMessageWithChecksum::write(os);
		os << AddressData;
		return os;
	}

  protected:
	ostream &printOn( ostream & ) const;

  protected:
	TNElementsElement<TNetworkAddressElement> AddressData;
};

//
// Class: TMessage_addr
// Description:
//
class TMessage_addr_31402 : public TMessage_addr
{
  public:
	const char *className() const { return "TMessage_addr_31402"; }
	TMessage *clone() const { return new TMessage_addr_31402(*this); }

	istream &read( istream &is ) {
		TMessageWithChecksum::read(is);
		is >> AddressData;
		return is;
	}
	ostream &write( ostream &os ) const {
		TMessageWithChecksum::write(os);
		os << AddressData;
		return os;
	}

  protected:
	ostream &printOn( ostream & ) const;

	uint32_t minimumAcceptedVersion() const { return 31402; }

  protected:
	TNElementsElement<TTimedNetworkAddressElement> AddressData;
};

//
// Class: TMessage_InventoryBase
// Description:
//
class TMessage_InventoryBase : public TMessageWithChecksum
{
  public:
	const char *className() const { return "TMessage_InventoryBase"; }

	istream &read( istream &is ) {
		TMessageWithChecksum::read(is);
		is >> Inventory;
		return is;
	}
	ostream &write( ostream &os ) const {
		TMessageWithChecksum::write(os);
		os << Inventory;
		return os;
	}

  protected:
	TNElementsElement<TInventoryElement> Inventory;

	static const unsigned int MAXIMUM_PAYLOAD;
};

//
// Class: TMessage_inv
// Description:
// Allows a node to advertise its knowledge of one or more objects. It
// can be received unsolicited, or in reply to getblocks.
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
// getdata is used in response to inv, to retrieve the content of a
// specific object, and is usually sent after receiving an inv packet,
// after filtering known elements.
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
// Class: TMessageGetBase
// Description:
//
class TMessageGetBase : public TMessageWithChecksum
{
  public:
	const char *className() const { return "TMessageGetBase"; }

	istream &read( istream &is ) {
		TMessageWithChecksum::read(is);
		is >> Version
			>> HashStarts
			>> HashStop;
		return is;
	}
	ostream &write( ostream &os ) {
		TMessageWithChecksum::write(os);
		os << Version
			<< HashStarts
			<< HashStop;
		return os;
	}

  protected:
	ostream &printOn( ostream & ) const;

  protected:
	TLittleEndian32Element Version;
	TNElementsElement<THashElement> HashStarts;
	THashElement HashStop;
};

//
// Class: TMessage_getblocks
// Description:
// Return an inv packet containing the list of blocks starting at
// hash_start, up to hash_stop or 500 blocks, whichever comes first. To
// receive the next blocks hashes, one needs to issue getblocks again
// with the last known hash.
//
class TMessage_getblocks : public TMessageGetBase
{
  public:
	const char *className() const { return "TMessage_getblocks"; }
	TMessage *clone() const { return new TMessage_getblocks(*this); }

  protected:
	const char *commandString() const { return "getblocks"; }

  protected:
	static const unsigned int MAXIMUM_BLOCK_COUNT;
};

//
// Class: TMessage_getheaders
// Description:
// Return a headers packet containing the headers for blocks starting at
// hash_start, up to hash_stop or 2000 blocks, whichever comes first. To
// receive the next blocks hashes, one needs to issue getheaders again
// with the last known hash. The getheaders command is used by thin
// clients to quickly download the blockchain where the contents of the
// transactions would be irrelevant (because they are not ours)
//
class TMessage_getheaders : public TMessageGetBase
{
  public:
	const char *className() const { return "TMessage_getheaders"; }
	TMessage *clone() const { return new TMessage_getheaders(*this); }

  protected:
	const char *commandString() const { return "getheaders"; }

  protected:
	static const unsigned int MAXIMUM_BLOCK_COUNT;
};

//
// Class: TMessage_tx
// Description:
// The tx message is sent in response to a getdata message which
// requests transaction information from a transaction hash.
//
class TMessage_tx : public TMessageWithChecksum
{
  public:
	const char *className() const { return "TMessage_tx"; }
	TMessage *clone() const { return new TMessage_tx(*this); }

	istream &read( istream &is ) {
		TMessageWithChecksum::read(is);
		is >> Transaction;
		return is;
	}
	ostream &write( ostream &os ) const {
		TMessageWithChecksum::write(os);
		os << Transaction;
		return os;
	}

  protected:
	const char *commandString() const { return "tx"; }

	ostream &printOn( ostream & ) const;
  protected:
	TTransactionElement Transaction;
};

//
// Class: TMessage_block
// Description:
// The block message is sent in response to a getdata message which
// requests transaction information from a block hash.
//
class TMessage_block : public TMessageWithChecksum
{
  public:
	const char *className() const { return "TMessage_block"; }
	TMessage *clone() const { return new TMessage_block(*this); }

	istream &read( istream &is ) {
		TMessageWithChecksum::read(is);
		is >> BlockHeader
			>> Transactions;
		return is;
	}
	ostream &write( ostream &os ) const {
		TMessageWithChecksum::write(os);
		os << BlockHeader
			<< Transactions;
		return os;
	}

	virtual string calculateHash() const;

	const TBlockHeaderElement &blockHeader() const { return BlockHeader; }

  protected:
	const char *commandString() const { return "block"; }

	ostream &printOn( ostream & ) const;
  protected:
	TBlockHeaderElement BlockHeader;
	TNElementsElement<TTransactionElement> Transactions;
};

//
// Class: TMessage_headers
// Description:
// The headers packet returns block headers in response to a getheaders
// packet.
//
class TMessage_headers : public TMessageWithChecksum
{
  public:
	const char *className() const { return "TMessage_headers"; }
	TMessage *clone() const { return new TMessage_headers(*this); }

	istream &read( istream &is ) {
		TMessageWithChecksum::read(is);
		is >> BlockHeaders;
		return is;
	}
	ostream &write( ostream &os ) const {
		TMessageWithChecksum::write(os);
		os << BlockHeaders;
		return os;
	}

  protected:
	const char *commandString() const { return "headers"; }

	ostream &printOn( ostream & ) const;
  protected:
	TNElementsElement<TPaddedBlockHeaderElement> BlockHeaders;
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
class TMessage_getaddr : public TMessageWithChecksum
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
// This message is used for IP Transactions, to ask the peer if it
// accepts such transactions and allow it to look at the content of the
// order.
//
// It contains a CWalletTx object in the reference client.
//
// Not enough information in spec to implement.
//
class TMessage_checkorder : public TMessageWithChecksum
{
  public:
	const char *className() const { return "TMessage_checkorder"; }
	TMessage *clone() const { return new TMessage_checkorder(*this); }

	istream &read( istream &is ) {
		TMessageWithChecksum::read(is);
		is >> WalletTransaction;
		return is;
	}
	ostream &write( ostream &os ) const {
		TMessageWithChecksum::write(os);
		os << WalletTransaction;
		return os;
	}

  protected:
	const char *commandString() const { return "checkorder"; }

  protected:
	TWalletTxElement WalletTransaction;
};

//
// Class: TMessage_submitorder
// Description:
//
class TMessage_submitorder : public TMessage_checkorder
{
  public:
	const char *className() const { return "TMessage_submitorder"; }
	TMessage *clone() const { return new TMessage_submitorder(*this); }

	istream &read( istream &is ) {
		TMessageWithChecksum::read(is);
		is >> TransactionHash
			>> WalletTransaction;
		return is;
	}
	ostream &write( ostream &os ) const {
		TMessageWithChecksum::write(os);
		os << TransactionHash
			<< WalletTransaction;
		return os;
	}

  protected:
	const char *commandString() const { return "submitorder"; }

  protected:
	THashElement TransactionHash;
};

//
// Class: TMessage_reply
// Description:
//
class TMessage_reply : public TMessageWithChecksum
{
  public:
	const char *className() const { return "TMessage_reply"; }
	TMessage *clone() const { return new TMessage_reply(*this); }

	istream &read( istream &is ) {
		TMessageWithChecksum::read(is);
		is >> ReplyCode;
		return is;
	}
	ostream &write( ostream &os ) const {
		TMessageWithChecksum::write(os);
		os << ReplyCode;
		return os;
	}

  protected:
	const char *commandString() const { return "reply"; }

  protected:
	TLittleEndian32Element ReplyCode;
};

//
// Class: TMessage_ping
// Description:
// The ping message is sent primarily to confirm that the TCP/IP
// connection is still valid. An error in transmission is presumed to be
// a closed connection and the address is removed as a current peer. No
// reply is expected as a result of this message being sent nor any sort
// of action expected on the part of a client when it is used.
//
class TMessage_ping : public TMessageWithChecksum
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
class TMessage_alert : public TMessageWithChecksum
{
  public:
	const char *className() const { return "TMessage_alert"; }
	TMessage *clone() const { return new TMessage_alert(*this); }

	istream &read( istream &is ) {
		TMessageWithChecksum::read(is);
		is >> Message
			>> Signature;
		return is;
	}
	ostream &write( ostream &os ) const {
		TMessageWithChecksum::write(os);
		os << Message
			<< Signature;
		return os;
	}

  protected:
	const char *commandString() const { return "alert"; }

  protected:
	TVariableSizedStringElement Message;
	TVariableSizedStringElement Signature;

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
