// ----------------------------------------------------------------------------
// Project: additup
/// @file   constants.cc
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
#include "constants.h"

// -------------- Includes
// --- C
// --- C++
#include <string>
#include <sstream>
// --- Qt
// --- OS
// --- Project libs
// --- Project
#include "hashtypes.h"
#include "blockchain.h"
#include "messages.h"
#include "script.h"


// -------------- Namespace


// -------------- Macros
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))


// -------------- Module Globals


// -------------- Template instantiations


// -------------- Class declarations

//
// Class:	TPredefinedNetworkParameters
// Description:
//
class TPredefinedNetworkParameters : public TNetworkParameters
{
  public:
	TPredefinedNetworkParameters() {
		// Defined by whatever we support -- not sure this should be here,
		// would like to support multiple versions in the same client
		ProtocolVersion = 31800;
	}
	const char *className() const { return "TPredefinedNetworkParameters"; }

	unsigned int limitDifficultyTimespan( unsigned int ObservedTimespan ) const {
		if( ObservedTimespan < DIFFICULTY_TIMESPAN / 4 ) {
			return DIFFICULTY_TIMESPAN / 4;
		} else if( ObservedTimespan > DIFFICULTY_TIMESPAN * 4 ) {
			return DIFFICULTY_TIMESPAN * 4;
		}
		return ObservedTimespan;
	}

  protected:
	void configureGenesisMessage( TMessage_block & ) const;

  protected:
	static const TStackOperator *SATOSHI_GENESIS_SIGSCRIPT[];
	static const TStackOperator *SATOSHI_GENESIS_PUBKEYSCRIPT[];
};

const TStackOperator *TPredefinedNetworkParameters::SATOSHI_GENESIS_SIGSCRIPT[] = {
	// Initial difficult = 0xffff << ((0x1d-3) * 8)
	new TStackOperator_PUSH_N( string("\xff\xff\x00\x1d",4) ),
	// ???
	new TStackOperator_PUSH_N( string("\x04", 1) ),
	// Message
	new TStackOperator_PUSH_N( string(
		"The Times 03/Jan/2009 Chancellor "
		"on brink of second bailout for banks" ) )
};

const TStackOperator *TPredefinedNetworkParameters::SATOSHI_GENESIS_PUBKEYSCRIPT[] = {
	// Initial difficult = 0xffff << ((0x1d-3) * 8)
	TStackOperator::createPUSH( string(
				"\x04\x67\x8a\xfd\xb0\xfe\x55\x48"
				"\x27\x19\x67\xf1\xa6\x71\x30\xb7"
				"\x10\x5c\xd6\xa8\x28\xe0\x39\x09"
				"\xa6\x79\x62\xe0\xea\x1f\x61\xde"
				"\xb6\x49\xf6\xbc\x3f\x4c\xef\x38"
				"\xc4\xf3\x55\x04\xe5\x1e\xc1\x12"
				"\xde\x5c\x38\x4d\xf7\xba\x0b\x8d"
				"\x57\x8a\x4c\x70\x2b\x6b\xf1\x1d"
				"\x5f", 65) ),
	new TStackOperator_OP_CHECKSIG()
};

