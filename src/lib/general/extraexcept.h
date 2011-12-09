// ----------------------------------------------------------------------------
// Project: library
/// @file   extraexcept.h
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
#ifndef EXTRAEXCEPT_H
#define EXTRAEXCEPT_H

// -------------- Includes
// --- C
#ifndef __WIN32__
#	include <errno.h>
#endif
// --- C++
#include <stdexcept>
#include <string>
// --- Qt
// --- OS
#ifndef __WIN32__
#	include <netdb.h>
#else
#	include <windows.h>
#	include <winsock2.h>
#endif
// --- Project lib
// --- Project



// -------------- Namespace
	// --- Imported namespaces
	using namespace std;

// -------------- Defines
#if COMPILING_FOR_WINDOWS
	// http://msdn.microsoft.com/en-us/library/ms737828(VS.85).aspx
	// "Although error constants consistent with Berkeley Sockets 4.3
	// are provided for compatibility purposes, applications should,
	// where possible, use the WSA error code definitions. This is
	// because error codes returned by certain Windows Sockets routines
	// fall into the standard range of error codes as defined by
	// Microsoft C"
#	define NETERROR(x) WSA##x
#else
#	define NETERROR(x) x
#endif

// -------------- Library globals
#if COMPILING_FOR_WINDOWS
#else
extern int h_errno;
#endif


// -------------- Typedefs (pre-structure)


// -------------- Enumerations


// -------------- Structures/Unions


// -------------- Typedefs (post-structure)


// -------------- Class pre-declarations


// -------------- Class declarations

// exceptions from <stdexcept>
// class logic_error;
//     class domain_error;
//     class invalid_argument;
//     class length_error;
//     class out_of_range;
//
// class runtime_error;
//     class range_error;
//     class overflow_error;
//     class underflow_error;

//
// Class:	messagecaching_error
// Description:
/// Base class that generates and caches the error message in what()
//
class messagecaching_error : public runtime_error
{
  public:
	explicit messagecaching_error( const string &msg ) throw();
	~messagecaching_error() throw() {};

	virtual const char* what() const throw();

  protected:
	const string msg;

	mutable string CachedMessage;
};

//
// Class:	errnomsg_error
// Description:
/// Exception class for throwing errors using error numbers.
//
/// In addition, errnomsg_error accepts a string.
//
class errnomsg_error : public messagecaching_error
{
  public:
	explicit errnomsg_error( const string &msg, int ) throw();
	~errnomsg_error() throw() {};

	const char* what() const throw();

	int getErrno() const throw() { return ERRNO; }

  protected:
	virtual void appendErrorStringTo( int, string & ) const throw();

  protected:
	const int ERRNO;
};

//
// Class:	libc_error
// Description:
/// Exception class for throwing on errors after libc calls
//
/// Class for throwing errors from the system library.
/// Stores errno from the time it is thrown and looks up the standard
/// error description when asked.
//
class libc_error : public errnomsg_error
{
  public:
#if COMPILING_FOR_WINDOWS
	explicit libc_error( const string &s, int = GetLastError() ) throw();
#else
	explicit libc_error( const string &s, int = errno ) throw();
#endif
};

//
// Class:	netdb_error
// Description:
/// Exception class for throwing errors from the resolver library
//
/// Class for throwing errors from the resolver library.
/// Stores h_errno from the time it is thrown and looks up the standard
/// error description when asked.
//
class netdb_error : public errnomsg_error
{
  public:
#if COMPILING_FOR_WINDOWS
	explicit netdb_error( const string &s, int = WSAGetLastError() ) throw();
#else
	explicit netdb_error( const string &s, int = h_errno ) throw();
#endif

  protected:
	void appendErrorStringTo( int, string & ) const throw();
};

#if COMPILING_FOR_WINDOWS
//
// Class:	socket_error
// Description:
/// Exception class for throwing errors from the Windows socket library.
//
/// Class for throwing errors from Windows sockets..
/// Stores WSAGetLastError() from the time it is thrown and looks up the
/// standard error description when asked.
//
class socket_error : public libc_error
{
  public:
	explicit socket_error( const string &s, int e = WSAGetLastError() ) throw() :
		libc_error(s,e) {}
};
#else
//
// Typedef:	socket_error
// Description:
/// socket_error == libc_error in UNIX
//
typedef class libc_error socket_error;
#endif

//
// Class:	unsupported_method_error
// Description:
/// Thrown when an unsupported function is called
/// Makes sure that calls to unsupported functions do not propagate to
/// cause hard to debug problems later on.
//
class unsupported_method_error : public logic_error {
  public:
	explicit unsupported_method_error( const string &s ) :
		logic_error(s) {}
};


// -------------- Function prototypes


// -------------- World globals ("extern"s only)


// End of namespace
//}
// End of conditional compilation
#endif
