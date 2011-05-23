// ----------------------------------------------------------------------------
// Project: library
/// @file   bytearray.h
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
#ifndef BYTEARRAY_H
#define BYTEARRAY_H

// -------------- Includes
// --- C
#include <string.h>
// --- C++
#include <vector>
#include <string>
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
// Class:	TByteArray
// Description:
// Essentially a string, but no implicit assumption of printability,
// but with an assumption of being single bytes (strings could be
// multi-byte).
//
// We rely on the contiguity guarantee of C++ STL's vector<>:
// http://herbsutter.com/2008/04/07/cringe-not-vectors-are-guaranteed-to-be-contiguous/
//
// "Why is it so important that vectors be contiguous? Because that's
// what you need to guarantee that a vector is layout-compatible with a
// C array, and therefore we have no reason not to use vector as a
// superior and type-safe alternative to arrays even when we need to
// exchange data with C code. So vector is our gateway to other
// languages and most operating systems features, whose lingua franca is
// the venerable C array."
//
// This is really then a wrapper around vector<unsigned char>() which
// provides a few casts and operators to make it easier to use
// concisely.
//
class TByteArray : public vector<unsigned char>
{
  public:
	TByteArray() {}
	TByteArray( const string &s ) : vector<unsigned char>( s.begin(), s.end() ) {}
	TByteArray( const unsigned char *p, size_type n ) { assign(p,n); }
	TByteArray( const char *p ) { assign(p, strlen(p)); }
	TByteArray( const char *p, size_type n ) { assign(p,n); }
	TByteArray( size_type n, unsigned char v = 0 ) : vector<unsigned char>(n,v) {}
	TByteArray( const TByteArray &O ) : vector<unsigned char>(O) {}

	using vector<unsigned char>::operator=;

	typedef unsigned char *Pointer;
	Pointer ptr() { return &operator[](0); }
	typedef const unsigned char *constPointer;
	constPointer ptr() const { return &operator[](0); }

	// unsigned char typecasts
	operator Pointer() { return ptr(); }
	operator constPointer() const { return ptr(); }

	// char typecasts
	operator char *() { return reinterpret_cast<char*>(&operator[](0)); }
	operator const char *() const { return reinterpret_cast<const char*>(&operator[](0)); }

	operator string() const { return string().assign( *this, size() ); }

	TByteArray &assign(const char *p, size_type n) {
		return assign( reinterpret_cast<const unsigned char *>(p), n );
	}
	TByteArray &assign(const unsigned char *p, size_type n) {
		resize(n);
		memcpy(ptr(), p, n );
		return *this;
	}
};


// -------------- Function pre-class prototypes


// -------------- Class declarations


// -------------- Constants


// -------------- Inline Functions
inline ostream &operator<<( ostream &s, const TByteArray &O ) { return s.write(O,O.size()); }


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)

// End of conditional compilation
#endif
