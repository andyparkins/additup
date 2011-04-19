// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   structures.h
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
#ifndef STRUCTURES_H
#define STRUCTURES_H

// -------------- Includes
// --- C
#include <stdint.h>
// --- C++
// --- Qt
// --- OS
// --- Project
// --- Project lib


// -------------- Namespace
	// --- Imported namespaces
	using namespace std;


// -------------- Defines
// General
// Project


// -------------- Constants


// -------------- Typedefs (pre-structure)


// -------------- Enumerations

enum eWords {
	// Flow control
	OP_NOP = 0,
	OP_IF,
	OP_NOTIF,
	OP_ELSE,
	OP_ENDIF,
	OP_VERIFY,
	OP_RETURN,
	// Stack
	OP_TOALTSTACK,
	OP_FROMALTSTACK,
	OP_IFDUP,
	OP_DEPTH,
	OP_DROP,
	OP_DUP,
	OP_NIP,
	OP_OVER,
	OP_PICK,
	OP_ROLL,
	OP_ROT,
	OP_SWAP,
	OP_TUCK,
	OP_2DROP,
	OP_2DUP,
	OP_3DUP,
	OP_2OVER,
	OP_2ROT,
	OP_2SWAP,
	// Splice
	OP_CAT,
	OP_SUBSTR,
	OP_LEFT,
	OP_RIGHT,
	OP_SIZE,
	// Bitwise logic
	OP_INVERT,
	OP_AND,
	OP_OR,
	OP_XOR,
	OP_EQUAL,
	OP_EQUALVERIFY,
	// Arithmetic
	OP_1ADD,
	OP_1SUB,
	OP_2MUL,
	OP_2DIV,
	OP_NEGATE,
	OP_ABS,
	OP_NOT,
	OP_0NOTEQUAL,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_LSHIFT,
	OP_RSHIFT,
	OP_BOOLAND,
	OP_BOOLOR,
	OP_NUMEQUAL,
	OP_NUMEQUALVERIFY,
	OP_NUMNOTEQUAL,
	OP_LESSTHAN,
	OP_GREATERTHAN,
	OP_LESSTHANOREQUAL,
	OP_GREATERTHANOREQUAL,
	OP_MIN,
	OP_MAX,
	OP_WITHIN,
	// Crypto
	OP_RIPEMD160,
	OP_SHA1,
	OP_SHA256,
	OP_HASH160,
	OP_HASH256,
	OP_CODESEPARATOR,
	OP_CHECKSIG,
	OP_CHECKSIGVERIFY,
	OP_CHECKMULTISIG,
	OP_CHECKMULTISIGVERIFY,
	WORD_COUNT
};


// -------------- Structures/Unions

//
// Struct:	sMessageHeader
// Description:
//
/// All Network communications packets have the following initial bytes
/// that form a 'header' to identify a valid packet.
///
/// Data is appended after the Checksum depending upon the data type,
/// with verification using the packet size for the validity of the
/// packet as well.
///
/// (from net.h)
//
struct sMessageHeader
{
	uint32_t MessageStart;
	uint8_t Command[12];
	uint32_t PacketLength;
	uint32_t Checksum;
};

//
// Struct:	sAddressData
// Description:
/// Address data is used in several places in the packets.
//
/// (from CAddress in net.h)
//
struct sAddressData
{
	enum eServices {
		// This node can be asked for full blocks instead of just headers.
		NODE_NETWORK = 1
	};

	uint64_t Services;
	uint8_t Reserved[12];
	uint32_t IPv4Address;
	uint16_t PortNumber;
};

//
// Struct:	sHash
// Description:
//
struct sHash
{
	uint32_t Hash[32];
};

//
// Struct:	sInventoryVector
// Description:
// Inventory vectors are used for notifying other nodes about data they
// may have, and data which is being requested.
//
// (from net.h)
//
struct sInventoryVector
{
	enum eDataType {
		ERROR = 0,
		MSG_TX,
		MSG_BLOCK,
		// Other Data Type values are considered reserved for future
		// implementations.
		DATA_TYPE_COUNT
	};

	eDataType DataType;
	sHash Hash;
};


// -------------- Typedefs (post-structure)


// -------------- Class pre-declarations


// -------------- Function pre-class prototypes


// -------------- Class declarations


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations



// -------------- World globals ("extern"s only)

// End of conditional compilation
#endif
