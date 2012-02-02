// ----------------------------------------------------------------------------
// Project: additup
/// @file   main.cc
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
#include "main.h"

// -------------- Includes
// --- C
#include <stdlib.h>
// --- C++
#include <stdexcept>
#include <typeinfo>
#include <list>
// --- OS
// --- Project libs
#include <general/logstream.h>
#include <general/crypto.h>
#include <general/autoversion.h>
#include <additup/hashtypes.h>
#include <additup/hashtypes.h>
#include <additup/constants.h>
// --- Project


// -------------- Module functions


// -------------- Module Globals
enum eCLIInputMode {
	InputMode_Base58 = 0,
	InputMode_Hex,
	InputMode_SHA256Phrase
} CLIInputMode = InputMode_Base58;
unsigned int DefaultClass;


// -------------- World Globals (need "extern"s in header)


// -------------- Class member definitions


// -------------- Function Definitions

static void showKeyArray( const list<TEllipticCurveKey> &Keys )
{
	list<TEllipticCurveKey>::const_iterator it;

	for( it = Keys.begin(); it != Keys.end(); it++ ) {
		TBitcoinAddress addr(DefaultClass);
		TBitcoinBase58 secret;

		// Create a bitcoin address from the public key
		addr.fromKey(*it);
		// Convert the secret to base58
		secret.fromBytes( (*it).getSecret() );

		cerr << "--- KEY ------ " << secret.toString() << endl;
		cerr << "Secret       : ";
		dumpArray(cerr, secret.toBytes());
		cerr << endl;
		cerr << "Public key   : ";
		TLog::hexify( cerr, (*it).getPublicKey() );
		cerr << endl;
		cerr << "Private (DER): ";
		TLog::hexify( cerr, (*it).getPrivateKey() );
		cerr << endl;

		cerr << "Public addr  : " << addr.toString() << endl;
		cerr << "Class        : " << hex << (unsigned int)(addr.getClass()) << dec
			<< (addr.isValid() ? " (VALID " : " (INVALID ")
			<< ((unsigned int)(addr.getClass()) == NETWORK_PRODNET->AddressClass ? NETWORK_PRODNET->networkName() : "")
			<< ((unsigned int)(addr.getClass()) == NETWORK_TESTNET->AddressClass ? NETWORK_TESTNET->networkName() : "")
			<< ")" << endl;
		cerr << "Public hash  : ";
		dumpArray(cerr, addr.getHash());
		cerr << endl;
		cerr << "Hash CS      : ";
		dumpArray(cerr, addr.getChecksum());
		cerr << endl;
	}
}

// -------------

static void help()
{
	cerr << "Usage: " << endl
		<< " bitkeys <mode specifier> [<options>] [<parameters>]" << endl
		<< "      --help     show help" << endl
		<< "      --version  print version" << endl
		<< "      --address  show information about given addresses" << endl
		;
}

static void version()
{
	cerr << "bitkeys" << VCSID_CONST << endl;
}

static void address( int argc, char *argv[] )
{
	list<TBitcoinAddress> Addresses;
	list<TBitcoinAddress>::const_iterator it;

	for( unsigned int i = 1; i < argc; i++ ) {
		if( argv[i][0] == '-' )
			continue;
		if( CLIInputMode == InputMode_Base58 ) {
			Addresses.push_back( TBitcoinAddress(argv[i]) );
		} else if( CLIInputMode == InputMode_Hex ) {
			TBitcoinAddress x;
			x.fromString( argv[i], 16 );
			Addresses.push_back(x);
		} else {
			throw runtime_error("Phrase input not supported for address mode");
		}
	}

	for( it = Addresses.begin(); it != Addresses.end(); it++ ) {
		cerr << "--- " << (*it).toString() << endl;
		cerr << "Class    : " << hex << (unsigned int)((*it).getClass()) << dec
			<< ((*it).isValid() ? " (VALID " : " (INVALID ")
			<< ((unsigned int)((*it).getClass()) == NETWORK_PRODNET->AddressClass ? NETWORK_PRODNET->networkName() : "")
			<< ((unsigned int)((*it).getClass()) == NETWORK_TESTNET->AddressClass ? NETWORK_TESTNET->networkName() : "")
			<< ")" << endl;
		cerr << "Hash     : ";
		dumpArray(cerr, (*it).getHash());
		cerr << endl;
		cerr << "Checksum : ";
		dumpArray(cerr, (*it).getChecksum());
		cerr << endl;
	}
}