//
// Function:	TPredefinedNetworkParameters :: configureGenesisMessage
// Description:
//
void TPredefinedNetworkParameters::configureGenesisMessage( TMessage_block &message ) const
{
	message.blockHeader().Version = 1;
	// Genesis block has no parent, indicate with zero hash
	message.blockHeader().PreviousBlock = 0;
	message.blockHeader().MerkleRoot.get().invalidate();

	// Genesis transaction

	TBitcoinScript_0 genesisSignature(SATOSHI_GENESIS_SIGSCRIPT,
			ARRAY_SIZE(SATOSHI_GENESIS_SIGSCRIPT));
	TBitcoinScript_0 genesisPublicKey(SATOSHI_GENESIS_PUBKEYSCRIPT,
			ARRAY_SIZE(SATOSHI_GENESIS_PUBKEYSCRIPT));

	// Note: references so we can edit in place
	TTransactionElement &Transaction( message.createTransaction() );
	// Regardless of the defaults in the future, the genesis block will
	// be the same
	Transaction.Version = 1;
	Transaction.LockTime = 0;

	TInputSplitElement &Input( Transaction.createInput() );
	// Regardless of the defaults in the future, the genesis block will
	// be the same
	Input.OutPoint.TransactionHash = COINBASE_REFERENCE_HASH;
	Input.OutPoint.Index = COINBASE_REFERENCE_INDEX;
	Input.Sequence = 0xffffffff;
	Input.encodeSignatureScript( genesisSignature );

	TOutputSplitElement &Output( Transaction.createOutput() );
	Output.encodePubKeyScript( genesisPublicKey );
	Output.setValue( 50 );

//	genesisSignature.printOn(log());
//	genesisPublicKey.printOn(log());
}

//
// Class:	TTestnetNetworkParameters
// Description:
//
class TTestnetNetworkParameters : public TPredefinedNetworkParameters
{
  public:
	TTestnetNetworkParameters() {
		DefaultTCPPort = 18333;
		Magic = 0xdab5bffa;
		BitcoinAddressPrefix = 111;
		// 228 bits of 1
		ProofOfWorkLimit = (TBitcoinHash(1) << 228) - 1;

		// Genesis block
		TMessage_block message;
		configureGenesisMessage( message );
		message.blockHeader().Timestamp = 1296688602;
		message.blockHeader().Nonce = 384568319;
		// XXX: Doesn't this imply we need different genesis messages
		// for test and production networks?
		message.blockHeader().DifficultyBits.setTarget(0x07fff8, 0x1d);

		// We're done, update calculated fields
		message.setMerkleRoot();
		message.setHeader();

		TBitcoinHash GenesisHash;
		GenesisHash.fromBytes(
			string("\x00\x00\x00\x07\x19\x95\x08\xe3"
				"\x4a\x9f\xf8\x1e\x6e\xc0\xc4\x77"
				"\xa4\xcc\xcf\xf2\xa4\x76\x7a\x8e"
				"\xee\x39\xc1\x1d\xb3\x67\xb0\x08", 32) );

		// We've created the genesis transaction, wrapped it in a
		// TMessage_block, now we wrap that message in a TBlock, so that
		// it can be compared against the genesis block we eventually
		// see in the block chain
		GenesisBlock = new TMessageBasedBlock( NULL );
		GenesisBlock->updateFromMessage( GenesisHash, &message );
	}
	const char *className() const { return "TTestnetNetworkParameters"; }
};

