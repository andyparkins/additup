// ----------------------------------------------------------------------------
// Project: library
/// @file   extraexcept.cc
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
#include "extraexcept.h"

// -------------- Includes
// --- C
#include <string.h>
#include <stdio.h>
// --- C++
#include <sstream>
// --- OS
// --- Project libs
// --- Project


// ----- Namespace
//namespace LogSpace {
using namespace std;

// ----- Module Globals

// ----- World Globals (need "extern"s in header)

// ----- Constants

// ----- Function declarations

// ----- Class defintions

//
// Function:	messagecaching_error :: messagecaching_error
// Description:
/// messagecaching_error constructor.
//
/// Constructor: copy the current errno and accept a prefix from the thrower
/// (probably the name of the function that generated the error).
///  @param s           A user defined message to the messagecaching_error
//
messagecaching_error::messagecaching_error( const string &s ) throw() :
	runtime_error( s ),
	msg( s )
{
}

//
// Function:	messagecaching_error :: what
// Description:
/// Shows how to cache a message.  Not considered very useful in itself.
//
/// Overridden virtual from the exception base class that returns the
/// error description.
///
/// It's unlikely that anyone will ever use this directly, as they don't
/// get any benefit over using just runtime_error directly.  However, it
/// is useful as a base class, and this implementation shows how to
/// override what() sensibly.
//
const char* messagecaching_error::what() const throw()
{
	if( !CachedMessage.empty() )
		return CachedMessage.c_str();

	// --- Generate a message to be cached

	if( !msg.empty() )
		CachedMessage = msg;

	// --- Message complete

	return CachedMessage.c_str();
}

// ---------

//
// Function:	errnomsg_error :: errnomsg_error
// Description:
/// errnomsg_error constructor.
//
/// Constructor: copy the current errno and accept a prefix from the thrower
/// (probably the name of the function that generated the error).
///  @param s           An user defined prefix to the errnomsg_error (for example,
//                      which function generated the error?).
//   @param localerrno  In case the errno at the time the class is created
//                      is not the errno of the error you want to throw.
//
errnomsg_error::errnomsg_error( const string &s, int localerrno ) throw() :
	messagecaching_error( s ),
	ERRNO( localerrno )
{
}

//
// Function:	errnomsg_error :: what
// Description:
/// Generates an error message from libc.
//
/// Overridden virtual from the exception base class that returns the
/// error description.
//
const char* errnomsg_error::what() const throw()
{
	if( !CachedMessage.empty() )
		return CachedMessage.c_str();

	if( !msg.empty() )
		CachedMessage = msg + ": ";

	appendErrorStringTo( getErrno(), CachedMessage );
	return CachedMessage.c_str();
}

//
// Function:	errnomsg_error :: appendErrorStringTo
// Description:
/// Generates an error message from libc.
//
/// Overridden virtual from the exception base class that returns the
/// error description.
//
void errnomsg_error::appendErrorStringTo( int e, string &s ) const throw()
{
	// Convert the error number to a string
#ifndef __WIN32__
	s += strerror(e);
#else
	LPVOID lpMessageBuffer;
	int ret = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			e,
			0,
			(LPTSTR) &lpMessageBuffer,
			0,
			NULL
			);
	if( ret == 0 ) {
		char buffer[20];
		sprintf( buffer, "%i", e );
		s += "<ERROR MESSAGE #";
		s += buffer;
		s += " NOT AVAILABLE: ";
		appendErrorStringTo( GetLastError(), s );
		s += ">";
	} else {
		s += static_cast<const char*>( lpMessageBuffer );
	}
	LocalFree( lpMessageBuffer );
#endif
	// Remove trailing whitespace
	string::size_type found;
	found = s.find_last_not_of(" \t\f\v\n\r");
	if( found != string::npos )
		s.erase( found + 1 );
	else
		s.clear();
}

// ---------

//
// Function:	libc_error :: libc_error
// Description:
//
libc_error::libc_error( const string &s, int localerrno ) throw() :
	errnomsg_error( s, localerrno )
{
}

// --------------

//
// Function:	netdb_error :: netdb_error
// Description:
//
netdb_error::netdb_error( const string &s, int localerrno ) throw() :
	errnomsg_error( s, localerrno )
{
}

//
// Function:	netdb_error :: appendErrorStringTo
// Description:
/// Generates an error message from libc.
//
/// Overridden virtual from the exception base class that returns the
/// error description.
//
void netdb_error::appendErrorStringTo( int e, string &s ) const throw()
{
	// Convert the error number to a string
#ifndef __WIN32__
	s += hstrerror(e);
#else
	errnomsg_error::appendErrorStringTo(e,s);
#endif
}


// ----- Function definitions

#ifdef UNITTEST
#include <iostream>
#include "logstream.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	for( unsigned int i = 0; i < 100; i++ ) {
		try {
			throw libc_error("libc()", i);
		} catch( exception &e ) {
			log() << e.what() << endl;
		} catch( ... ) {
			return 1;
		}
		try {
			throw netdb_error("netdb()", i);
		} catch( exception &e ) {
			log() << e.what() << endl;
		} catch( ... ) {
			return 1;
		}
	}

	return 0;
}
#endif

// End of namespace
//}
