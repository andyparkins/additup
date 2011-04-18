// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   crypto.h
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
#ifndef CRYPTO_H
#define CRYPTO_H

// -------------- Includes
// --- C
#include <openssl/objects.h>
#include <openssl/ecdsa.h>
// --- C++
#include <string>
// --- OS
// --- Project


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

class TEllipticCurveKey
{
  public:
	TEllipticCurveKey() {
		Key = EC_KEY_new_by_curve_name( NID_secp256k1 );
		EC_KEY_generate_key( Key );
	}
	~TEllipticCurveKey() { EC_KEY_free( Key ); }

	unsigned int getSize() const {
		return ECDSA_size( Key );
	}

	string sign( const string &s ) const {
		string Out;
		unsigned int OutLen = getSize();
		unsigned char *buffer = new unsigned char[OutLen];
		ECDSA_sign( 0,
				reinterpret_cast<const unsigned char *>(s.data()), s.size(),
				buffer, &OutLen,
				Key);
		Out.assign( reinterpret_cast<const char*>(buffer), OutLen );
		delete[] buffer;
		return Out;
	}

	bool verify( const string &digest, const string &signature ) const {
		int ret;
		ret = ECDSA_verify( 0,
				reinterpret_cast<const unsigned char *>(digest.data()), digest.size(),
				reinterpret_cast<const unsigned char *>(signature.data()), signature.size(),
				Key);
		return ret == 1;
	}

  protected:
	EC_KEY *Key;
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)

// End of conditional compilation
#endif
