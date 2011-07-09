// ----------------------------------------------------------------------------
// Project: additup
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
#include <sstream>
// --- Qt
// --- OS
// --- Project libs
#include "crypto.h"
#include "messagefactory.h"
#include "logstream.h"
#include "peer.h"
#include "bitcoinnetwork.h"
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
		new TMessage_version_20900(),
		new TMessage_version_10600(),
		new TMessage_version_0(),
		new TMessage_verack(),
		new TMessage_addr_31402(),
		new TMessage_addr_0(),
		new TMessage_inv(),
		new TMessage_getdata(),
		new TMessage_getblocks(),
		new TMessage_getheaders(),
		new TMessage_tx(),
		new TMessage_block(),
		new TMessage_headers(),
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
		Templates.back()->setTemplate( true );
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
// Function:	TMessage :: TMessage
// Description:
//
TMessage::TMessage() :
	Peer( NULL ),
	TemplateMessage( false )
{
}

//
// Function:	TMessage :: factory
// Description:
//
const TMessageFactory *TMessage::factory() const
{
	return Peer->factory();
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
// Function:	TMessage :: read
// Description:
//
istream &TMessage::read( istream &is )
{
	streampos p = is.tellg();
	try {
		is >> MessageHeader;
	} catch( ios::failure &e ) {
		is.clear();
		throw message_parse_error_underflow();
	}

	// If the network parameters are available, then confirm the magic
	// number
	if( Peer != NULL && Peer->getNetworkParameters() != NULL ) {
		if( MessageHeader.Magic.getValue() != Peer->getNetworkParameters()->Magic ) {
			is.seekg(p);
			throw message_parse_error_magic();
		}
	}

	if( !acceptCommandCode( MessageHeader.Command.getValue() ) )
		throw message_parse_error_type();

	return is;
}

//
// Function:	TMessage :: write
// Description:
//
ostream &TMessage::write( ostream &os ) const
{
	os << MessageHeader;
	return os;
}

//
// Function:	TMessage :: printOn
// Description:
//
ostream &TMessage::printOn( ostream &s ) const
{
	s << className() << "{"
		<< " Command=\"" << MessageHeader.Command.getValue() << "\""
		<< "; Payload=" << MessageHeader.PayloadLength.getValue()
		<< " }";
	return s;
}

// --------

//
// Static:	TMessageWithChecksum :: PayloadHasher
// Description:
//
TMessageDigest *TMessageWithChecksum::PayloadHasher = new TDoubleHash( new THash_sha256, new THash_sha256 );

//
// Function:	TMessageWithChecksum :: read
// Description:
//
istream &TMessageWithChecksum::read( istream &is )
{
	TMessage::read(is);

	is >> MessageHeader.Checksum;

	// Pull the payload out, but preserve position
	TSizedStringElement PL( MessageHeader.PayloadLength );
	streampos p = is.tellg();
	is >> PL;
	is.seekg(p);
	RawPayload = PL.getValue();

	// Confirm the checksum
	verifyPayloadChecksum();

	return is;
}

//
// Function:	TMessageWithChecksum :: write
// Description:
//
ostream &TMessageWithChecksum::write( ostream &os ) const
{
	TMessage::write(os);
	os << MessageHeader.Checksum;
	return os;
}

//
// Function:	TMessageWithChecksum :: verifyPayloadChecksum
// Description:
// First 4 bytes of sha256(sha256(payload))
//
void TMessageWithChecksum::verifyPayloadChecksum() const
{
	string digest = PayloadHasher->transform( RawPayload );
	uint32_t CalculatedChecksum = TMessageElement::littleEndian32FromString( digest, 0 );

	if( CalculatedChecksum != MessageHeader.Checksum ) {
		throw message_parse_error_checksum();
	}
}

//
// Function:	TMessageWithChecksum :: setHeader
// Description:
//
void TMessageWithChecksum::setHeader()
{
	// Write the message to a buffer
	ostringstream oss;
	write( oss );

	// We find the size of the header by asking getMessageSize(), but we
	// need to zero the payload length first
	MessageHeader.PayloadLength = 0;

	// Copy the payload part to RawPayload
	RawPayload = oss.str().substr( getMessageSize(), oss.str().size() - getMessageSize() );

	// Update PayloadLength, now that we know it
	MessageHeader.PayloadLength = RawPayload.size();

	// Update checksum
	generatePayloadChecksum();
}

//
// Function:	TMessageWithChecksum :: generatePayloadChecksum
// Description:
// First 4 bytes of sha256(sha256(payload))
//
void TMessageWithChecksum::generatePayloadChecksum()
{
	string digest = PayloadHasher->transform( RawPayload );
	MessageHeader.Checksum = TMessageElement::littleEndian32FromString( digest, 0 );
	MessageHeader.hasChecksum = true;

	// XXX: This won't work, the message header gets written first,
	// which contains the checksum, but we need the payload to be
	// written in order that we can calculate the checksum.  Therefore
	// -- this won't work.
}

//
// Function:	TMessageWithChecksum :: printOn
// Description:
//
ostream &TMessageWithChecksum::printOn( ostream &s ) const
{
	s << className() << "{"
		<< " Command=\"" << MessageHeader.Command.getValue() << "\""
		<< "; Payload=" << MessageHeader.PayloadLength.getValue()
		<< "; Checksum=" << hex << MessageHeader.Checksum.getValue() << dec
		<< " }";
	return s;
}

// --------

//
// Function:	TMessageWithoutChecksum :: read
// Description:
//
istream &TMessageWithoutChecksum::read( istream &is )
{
	TMessage::read(is);

	MessageHeader.Checksum = 0;
	MessageHeader.hasChecksum = false;

	// Pull the payload out
	TSizedStringElement PL( MessageHeader.PayloadLength );
	RawPayload = PL.getValue();
	// TMessage parses none of the payload, so we point at zero

	return is;
}

// --------

//
// Function:	TMessage_version :: printOn
// Description:
//
ostream &TMessage_version::printOn( ostream &s ) const
{
	TMessageWithoutChecksum::printOn(s);
	s << "{ Version=" << Version.getValue()
		<< "; Services=["
		<< (Services.getValue() & TNetworkAddressElement::NODE_NETWORK ? " NODE_NETWORK" : "" )
		<< " ]; Time=" << Timestamp.getValue()
		<< "; SenderAddress=";
	if( Version >= 10600 ) {
		s << "; ReceiverAddress="
			<< "; Nonce=" << Nonce.getValue()
			<< "; SubVersion=\"" << SubVersionNum.getValue() << "\"";
	}
	if( Version >= 20900 ) {
		s << "; Height=" << StartingHeight.getValue();
	}

	s << " }";
	return s;
}

//
// Function:	TMessage_version :: read
// Description:
//
istream &TMessage_version::read( istream &is )
{
	TMessageWithoutChecksum::read(is);

	// Clear everything
	Version = 0;
	Services = 0;
	Timestamp = 0;
	AddrMe.clear();
	AddrFrom.clear();
	Nonce = 0;
	SubVersionNum.clear();
	StartingHeight = 0;

	return is;
}

//
// Function:	TMessage_version_0 :: read
// Description:
//
istream &TMessage_version_0::read( istream &is )
{
	TMessage_version::read(is);

	// d0
	is >> Version;

	if( Version < minimumAcceptedVersion() )
		throw message_parse_error_version();

	is >> Services;
	is >> Timestamp;

	// d20
	is >> AddrMe;

	return is;
}

//
// Function:	TMessage_version_0 :: write
// Description:
//
ostream &TMessage_version_0::write( ostream &os ) const
{
	TMessage_version::write(os);

	// d0
	os << Version;
	os << Services;
	os << Timestamp;
	os << AddrMe;

	return os;
}

//
// Function:	TMessage_version_0 :: createMessageFactory
// Description:
//
TVersionedMessageFactory *TMessage_version_0::createMessageFactory() const
{
	TMessageFactory_0 *factory = new TMessageFactory_0;
	// The factory has the same peer as us
	factory->setPeer( Peer );
	return factory;
}

//
// Function:	TMessage_version_10600 :: read
// Description:
//
istream &TMessage_version_10600::read( istream &is )
{
	TMessage_version_0::read(is);

	// d46
	is >> AddrFrom;

	// d72
	is >> Nonce;

	// d80: Variable sized NUL-terminated string
	is >> SubVersionNum;

	return is;
}

//
// Function:	TMessage_version_10600 :: write
// Description:
//
ostream &TMessage_version_10600::write( ostream &os ) const
{
	TMessage_version_0::write(os);

	os << AddrFrom;
	os << Nonce;
	os << SubVersionNum;

	return os;
}

//
// Function:	TMessage_version_10600 :: createMessageFactory
// Description:
//
TVersionedMessageFactory *TMessage_version_10600::createMessageFactory() const
{
	TMessageFactory_10600 *factory = new TMessageFactory_10600;
	// The factory has the same peer as us
	factory->setPeer( Peer );
	return factory;
}

//
// Function:	TMessage_version_20900 :: read
// Description:
//
istream &TMessage_version_20900::read( istream &is )
{
	TMessage_version_10600::read(is);

	// Version >= 209
	is >> StartingHeight;

	return is;
}

//
// Function:	TMessage_version_20900 :: write
// Description:
//
ostream &TMessage_version_20900::write( ostream &os ) const
{
	TMessage_version_10600::write(os);
	os << StartingHeight;

	return os;
}

//
// Function:	TMessage_version_20900 :: createMessageFactory
// Description:
//
TVersionedMessageFactory *TMessage_version_20900::createMessageFactory() const
{
	TVersionedMessageFactory *factory;
	// There is no special TMessage_version_31402, so we'll handle the
	// creation of 31402's message factory here
	if( Version < 31402 ) {
		factory = new TMessageFactory_20900;
	} else {
		factory = new TMessageFactory_31402;
	}
	// The factory has the same peer as us
	factory->setPeer( Peer );
	return factory;
}

// --------

//
// Function:	TMessage_addr_0 :: printOn
// Description:
//
ostream &TMessage_addr_0::printOn( ostream &s ) const
{
	TMessageWithChecksum::printOn(s);
	s << "{ N=" << AddressData.size();
	for( unsigned int i = 0; i < AddressData.size(); i++ ) {
		s << "; [" << i << ":"
			<< "; Address=" << AddressData[i].Address.getValue()
			<< " ]";
	}
	s << " }";
	return s;
}

// --------

//
// Function:	TMessage_addr_31402 :: printOn
// Description:
//
ostream &TMessage_addr_31402::printOn( ostream &s ) const
{
	TMessageWithChecksum::printOn(s);
	s << "{ N=" << AddressData.size();
	for( unsigned int i = 0; i < AddressData.size(); i++ ) {
		s << "; [" << i << ":"
			<< " Time=" << AddressData[i].Time.getValue()
			<< "; Address=" << AddressData[i].Address.getValue()
			<< " ]";
	}
	s << " }";
	return s;
}

// --------

//
// Function:	TMessageGetBase :: printOn
// Description:
//
ostream &TMessageGetBase::printOn( ostream &s ) const
{
	TMessageWithChecksum::printOn(s);
	s << "{ N=" << HashStarts.size();
	s << " }";
	return s;
}

// --------

//
// Function:	TMessage_tx :: printOn
// Description:
//
ostream &TMessage_tx::printOn( ostream &s ) const
{
	TMessageWithChecksum::printOn(s);
	s << "{ Ni=" << Transaction.Inputs.size()
		<< "; No=" << Transaction.Outputs.size();
	s << "; Outputs=[";
	for( unsigned int i = 0; i < Transaction.Outputs.size(); i++ ) {
		s << " Value=" << (Transaction.Outputs[i].getValue());
	}
	s << " ]";
	s << " }";
	return s;
}

// --------

//
// Function:	TMessage_block :: calculateHash
// Description:
//
string TMessage_block::calculateHash() const
{
	string hash;
	ostringstream oss;

	// "The SHA256 hash that identifies each block (and which must have
	// a run of 0 bits) is calculated from the first 6 fields of this
	// structure (version, prev_block, merkle_root, timestamp, bits,
	// nonce, and standard SHA256 padding, making two 64-byte chunks in
	// all) and not from the complete block. To calculate the hash, only
	// two chunks need to be processed by the SHA256 algorithm. Since
	// the nonce  field is in the second chunk, the first chunk stays
	// constant during mining and therefore only the second chunk needs
	// to be processed. However, a Bitcoin hash is the hash of the hash,
	// so two SHA256 rounds are needed for each mining iteration."
	BlockHeader.write(oss);

	// Field sizes: 4 + 32 + 32 + 4 + 4 + 4 = 80
	// OpenSSL should pad on its own...

	// Conveniently, PayloadHasher is already double SHA256
	hash = PayloadHasher->transform( oss.str() );

	return hash;
}

//
// Function:	TMessage_block :: calculateMerkleTree
// Description:
//
// From Bitcoin Java library:
// "The merkle hash is based on a tree of hashes calculated from the
// transactions:
//
//          merkleHash
//             /\               (ignore--not multiline comment)
//            /  \              (ignore--not multiline comment)
//          A      B
//         / \    / \           (ignore--not multiline comment)
//       tx1 tx2 tx3 tx4
//
// Basically transactions are hashed, then the hashes of the
// transactions are hashed again and so on upwards into the tree. The
// point of this scheme is to allow for disk space savings later on."
//
// This is a transalation of Block::buildMerkleTree(), which claims to
// be a translation of CBlock::BuildMerkleTree().
//
void TMessage_block::calculateMerkleTree()
{
	MerkleTree.clear();
	MerkleTree.reserve( Transactions.size() * 2 );

	// The tree is primed using the hashes of the transactions
	ostringstream oss;
	for( unsigned int i = 0; i < Transactions.size(); i++ ) {
		MerkleTree.push_back( Transactions[i].getHash() );
	}

	// We run two loops, the first of which is going to iterate through
	// the leaves, like this (using an 8 transaction input as an
	// example)
	//
	//  size = {8, 4, 2, 1}
	//
	// For each of these, a loop from 0 to size is iterated through, two
	// at a time.  Further, a second index is calculated, being the
	// first offset by one:
	//
	//   size == 8  ->  i = {0, 2, 4, 6}   i2 = {1, 3, 5, 7}
	//   size == 4  ->  i = {0, 2}         i2 = (1, 3}
	//   size == 2  ->  i = {0}            i2 = {1}
	//   size == 1  ->  i = {0}            i2 = {1}
	//
	// Finally, the two array indexes for the two hashes being in turn
	// hashed are calculated as offsets from a final iterator, j.
	//
	//   size == 8  ->  j = 0   i = {0, 2, 4, 6}   i2 = {1, 3, 5, 7}
	//   size == 4  ->  j = 8   i = {0, 2}         i2 = (1, 3}
	//   size == 2  ->  j = 12  i = {0}            i2 = {1}
	//   size == 1  ->  j = 14  i = {0}            i2 = {1}
	//
	// indexes are therefore
	//
	//   size == 8  ->  pairs = { {0,1}, {2,3}, {4,5}, {6,7} }  size() = 12
	//   size == 4  ->  pairs = { {8,9}, {10,11} }              size() = 14
	//   size == 2  ->  pairs = { {12, 13} }                    size() = 15
	//   size == 1  ->  pairs = { {14, 15} }                    size() = 16
	//
	// Bug note: The above is what I think the algorithm _should_ do,
	// but there is a misuse of a protective condition that means what
	// it actually does is this:
	//
	//   size == 8  ->  pairs = { {0,1}, {2,3}, {4,5}, {6,7} }  size() = 12
	//   size == 4  ->  pairs = { {8,3}, {10,3} }               size() = 14
	//   size == 2  ->  pairs = { {12, 1} }                     size() = 15
	//   size == 1  ->  pairs = { {14, 0} }                     size() = 16
	//
	// Note that the second in each pair is limited to (size-1), when I
	// suspect it should be limited to (tree.size()-1).
	//

	unsigned int j = 0;
	for( unsigned int size = Transactions.size(); size > 1; size = (size + 1) / 2 ) {
		for( unsigned int i = 0; i < size; i += 2 ) {
			// XXX: suspect this is wrong; should be tree.size()
			// not size itself.
			unsigned int i2 = (i + 1 < size - 1) ? (i + 1) : (size - 1);

			string Buffer;
			Buffer = MerkleTree[j + i].toBytes()
				+ MerkleTree[j + i2].toBytes();

			// The PayloadHasher is fine for creating the Merkle hashes
			Buffer = PayloadHasher->transform( Buffer );

			MerkleTree.push_back( TBigInteger().fromBytes( Buffer ) );
		}
		j += size;
	}
}

//
// Function:	TMessage_block :: setMerkleRoot
// Description:
//
void TMessage_block::setMerkleRoot()
{
	if( MerkleTree.empty() )
		calculateMerkleTree();

	// The last item in the merkle tree is the root.  We simply copy it
	// to the block header

	blockHeader().MerkleRoot = MerkleTree.back().reversedBytes().toBytes();
}

//
// Function:	TMessage_block :: printOn
// Description:
//
ostream &TMessage_block::printOn( ostream &s ) const
{
	TMessageWithChecksum::printOn(s);
	s << "{ N=" << Transactions.size();
	s << " }";
	return s;
}

// --------

//
// Function:	TMessage_headers :: printOn
// Description:
//
ostream &TMessage_headers::printOn( ostream &s ) const
{
	TMessageWithChecksum::printOn(s);
	s << "{ N=" << BlockHeaders.size();
	s << " }";
	return s;
}


// --------

const string TMessage_alert::ALERT_VERIFICATION_KEYS[] = {
	// Hash: 1AGRxqDa5WjUKBwHB9XYEjmkv1ucoUUy1s
	string("04fc9702847840aaf195de8442ebeced"
	"f5b095cdbb9bc716bda9110971b28a49"
	"e0ead8564ff0db22209e0374782c093b"
	"b899692d524e9d6a6956e7c5ecbcd68284"),
	string("")
	};



// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include <typeinfo>
#include "logstream.h"
#include "unittest.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
		TMessageTemplates::container::iterator it;

		log() << "--- Available TMessage templates" << endl;

		for( it = TMessageTemplates::t.Templates.begin();
				it != TMessageTemplates::t.Templates.end(); it++ ) {
			log() << (*it)->className() << endl;
		}
	} catch( std::exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	try {
		TMessageTemplates::container::iterator it;

		log() << "--- Testing parser" << endl;

		const string *p = UNITTESTSampleMessages;
		while( !p->empty() ) {
			TMessage *potential = NULL;
			istringstream iss(*p);
			iss.exceptions( ios::eofbit | ios::failbit | ios::badbit );
			ios::streampos sp;

			log() << "* Attempting to parse " << p << endl;
			for( it = TMessageTemplates::t.Templates.begin();
					it != TMessageTemplates::t.Templates.end(); it++ ) {
				potential = (*it)->clone();
				try {
					sp = iss.tellg();
					potential->read( iss );
				} catch( ios::failure &e ) {
					log() << " - message parse by " << potential->className()
						<< " failed with I/O error.  Message is likely too short." << endl;
					delete potential;
					potential = NULL;
				} catch( message_parse_error_type &e ) {
					delete potential;
					potential = NULL;
					// Try next template
					iss.seekg( sp, ios::beg );
					continue;
				} catch( std::exception &e ) {
					log() << " - message parse by " << potential->className()
						<< " failed, " << e.what()
#ifdef _TYPEINFO
						<< " (" << typeid(e).name() << ")"
#endif
						<< endl;
					delete potential;
					potential = NULL;
					// Try next template
					iss.seekg( sp, ios::beg );
					continue;
				}
				break;
			}
			if( potential != NULL ) {
				log() << " - is a " << *potential << endl;
			} else {
				log() << " - is not a message" << endl;
				p++;
				continue;
			}

			// Now check that conversion back produces the input
			ostringstream oss;
			potential->write( oss );
			if( oss.str() != *p ) {
				log() << "Original message was " << p->size()
					<< " bytes; generated was " << oss.str().size() << endl;
				throw runtime_error("message didn't invert");
			}

			delete potential;

			p++;
		}
	} catch( std::exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

