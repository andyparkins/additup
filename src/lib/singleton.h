// ----------------------------------------------------------------------------
// Project: library
/// @file   singleton.h
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
#ifndef SINGLETON_H
#define SINGLETON_H

// -------------- Includes
// --- C
// --- C++
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

//
// Class:	TSingleton
// Description:
// Rather than make every singleton class write this code again and
// again, this template makes it common.
//
// The way to use this is to instantiate this template in the module
// that owns the singleton; this can be done like this.
//
// \code
//   // Header
//   class TObject {};
//   extern template class TSingleton<TObject>;
//   // Module
//   template class TSingleton<TObject>;
// \endcode
//
// The forward template declaration stops other modules that use the
// singleton from creating a different instance.  Then the explicit
// template instantiation in the module itself gives a specific location
// to the singleton.
//
// From then on using the singleton object is a simple matter of calling
// instance().
//
// \code
//   TSingleton<TObject>::instance()->memberCall();
// \endcode
//
// The only drawback of using this method is that it doesn't implicitly
// prevent creation of more than one of the singleton class as would be
// possible if we made each singleton class explicitly singleton.
//
template <typename Singleton>
class TSingleton
{
  public:
	typedef Singleton T;

  public:
	static void create() { if( Instance == 0 ) Instance = new Singleton; }
	static void destroy() { delete Instance; Instance = 0; }

	static bool exists() { return Instance != 0; }
	static Singleton *instance() { create(); return Instance; }
	// We can't have static operator(), so supply a function that does
	// nearly that.
	static Singleton &O() { return *instance(); }

  protected:
	static Singleton *Instance;

  private:
	// Hide the constructors and make it impossible to create a class of
	// this type
	TSingleton() { throw Instance; }
	~TSingleton() { throw Instance; }
};

template <typename Singleton>
Singleton *TSingleton<Singleton>::Instance = 0;


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