//
// Class:	TProdnetNetworkParameters
// Description:
//
class TProdnetNetworkParameters : public TPredefinedNetworkParameters
{
  public:
	TProdnetNetworkParameters() {
		DefaultTCPPort = 8333;
		Magic = 0xd9b4bef9;
		BitcoinAddressPrefix = 0;
		// 228 bits of 1
		ProofOfWorkLimit = (TBitcoinHash(1) << 228) - 1;

		// Genesis block
		TMessage_block message;
		configureGenesisMessage( message );
		message.blockHeader().Timestamp = 1231006505;
		message.blockHeader().Nonce = 2083236893;
		message.blockHeader().DifficultyBits.setTarget(0x00ffff, 0x1d);

		// We're done, update calculated fields
		message.setMerkleRoot();
		message.setHeader();

		TBitcoinHash GenesisHash;
		GenesisHash.fromBytes(
			string( "\x00\x00\x00\x00\x00\x19\xd6\x68"
				"\x9c\x08\x5a\xe1\x65\x83\x1e\x93"
				"\x4f\xf7\x63\xae\x46\xa2\xa6\xc1"
				"\x72\xb3\xf1\xb6\x0a\x8c\xe2\x6f" , 32 ) );

		// From http://blockexplorer.com/b/0
		// {
		//  "hash":"000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f",
		//  "ver":1,
		//  "prev_block":"0000000000000000000000000000000000000000000000000000000000000000",
		//  "mrkl_root":"4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b",
		//  "time":1231006505,
		//  "bits":486604799,
		//  "nonce":2083236893,
		//  "n_tx":1,
		//  "size":285,
		//  "tx":[
		//    {
		//      "hash":"4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b",
		//      "ver":1,
		//      "vin_sz":1,
		//      "vout_sz":1,
		//      "lock_time":0,
		//      "size":204,
		//      "in":[
		//        {
		//          "prev_out":{
		//            "hash":"0000000000000000000000000000000000000000000000000000000000000000",
		//            "n":4294967295
		//          },
		//          "coinbase":"04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73"
		//        }
		//      ],
		//      "out":[
		//        {
		//          "value":"50.00000000",
		//          "scriptPubKey":"04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f OP_CHECKSIG"
		//        }
		//      ]
		//    }
		//  ],
		//  "mrkl_tree":[
		//    "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"
		//  ]
		//}

		// We've created the genesis transaction, wrapped it in a
		// TMessage_block, now we wrap that message in a TBlock, so that
		// it can be compared against the genesis block we eventually
		// see in the block chain
		GenesisBlock = new TMessageBasedBlock( NULL );
		GenesisBlock->updateFromMessage( GenesisHash, &message );
	}
	const char *className() const { return "TProdnetNetworkParameters"; }
};

// -------------- Class member definitions

//
// Function:	TOfficialSeedNode :: TOfficialSeedNode
// Description:
// Store the 32 bit IP address in network byte order, so that if you
// printed the 32 bit number in hex, you could simply read the IP
// address left-to-right.
//
// Done like this so that I can simply copy and paste the seed node list
// from the official client, which stores the addresses right-to-left.
//
TOfficialSeedNode::TOfficialSeedNode( uint32_t IP_LittleEndian ) :
	TNodeInfo( 0 )
{
	IPv4 = (IP_LittleEndian & 0xff) << 24
		| (IP_LittleEndian & 0xff00) << 8
		| (IP_LittleEndian & 0xff0000) >> 8
		| (IP_LittleEndian & 0xff000000) >> 24;
}

// -------------- World Globals (need "extern"s in header)

