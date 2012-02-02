// ----------------------------------------------------------------------------
// Project: additup
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
	/// The log level enumeration is an ordered list, running from least
	/// noisy to most noisy.
	enum eLogLevel {
		Noisy,     ///< Always emit this message (i.e. louder than silent)
		Silent,    ///< Never emit any message
		Error,     ///< Only fatal errors
		Warning,   ///< Warnings - i.e. non fatal errors
		Status,    ///< What the system is doing
		Info,      ///< Informational messages
		Verbose,   ///< Highly descriptive
		Debug,     ///< Every possible message
	};
  public:
	static TLog &instance();
	static ostream &hexify( ostream &, const string & );
	virtual ~TLog();

	ostream &getStream() const { return cerr; }

	void setLogLevel( eLogLevel l ) { Level = l; }
	eLogLevel getLogLevel() const { return Level; }

  protected:
	static TLog *Singleton;

	eLogLevel Level;

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

ostream &log( TLog::eLogLevel = TLog::Info );


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
