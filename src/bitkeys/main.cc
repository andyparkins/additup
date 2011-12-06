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
// --- OS
// --- Project libs
#include <logstream.h>
#include <hashtypes.h>
#include <autoversion.h>
// --- Project


// -------------- Module functions


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Class member definitions


// -------------- Function Definitions

static void help()
{
	cerr << "Usage: " << endl
		<< " bitkeys <mode specifier> <options>" << endl
		<< "      --help     show help" << endl
		<< "      --version  print version" << endl
		;
}

static void version()
{
	cerr << "bitkeys" << VCSID_CONST << endl;
}

// ----- Main

int main( int argc, char *argv[] )
{
	enum eApplicationMode {
		MODE_INVALID,
		MODE_HELP,
		MODE_VERSION,
		MODE_COUNT
	} Mode = MODE_INVALID;

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

