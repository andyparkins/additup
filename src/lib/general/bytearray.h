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
// Class:	TBufferDescription
// Description:
// A simple class to wrap the three properties of a memory buffer:
//  - Address
//  - Maximum size
//  - Used size
// For type safety its far better (and quicker) to pass one of these
// around than (address, size) pairs.
//
class TBufferDescription
{
  public:
	TBufferDescription( void *, size_t );
	~TBufferDescription();

	bool isValid() const { return buffer() != NULL; }

	void *buffer() const { return Pointer; }
	void *ptr() const { return Pointer; }
	size_t capacity() const { return Capacity; }
	size_t used() const { return Used; }
	size_t size() const { return Used; }

	void setUsed( size_t u ) { Used = u; }
	void resize( size_t u ) { Used = u; }

	// Auto-cast
	operator void*() const { return buffer(); }

  protected:
	void *Pointer;
	size_t Capacity;
	size_t Used;
};

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
			munlock( ptr, sizeof(T) * n );
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
class TByteArray_t : public vector<unsigned char, TAllocator>
{
  public:
	typedef unsigned char T;
	typedef T *Pointer;
	typedef const T *constPointer;
	typedef typename vector<T, TAllocator>::size_type size_type;

  public:
	TByteArray_t() {}
	TByteArray_t( const string &s ) : vector<T, TAllocator>( s.begin(), s.end() ) {}
	TByteArray_t( const T *p, size_type n ) { assign(p,n); }
	TByteArray_t( const char *p ) { assign(p, strlen(p)); }
	TByteArray_t( const char *p, size_type n ) { assign(p,n); }
	TByteArray_t( size_type n, T v = 0 ) : vector<T,TAllocator>(n,v) {}
	TByteArray_t( const TByteArray_t &O ) : vector<T,TAllocator>(O) {}

	using vector<T,TAllocator>::operator=;
	using vector<T,TAllocator>::size;
	using vector<T,TAllocator>::resize;

	Pointer ptr( size_type n = 0 ) { return &vector<T,TAllocator>::operator[](n); }
	constPointer ptr( size_type n = 0) const { return &vector<T,TAllocator>::operator[](n); }

	T &at(int n) { return *ptr(n); }
	const T &at(int n) const { return *ptr(n); }

	// unsigned char typecasts
	operator Pointer() { return ptr(); }
	operator constPointer() const { return ptr(); }

	// char typecasts
	operator char *() { return reinterpret_cast<char*>(ptr()); }
	operator const char *() const { return reinterpret_cast<const char*>(ptr()); }

	// void typecasts
	operator void *() { return reinterpret_cast<void*>(ptr()); }
	operator const void *() const { return reinterpret_cast<const void*>(ptr()); }

	// string conversions
	operator string() const { return string().assign( *this, size() ); }
	string str() const { return string().assign( *this, size() ); }

	TByteArray_t &assign(const char *p, size_type n) {
		return assign( reinterpret_cast<const T *>(p), n );
	}
	TByteArray_t &assign(const T *p, size_type n) {
		resize(n);
		memcpy(ptr(), p, n * sizeof(T) );
		return *this;
	}

	// comparisons
	bool operator==( const char *m ) const { return strncmp(*this, m, size()) == 0; }
	bool operator<( const char *m ) const { return strncmp(*this, m, size()) < 0; }
	bool operator>( const char *m ) const { return strncmp(*this, m, size()) > 0; }
};


// -------------- Typedefs
typedef TByteArray_t<allocator<unsigned char> > TByteArray;
typedef TByteArray_t<TAutoClearAllocator<unsigned char> > TSecureByteArray;


// -------------- Constants


// -------------- Inline Functions
// Note: we do not supply an operator<< for the auto clearing secure
// byte array; there is no reason to make it easy to show secure bytes
inline ostream &operator<<( ostream &s, const TByteArray &O ) { return s.write(O,O.size()); }
ostream &dumpArray( ostream &s, const TByteArray &O );


// -------------- Function prototypes


// -------------- Template instantiations
extern template class TByteArray_t<allocator<unsigned char> >;
extern template class TByteArray_t<TAutoClearAllocator<unsigned char> >;



// -------------- World globals ("extern"s only)

// End of conditional compilation
#endif
