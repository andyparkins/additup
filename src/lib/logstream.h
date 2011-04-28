// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   logstream.h
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
#ifndef LOGSTREAM_H
#define LOGSTREAM_H

// -------------- Includes
// --- C
// --- C++
#include <iostream>
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

//
// Class:	TLog
// Description:
//
class TLog
{
  public:
	static TLog &instance();
	static ostream &hexify( ostream &, const string & );
	virtual ~TLog();

	ostream &getStream() const { return cerr; }

  protected:
	static TLog *Singleton;

  private:
	// Only TLog is allowed to make TLogs - this ensures that only one
	// instance of this class will ever exist
	TLog();
	TLog( const TLog & ) {};
};

// -------------- Function pre-class prototypes


// -------------- Class declarations


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes

ostream &log();


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