//
// Global:	SEED_NODE_IPV4
// Description:
// The seed list from the official client.
//
// Given as little endian integers (which isn't really sensible, as C
// doesn't have an inherent endianness, so it would be more sensible to
// store them big endian).
//
const TOfficialSeedNode SEED_NODES[] =
{
	0x1ddb1032, 0x6242ce40, 0x52d6a445, 0x2dd7a445, 0x8a53cd47, 0x73263750, 0xda23c257, 0xecd4ed57,
	0x0a40ec59, 0x75dce160, 0x7df76791, 0x89370bad, 0xa4f214ad, 0x767700ae, 0x638b0418, 0x868a1018,
	0xcd9f332e, 0x0129653e, 0xcc92dc3e, 0x96671640, 0x56487e40, 0x5b66f440, 0xb1d01f41, 0xf1dc6041,
	0xc1d12b42, 0x86ba1243, 0x6be4df43, 0x6d4cef43, 0xd18e0644, 0x1ab0b344, 0x6584a345, 0xe7c1a445,
	0x58cea445, 0xc5daa445, 0x21dda445, 0x3d3b5346, 0x13e55347, 0x1080d24a, 0x8e611e4b, 0x81518e4b,
	0x6c839e4b, 0xe2ad0a4c, 0xfbbc0a4c, 0x7f5b6e4c, 0x7244224e, 0x1300554e, 0x20690652, 0x5a48b652,
	0x75c5c752, 0x4335cc54, 0x340fd154, 0x87c07455, 0x087b2b56, 0x8a133a57, 0xac23c257, 0x70374959,
	0xfb63d45b, 0xb9a1685c, 0x180d765c, 0x674f645d, 0x04d3495e, 0x1de44b5e, 0x4ee8a362, 0x0ded1b63,
	0xc1b04b6d, 0x8d921581, 0x97b7ea82, 0x1cf83a8e, 0x91490bad, 0x09dc75ae, 0x9a6d79ae, 0xa26d79ae,
	0x0fd08fae, 0x0f3e3fb2, 0x4f944fb2, 0xcca448b8, 0x3ecd6ab8, 0xa9d5a5bc, 0x8d0119c1, 0x045997d5,
	0xca019dd9, 0x0d526c4d, 0xabf1ba44, 0x66b1ab55, 0x1165f462, 0x3ed7cbad, 0xa38fae6e, 0x3bd2cbad,
	0xd36f0547, 0x20df7840, 0x7a337742, 0x549f8e4b, 0x9062365c, 0xd399f562, 0x2b5274a1, 0x8edfa153,
	0x3bffb347, 0x7074bf58, 0xb74fcbad, 0x5b5a795b, 0x02fa29ce, 0x5a6738d4, 0xe8a1d23e, 0xef98c445,
	0x4b0f494c, 0xa2bc1e56, 0x7694ad63, 0xa4a800c3, 0x05fda6cd, 0x9f22175e, 0x364a795b, 0x536285d5,
	0xac44c9d4, 0x0b06254d, 0x150c2fd4, 0x32a50dcc, 0xfd79ce48, 0xf15cfa53, 0x66c01e60, 0x6bc26661,
	0xc03b47ae, 0x4dda1b81, 0x3285a4c1, 0x883ca96d, 0x35d60a4c, 0xdae09744, 0x2e314d61, 0x84e247cf,
	0x6c814552, 0x3a1cc658, 0x98d8f382, 0xe584cb5b, 0x15e86057, 0x7b01504e, 0xd852dd48, 0x56382f56,
	0x0a5df454, 0xa0d18d18, 0x2e89b148, 0xa79c114c, 0xcbdcd054, 0x5523bc43, 0xa9832640, 0x8a066144,
	0x3894c3bc, 0xab76bf58, 0x6a018ac1, 0xfebf4f43, 0x2f26c658, 0x31102f4e, 0x85e929d5, 0x2a1c175e,
	0xfc6c2cd1, 0x27b04b6d, 0xdf024650, 0x161748b8, 0x28be6580, 0x57be6580, 0x1cee677a, 0xaa6bb742,
	0x9a53964b, 0x0a5a2d4d, 0x2434c658, 0x9a494f57, 0x1ebb0e48, 0xf610b85d, 0x077ecf44, 0x085128bc,
	0x5ba17a18, 0x27ca1b42, 0xf8a00b56, 0xfcd4c257, 0xcf2fc15e, 0xd897e052, 0x4cada04f, 0x2f35f6d5,
	0x382ce8c9, 0xe523984b, 0x3f946846, 0x60c8be43, 0x41da6257, 0xde0be142, 0xae8a544b, 0xeff0c254,
	0x1e0f795b, 0xaeb28890, 0xca16acd9, 0x1e47ddd8, 0x8c8c4829, 0xd27dc747, 0xd53b1663, 0x4096b163,
	0x9c8dd958, 0xcb12f860, 0x9e79305c, 0x40c1a445, 0x4a90c2bc, 0x2c3a464d, 0x2727f23c, 0x30b04b6d,
	0x59024cb8, 0xa091e6ad, 0x31b04b6d, 0xc29d46a6, 0x63934fb2, 0xd9224dbe, 0x9f5910d8, 0x7f530a6b,
	0x752e9c95, 0x65453548, 0xa484be46, 0xce5a1b59, 0x710e0718, 0x46a13d18, 0xdaaf5318, 0xc4a8ff53,
	0x87abaa52, 0xb764cf51, 0xb2025d4a, 0x6d351e41, 0xc035c33e, 0xa432c162, 0x61ef34ae, 0xd16fddbc,
	0x0870e8c1, 0x3070e8c1, 0x9c71e8c1, 0xa4992363, 0x85a1f663, 0x4184e559, 0x18d96ed8, 0x17b8dbd5,
	0x60e7cd18, 0xe5ee104c, 0xab17ac62, 0x1e786e1b, 0x5d23b762, 0xf2388fae, 0x88270360, 0x9e5b3d80,
	0x7da518b2, 0xb5613b45, 0x1ad41f3e, 0xd550854a, 0x8617e9a9, 0x925b229c, 0xf2e92542, 0x47af0544,
	0x73b5a843, 0xb9b7a0ad, 0x03a748d0, 0x0a6ff862, 0x6694df62, 0x3bfac948, 0x8e098f4f, 0x746916c3,
	0x02f38e4f, 0x40bb1243, 0x6a54d162, 0x6008414b, 0xa513794c, 0x514aa343, 0x63781747, 0xdbb6795b,
	0xed065058, 0x42d24b46, 0x1518794c, 0x9b271681, 0x73e4ffad, 0x0654784f, 0x438dc945, 0x641846a6,
	0x2d1b0944, 0x94b59148, 0x8d369558, 0xa5a97662, 0x8b705b42, 0xce9204ae, 0x8d584450, 0x2df61555,
	0xeebff943, 0x2e75fb4d, 0x3ef8fc57, 0x9921135e, 0x8e31042e, 0xb5afad43, 0x89ecedd1, 0x9cfcc047,
	0x8fcd0f4c, 0xbe49f5ad, 0x146a8d45, 0x98669ab8, 0x98d9175e, 0xd1a8e46d, 0x839a3ab8, 0x40a0016c,
	0x6d27c257, 0x977fffad, 0x7baa5d5d, 0x1213be43, 0xb167e5a9, 0x640fe8ca, 0xbc9ea655, 0x0f820a4c,
	0x0f097059, 0x69ac957c, 0x366d8453, 0xb1ba2844, 0x8857f081, 0x70b5be63, 0xc545454b, 0xaf36ded1,
	0xb5a4b052, 0x21f062d1, 0x72ab89b2, 0x74a45318, 0x8312e6bc, 0xb916965f, 0x8aa7c858, 0xfe7effad,
	// End marker
	0x0
};

