// ----------------------------------------------------------------------------
// Project: additup
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
// --- C++
#include <vector>
// --- OS
// --- Lib
#include <openssl/objects.h>
#include <openssl/ecdsa.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
// --- Project
#include "extraexcept.h"
#include "bytearray.h"


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
// Class:	ssl_error
// Description:
/// Exception class for ssl library errors
//
class ssl_error : public libc_error
{
  protected:
	SSL *SSLHandle;
	int SSLerror;
	unsigned long SSLQueuedError;
	int ret;
  public:
	// First form of constructor is for errors generated by
	// TSecureSocketConnection.
	explicit ssl_error( const string &s ) throw() :
		libc_error(s, 0), SSLHandle(NULL), SSLerror(SSL_ERROR_NONE), ret(0) {
			SSLQueuedError = ERR_get_error();
		};
	// Second form of constructor is for errors generated by
	// OpenSSL itself
	explicit ssl_error( SSL *h, int r, const string &s ) throw() :
		libc_error(s), SSLHandle(h), ret(r) {
			SSLerror = SSL_get_error(SSLHandle, ret);
			SSLQueuedError = ERR_get_error();
		}

	virtual ~ssl_error() throw() {};

	SSL *getHandle() const { return SSLHandle; }
	int getSSLError() const { return SSLerror; }

	virtual const char* what() const throw();
};

// ------------

//
// Class:	TDigitalSignature
// Description:
/// Abstract base class to represent public key digital signature
//
// A digital signature uses asymmetric cryptography to produce a unique
// number from a given message and a private key and public key.  A
// receiver of the message can verify the signature is valid given the
// public key and the message.
//
// This class assumes that the signature is generated for small
// quantities of data (asymmetric cryptography is CPU intensive so it is
// more efficient to hash a message and sign the hash).  This
// representative hash is called the message digest.  The sign()
// function returns a signature, and the verify function checks that
// signature against the given digest to establish veracity.
//
class TDigitalSignature
{
  public:
	TDigitalSignature();
	virtual ~TDigitalSignature() {}

	virtual void generate() = 0;
	virtual void invalidate() = 0;

	virtual TByteArray getPublicKey() const = 0;
	virtual TSecureByteArray getPrivateKey() const = 0;
	virtual void setPublicKey( const TByteArray & ) = 0;
	virtual void setPrivateKey( const TSecureByteArray & ) = 0;

	virtual TByteArray sign( const TByteArray &digest ) const = 0;
	virtual bool verify( const TByteArray &digest, const TByteArray &signature ) const = 0;

	bool isValid() const { return KeyAvailable; }

  protected:
	bool KeyAvailable;
};

//
// Class:	TEllipticCurveKey
// Description:
//
class TEllipticCurveKey : public TDigitalSignature
{
  public:
	TEllipticCurveKey();
	TEllipticCurveKey( const TEllipticCurveKey & );
	~TEllipticCurveKey();

	TEllipticCurveKey &operator=( const TEllipticCurveKey & );

	TByteArray getPublicKey() const;
	TSecureByteArray getPrivateKey() const;
	void setPublicKey( const TByteArray & );
	void setPrivateKey( const TSecureByteArray & );

	void generate();
	void invalidate();

	TByteArray sign( const TByteArray &digest ) const;
	bool verify( const TByteArray &digest, const TByteArray &signature ) const;

  protected:
	TByteArray::size_type getMaximumSignatureSize() const;

	// const EC_GROUP *EC_KEY_get0_group(const EC_KEY *);
	// int EC_KEY_set_group(EC_KEY *, const EC_GROUP *);
	//
	// const BIGNUM *EC_KEY_get0_private_key(const EC_KEY *);
	// int EC_KEY_set_private_key(EC_KEY *, const BIGNUM *);
	//
	// const EC_POINT *EC_KEY_get0_public_key(const EC_KEY *);
	// int EC_KEY_set_public_key(EC_KEY *, const EC_POINT *);

  protected:
	EC_KEY *Key;

	BIGNUM *Precompute_kinv;
	BIGNUM *Precompute_rp;

	static const int EC_SIGNATURE_TYPE;
};

// ------------