static void secret( int argc, char *argv[] )
{
	list<TEllipticCurveKey> Keys;

	for( unsigned int i = 1; i < argc; i++ ) {
		if( argv[i][0] == '-' )
			continue;

		TEllipticCurveKey ECKEY;

		if( strcmp(argv[i], "generate") != 0 ) {
			TBitcoinBase58 base58;

			if( CLIInputMode == InputMode_Base58 ) {
				base58.fromString( argv[i] );
			} else if( CLIInputMode == InputMode_Hex ) {
				base58.fromString( argv[i], 16 );
			} else if( CLIInputMode == InputMode_SHA256Phrase ) {
				THash_sha256 SHA256;
				TByteArray message(argv[i]);
				TByteArray digest;
				digest = SHA256.transform(message);
				base58.fromBytes( digest );
			}

			// Set the secret part of the key, which will automatically
			// reconstruct the public part
			ECKEY.setSecret( base58.toBytes(32) );
		} else {
			// Make a random new key
			ECKEY.generate();
		}

		Keys.push_back(ECKEY);
	}

	showKeyArray( Keys );
}

// ----- Main

int main( int argc, char *argv[] )
{
	enum eApplicationMode {
		MODE_INVALID,
		MODE_HELP,
		MODE_VERSION,
		MODE_ADDRESS,
		MODE_SECRET,
		MODE_BRAINWALLET,
		MODE_COUNT
	} Mode = MODE_HELP;

	// Initialise network parameters
	KNOWN_NETWORKS::create();
	// Set the default class to PRODNET
	DefaultClass = NETWORK_PRODNET->AddressClass;

	try {
		unsigned int i;

		for( i = 1; i < argc; i++ ) {
			if( strcmp(argv[i], "--debug") == 0 ) {
				TLog::instance().setLogLevel( TLog::Debug );
				log( TLog::Debug ) << "Log level changed because --debug switch found on command line" << endl;
			} else if( strcmp(argv[i], "--verbose") == 0 ) {
				TLog::instance().setLogLevel( TLog::Verbose );
				log( TLog::Verbose ) << "Log level changed because --verbose switch found on command line" << endl;
			} else if( strcmp(argv[i], "--quiet") == 0 ) {
				TLog::instance().setLogLevel( TLog::Warning );
				log( TLog::Info ) << "Log level changed because --quiet switch found on command line" << endl;
			} else if( strcmp(argv[i], "--version") == 0 ) {
				Mode = MODE_VERSION;
			} else if( strcmp(argv[i], "--hex") == 0 ) {
				CLIInputMode = InputMode_Hex;
			} else if( strcmp(argv[i], "--testnet") == 0 ) {
				DefaultClass = NETWORK_TESTNET->AddressClass;
			} else if( strcmp(argv[i], "--secret") == 0 ) {
				Mode = MODE_SECRET;
			} else if( strcmp(argv[i], "--brainwallet") == 0 ) {
				Mode = MODE_BRAINWALLET;
			} else if( strcmp(argv[i], "--address") == 0 ) {
				Mode = MODE_ADDRESS;
			} else if( strcmp(argv[i], "--help") == 0 ) {
				Mode = MODE_HELP;
			}
		}

		switch( Mode ) {
			case MODE_HELP:
				help();
				break;
			case MODE_VERSION:
				version();
				break;
			case MODE_ADDRESS:
				address( argc, argv );
				break;
			case MODE_BRAINWALLET:
				CLIInputMode = InputMode_SHA256Phrase;
			case MODE_SECRET:
				secret( argc, argv );
				break;
			default:
				break;
		}

	} catch( exception &e ) {
		log(TLog::Error) << typeid(e).name() << " " << e.what() << endl;
		// Core dump, in case it's useful for analysis
		abort();
	}

	return 0;
}