//
// Global:	NETWORK_TESTNET
// Description:
//
TTestnetNetworkParameters localTESTNET;
const TNetworkParameters *NETWORK_TESTNET = &localTESTNET;

//
// Global:	NETWORK_PRODNET
// Description:
//
TProdnetNetworkParameters localPRODNET;
const TNetworkParameters *NETWORK_PRODNET = &localPRODNET;

//
// Global:	NETWORK_TESTNET
// Description:
//
const TNetworkParameters *KNOWN_NETWORKS[] = {
		NETWORK_PRODNET,
		NETWORK_TESTNET,
		NULL
};


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include "logstream.h"

// -------------- main()

int main( int argc, char *argv[] )
{
//	try {
//		const TOfficialSeedNode *pSeed = SEED_NODES;
//
//		log() << "--- Official seed nodes" << endl;
//		while( *pSeed ) {
//			 pSeed->write(log());
//			 log() << endl;
//			 pSeed++;
//		}
//	} catch( std::exception &e ) {
//		log() << e.what() << endl;
//		return 255;
//	}

	try {
		const TNetworkParameters **pNetwork = KNOWN_NETWORKS;

		log() << "--- Known networks" << endl;
		while( *pNetwork ) {
			log() << (*pNetwork)->className() << endl;
			log() << "GenesisBlock = ";
			(*pNetwork)->GenesisBlock->printOn( log() );
			log() << endl;

			pNetwork++;
		}
	} catch( std::exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

