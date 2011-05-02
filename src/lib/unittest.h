// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   unittest.h
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
#ifndef UNITTEST_H
#define UNITTEST_H
#ifdef UNITTEST

// -------------- Includes
// --- C
// --- C++
#include <string>
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


// -------------- Structures/Unions


// -------------- Typedefs (post-structure)


// -------------- Class pre-declarations


// -------------- Function pre-class prototypes


// -------------- Class declarations


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)

//
// Global:	SampleMessages[]
// Description:
//
static const string UNITTESTSampleMessages[] = {
	// Short invalid message
	string("\xf9\xbe\xb4\xd9"    // Magic
			"unimplement\0"      // Command
			, 16 ),
	// TMessageUnimplemented
	string("\xf9\xbe\xb4\xd9"    // Magic
			"unimplement\0"      // Command
			"\0\0\0\0"         // Length
			, 20 ),
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

//
// Global:	UNITTESTSampleScripts[]
// Description:
//
static const string UNITTESTSampleScripts[] = {
	//   76       A9             14
	// OP_DUP OP_HASH160    Bytes to push
	//
	// 89 AB CD EF AB BA AB BA AB BA AB BA AB BA AB BA AB BA AB BA   88         AC
	//                       Data to push                     OP_EQUALVERIFY OP_CHECKSIG
	string("\x76\xa9"
			"\x14"
			"\x89\xAB\xCD\xEF\xAB\xBA\xAB\xBA\xAB\xBA\xAB\xBA\xAB\xBA\xAB\xBA\xAB\xBA\xAB\xBA"
			"\x88\xAC"
			, 25 ),

	string()
};

// End of conditional compilation
#endif
#endif
