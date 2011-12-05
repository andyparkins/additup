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
#include <sys/mman.h>
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

//
// Class:	TAutoClearAllocator
// Description:
//
// http://www.codeproject.com/KB/cpp/allocator.aspx
// https://github.com/bitcoin/bitcoin/commit/c1aacf0be347b10a6ab9bbce841e8127412bce41
//
template<typename T, typename baseAllocator = std::allocator<T> >
class TAutoClearAllocator : public baseAllocator
{
  public:
	// --- typedefs
	typedef typename baseAllocator::pointer pointer;
	typedef typename baseAllocator::const_pointer const_pointer;
	typedef typename baseAllocator::reference reference;
	typedef typename baseAllocator::const_reference const_reference;
	typedef typename baseAllocator::value_type value_type;
	typedef typename baseAllocator::size_type size_type;
	typedef typename baseAllocator::difference_type  difference_type;

	// --- constructors/destructor
	TAutoClearAllocator() throw () {}
	TAutoClearAllocator( const TAutoClearAllocator &O ) throw() :
		baseAllocator(O) {}
	// Allow our initialisation by copying any of our cousin classes
	template <typename AnotherT>
		TAutoClearAllocator( const TAutoClearAllocator<AnotherT> &O ) throw() :
			baseAllocator(O) {}
	~TAutoClearAllocator() throw() {}

	// --- allocator interface

	// "The template class member rebind in the table above is
	// effectively a template typedef: if the name Allocator is bound to
	// SomeAllocator<T>, then Allocator::rebind<U>::other is the same
	// type as SomeAllocator<U>."  This is needed to enable list<int> to
	// allocate memory to Node<int> instead of <int>.  vector<> won't
	// care.
	template <typename U>
	struct rebind {
		typedef TAutoClearAllocator<U> other;
	};

	// Private memory should be locked as well as cleared
	T *allocate( size_type n, const void *target = NULL )  {
		T *ptr;
		ptr = baseAllocator::allocate(n, target);
		if( ptr != NULL )
			mlock( ptr, sizeof(T) * n );
		return ptr;
	}

	// Override deallocation to zero the memory before freeing
	void deallocate( T *ptr, size_type n ) {
		if( ptr != NULL ) {
			memset( ptr, 0, sizeof(T) * n );
			munlock( ptr );
		}
		baseAllocator::deallocate( ptr, n );
	}
};

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
template <typename TAllocator = std::allocator<unsigned char> >
class TByteArray_t : public vector<unsigned char>
{
  public:
	TByteArray_t() {}
	TByteArray_t( const string &s ) : vector<unsigned char>( s.begin(), s.end() ) {}
	TByteArray_t( const unsigned char *p, size_type n ) { assign(p,n); }
	TByteArray_t( const char *p ) { assign(p, strlen(p)); }
	TByteArray_t( const char *p, size_type n ) { assign(p,n); }
	TByteArray_t( size_type n, unsigned char v = 0 ) : vector<unsigned char>(n,v) {}
	TByteArray_t( const TByteArray_t &O ) : vector<unsigned char>(O) {}

	using vector<unsigned char>::operator=;

	typedef unsigned char *Pointer;
	Pointer ptr( size_type n = 0 ) { return &operator[](n); }
	typedef const unsigned char *constPointer;
	constPointer ptr( size_type n = 0) const { return &operator[](n); }

	// unsigned char typecasts
	operator Pointer() { return ptr(); }
	operator constPointer() const { return ptr(); }

	// char typecasts
	operator char *() { return reinterpret_cast<char*>(&operator[](0)); }
	operator const char *() const { return reinterpret_cast<const char*>(&operator[](0)); }

	// void typecasts
	operator void *() { return reinterpret_cast<void*>(&operator[](0)); }
	operator const void *() const { return reinterpret_cast<const void*>(&operator[](0)); }

	// string conversions
	operator string() const { return string().assign( *this, size() ); }
	string str() const { return string().assign( *this, size() ); }

	TByteArray_t &assign(const char *p, size_type n) {
		return assign( reinterpret_cast<const unsigned char *>(p), n );
	}
	TByteArray_t &assign(const unsigned char *p, size_type n) {
		resize(n);
		memcpy(ptr(), p, n );
		return *this;
	}
};


// -------------- Typedefs
typedef TByteArray_t<allocator<unsigned char> > TByteArray;
typedef TByteArray_t<TAutoClearAllocator<allocator<unsigned char> > > TSecureByteArray;

// -------------- Constants


// -------------- Inline Functions
// Note: we do not supply an operator<< for the auto clearing secure
// byte array; there is no reason to make it easy to show secure bytes
inline ostream &operator<<( ostream &s, const TByteArray &O ) { return s.write(O,O.size()); }


// -------------- Function prototypes


// -------------- Template instantiations
extern template class TByteArray_t<TAutoClearAllocator<allocator<unsigned char> > >;



// -------------- World globals ("extern"s only)

// End of conditional compilation
#endif