//
// Class:		TMessageDigest
// Description:
/// Abstract base class to make a common interface to hashes.
//
/// "Message digest" is the name that OpenSSL (and other cyrptographers)
/// give to what is commonly called a "hash".  The idea is that an
/// abritrary length of arbitrary data has some operation performed on it
/// such that a number is generated.  That number is a cryptographicly
/// robust representation of that data.  The term "cryptographically
/// robust" is used here to mean that it is very difficult to create a
/// set of data that will generate any particular hash.  In other words,
/// we may rely on the hash as representing that particular combination
/// of input data in a way that is not easily faked.
///
/// A good hash is highly sensitive to small changes in the input data,
/// is unpredicatable, and fast.
///
//
class TMessageDigest
{
  public:
	TMessageDigest() : Initialised( false ) {}
	virtual ~TMessageDigest() {}
	virtual TByteArray transform( const TByteArray & ) = 0;

  protected:
	virtual void init() { Initialised = true; }
	virtual void deinit() { Initialised = false; }

  protected:
	bool Initialised;
};

//
// Class:		TDoubleHash
// Description:
//
class TDoubleHash : public TMessageDigest
{
  public:
	TDoubleHash( TMessageDigest *h2, TMessageDigest *h1 ) :
		Hash1(h1), Hash2(h2) {}

	TByteArray transform( const TByteArray &s ) { return Hash2->transform( Hash1->transform(s) ); }

  protected:
	TMessageDigest *Hash1;
	TMessageDigest *Hash2;
};


//
// Class:		TSSLMessageDigest
// Description:
/// Wrapper class around an OpenSSL EVP message digest collection.
//
/// This class represents a convenience wrapper around OpenSSL's message
/// digest functions.  Combined with TSSLMessageDigestTemplate below, it
/// makes hashing data as simple as:
///
///   \code
///   THash_sha1 Hasher;
///   TByteArray output = Hasher.transform( data_to_be_hashed );
///   \endcode
///
/// You will find the full list of available hashes at the end of this
/// file in the form:
///
///   \code
///   TEMPLATE_INSTANCE(TSSLMessageDigestTemplate<EVP_md5>, THash_md5);
///   \endcode
///
/// They are template instantiations to make objects per hash type.
//
class TSSLMessageDigest : public TMessageDigest
{
  public:
	TSSLMessageDigest();
	TSSLMessageDigest( const TSSLMessageDigest & );
	virtual ~TSSLMessageDigest();

	TByteArray transform( const TByteArray & );
	void update( const TByteArray & );
	TByteArray final();

  protected:
	virtual const EVP_MD *getMD() = 0;
	void init();

  protected:
	EVP_MD_CTX EVPContext;
};

//
// Template:		TSSLMessageDigestTemplate
// Description:
/// Template to generate the various hash classes
//
template < const EVP_MD *(*F)() >
class TSSLMessageDigestTemplate : public TSSLMessageDigest
{
  public:
	TSSLMessageDigestTemplate() : TSSLMessageDigest() {}

  protected:
	const EVP_MD *getMD() { return F(); }
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations

#define TEMPLATE_INSTANCE( fulltype, shorttype ) \
	extern template class fulltype; \
	typedef fulltype shorttype;
//
// This block is pulled out of the openssl/evp.h file, but manipulated
// to make the hash classes I want
//
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_md_null>, THash_md_null );
#ifndef OPENSSL_NO_MD2
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_md2>, THash_md2 );
#endif
#ifndef OPENSSL_NO_MD4
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_md4>, THash_md4 );
#endif
#ifndef OPENSSL_NO_MD5
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_md5>, THash_md5 );
#endif
#ifndef OPENSSL_NO_SHA
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_sha>, THash_sha );
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_sha1>, THash_sha1 );
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_dss>, THash_dss );
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_dss1>, THash_dss1 );
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_ecdsa>, THash_ecdsa );
#endif
#ifndef OPENSSL_NO_SHA256
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_sha224>, THash_sha224 );
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_sha256>, THash_sha256 );
#endif
#ifndef OPENSSL_NO_SHA512
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_sha384>, THash_sha384 );
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_sha512>, THash_sha512 );
#endif
#ifndef OPENSSL_NO_MDC2
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_mdc2>, THash_mdc2 );
#endif
#ifndef OPENSSL_NO_RIPEMD
TEMPLATE_INSTANCE( TSSLMessageDigestTemplate<EVP_ripemd160>, THash_ripemd160 );
#endif
#undef TEMPLATE_INSTANCE


// -------------- World globals ("extern"s only)

// End of conditional compilation
#endif
