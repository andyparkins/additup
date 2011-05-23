// ----------------------------------------------------------------------------
// Project: additup
/// @file   crypto.cc
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
#include "crypto.h"

// -------------- Includes
// --- C
// --- C++
// --- Qt
// --- OS
// --- Project libs
// --- Project


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Class declarations

//
// Function:	ssl_error :: what
// Description:
//
const char* ssl_error::what() const throw()
{
	char buffer[200];

	if( !CachedMessage.empty() )
		return CachedMessage.c_str();

	if( SSLHandle != NULL ) {
		// Here when the error was from OpenSSL
		if( !msg.empty() )
			CachedMessage = msg + ", ";
		switch( getSSLError() ) {
		case SSL_ERROR_NONE:
			// ret was greater than zero - there wasn't an error.  (what
			// are we doing here?)
			CachedMessage += "SSL_ERROR_NONE";
			break;
		case SSL_ERROR_ZERO_RETURN:
			// The SSL part of the connection has been closed cleanly.
			// There is no assurance that the underlying transport has
			// been closed.
			CachedMessage += "SSL_ERROR_ZERO_RETURN";
			break;
		case SSL_ERROR_WANT_READ:
			CachedMessage += "SSL_ERROR_WANT_READ";
			break;
		case SSL_ERROR_WANT_WRITE:
			CachedMessage += "SSL_ERROR_WANT_WRITE";
			break;
		case SSL_ERROR_WANT_CONNECT:
			CachedMessage += "SSL_ERROR_WANT_CONNECT";
			break;
		case SSL_ERROR_WANT_ACCEPT:
			CachedMessage += "SSL_ERROR_WANT_ACCEPT";
			break;
		case SSL_ERROR_WANT_X509_LOOKUP:
			CachedMessage += "SSL_ERROR_WANT_X509_LOOKUP";
			break;
		case SSL_ERROR_SYSCALL:
			CachedMessage += "SSL_ERROR_SYSCALL";
			// Check the error queue; if the error queue is empty, then
			// if ret == 0 an EOF that violates protocol occurred (i.e.
			// was unexpected), if ret == -1, the underlying transport
			// reported an error (check errno; or, as we are libc_error
			// derived, check getErrno())
			if( SSLQueuedError == 0 ) {
				CachedMessage += "(";
				appendErrorStringTo( getErrno(), CachedMessage );
				CachedMessage += ")";
			} else {
				ERR_error_string_n( SSLQueuedError, buffer, sizeof(buffer) );
				CachedMessage += "(";
				CachedMessage += buffer;
				CachedMessage += ")";
			}
			break;
		case SSL_ERROR_SSL:
			CachedMessage += "SSL_ERROR_SSL";
			// A failure in the SSL library itself occurred.  Probably a
			// protocol error - check the error queue.
			ERR_error_string_n( SSLQueuedError, buffer, sizeof(buffer) );
			CachedMessage += "(";
			CachedMessage += buffer;
			CachedMessage += ")";
			break;
		}

		CachedMessage += "; ";
	} else {
		// Generate CachedMessage
		libc_error::what();
	}

	return CachedMessage.c_str();
}

// -----------------

//
// Function:	TDigitalSignature :: TDigitalSignature
// Description:
//
TDigitalSignature::TDigitalSignature() :
	KeyAvailable( false )
{
}

// -----------------

//
// Static:	TEllipticCurveKey :: EC_SIGNATURE_TYPE
// Description:
// "The parameter type is ignored"
//
const int TEllipticCurveKey::EC_SIGNATURE_TYPE = 0;

//
// Function:	TEllipticCurveKey :: TEllipticCurveKey
// Description:
//
TEllipticCurveKey::TEllipticCurveKey() :
	Key( NULL ),
	Precompute_kinv( NULL ),
	Precompute_rp( NULL )
{
	// Create a new key structure
	Key = EC_KEY_new_by_curve_name( NID_secp256k1 );
}

//
// Function:	TEllipticCurveKey :: TEllipticCurveKey
// Description:
//
TEllipticCurveKey::TEllipticCurveKey( const TEllipticCurveKey &O ) :
	Key( NULL ),
	Precompute_kinv( NULL ),
	Precompute_rp( NULL )
{
	// Copy the curve and the keys into a newly allocated structure
	Key = EC_KEY_dup( O.Key );
}

//
// Function:	TEllipticCurveKey :: ~TEllipticCurveKey
// Description:
//
TEllipticCurveKey::~TEllipticCurveKey()
{
	EC_KEY_free( Key );
}

//
// Function:	TEllipticCurveKey :: operator=
// Description:
//
TEllipticCurveKey &TEllipticCurveKey::operator=( const TEllipticCurveKey &O )
{
	// Copy the keys into an already-allocated structure
	EC_KEY_copy( Key, O.Key );

	return *this;
}

//
// Function:	TEllipticCurveKey :: getPrivateKey
// Description:
//
TByteArray TEllipticCurveKey::getPrivateKey() const
{
	// Query the buffer size by using NULL as the target
	int KeySize = i2d_ECPrivateKey( Key, NULL );
	if( KeySize == 0 )
		throw runtime_error( "i2d_ECPrivateKey()" );

	TByteArray ba;
	ba.resize( KeySize );

	// i2d_xxx() wants to leave the pointer pointing after the data its
	// written; we don't care but we need to supply it a pointer it can
	// modify...
	TByteArray::Pointer pba = ba;

	// Internal to DER
	if( i2d_ECPrivateKey( Key, &pba ) != KeySize )
		throw runtime_error( "i2d_ECPrivateKey()" );

	return ba;
}

//
// Function:	TEllipticCurveKey :: getPublicKey
// Description:
//
TByteArray TEllipticCurveKey::getPublicKey() const
{
	// Query the buffer size by using NULL as the target
	int KeySize = i2o_ECPublicKey( Key, NULL );
	if( KeySize == 0 )
		throw runtime_error( "i2d_ECPublicKey()" );

	TByteArray ba;
	ba.resize( KeySize );

	// i2d_xxx() wants to leave the pointer pointing after the data its
	// written; we don't care but we need to supply it a pointer it can
	// modify...
	TByteArray::Pointer pba = ba;

	// Internal to DER
	if( i2o_ECPublicKey( Key, &pba ) != KeySize )
		throw runtime_error( "i2d_ECPublicKey()" );

	return ba;
}

//
// Function:	TEllipticCurveKey :: setPrivateKey
// Description:
// Decode DER encoded private key from given buffer, to internal
// structured representation
//
// This has to be DER encoded, because an EC private key is more than
// just a number, it is two numbers representing curve coordinates
// (r,s).
//
void TEllipticCurveKey::setPrivateKey( const TByteArray &s )
{
	// d2i_ECPrivateKey() needs a pointer to modify
	TByteArray::constPointer pba = s;

	// Read SEC1 formatted private key into our already-allocated
	// structure
	if( d2i_ECPrivateKey( &Key, &pba, s.size() ) == NULL )
		throw runtime_error( "d2i_ECPrivateKey()" );
}

//
// Function:	TEllipticCurveKey :: setPublicKey
// Description:
// Decode octet stream public to internal structured representation.
//
void TEllipticCurveKey::setPublicKey( const TByteArray &s )
{
	// d2i_ECPrivateKey() needs a pointer to modify
	TByteArray::constPointer pba = s;

	if( o2i_ECPublicKey( &Key, &pba, s.size() ) == NULL )
		throw runtime_error( "o2i_ECPublicKey()" );
}

//
// Function:	TEllipticCurveKey :: generate
// Description:
//
void TEllipticCurveKey::generate()
{
	// Create a new private and public key
	if( !EC_KEY_generate_key( Key ) )
		throw runtime_error( "EC_KEY_generate_key()" );

	// Future: speed things up by using ECSDA_sign_setup() to precompute
	// kinv and rp for passing to ECDSA_sign_ex().
}

//
// Function:	TEllipticCurveKey :: getMaximumSignatureSize
// Description:
//
unsigned int TEllipticCurveKey::getMaximumSignatureSize() const
{
	return ECDSA_size( Key );
}

//
// Function:	TEllipticCurveKey :: sign
// Description:
//
TByteArray TEllipticCurveKey::sign( const TByteArray &digest ) const
{
	unsigned int SignatureLength = getMaximumSignatureSize();
	TByteArray Signature;
	Signature.resize( SignatureLength );

	// Create a DER-encoded signature of digest using our public key,
	// storing the resulting signature in Signature
	ECDSA_sign_ex( EC_SIGNATURE_TYPE,
			digest,
			digest.size(),
			Signature, &SignatureLength,
			Precompute_kinv,
			Precompute_rp,
			Key);

	// ECDSA_do_sign_ex() outputs to a newly allocated ECDSA_SIG
	// structure instead of the DER encoded buffer we output here.

	return Signature;
}

//
// Function:	TEllipticCurveKey :: verify
// Description:
//
bool TEllipticCurveKey::verify( const TByteArray &digest, const TByteArray &DERSignature ) const
{
	int ret;

	// Verify that DERSignature is a valid signature of digest, with our
	// public key
	ret = ECDSA_verify( EC_SIGNATURE_TYPE,
			digest, digest.size(),
			DERSignature, DERSignature.size(),
			Key);

	return ret == 1;
}

// -----------------

//
// Function:	TSSLMessageDigest :: TSSLMessageDigest
// Description:
// Call EVP_MD_CTX_init() for the EVP_MD_CTX object that
// TSSLMessageDigest wraps.  This is the OpenSSL equivalent of setting
// the structure to zero.
//
TSSLMessageDigest::TSSLMessageDigest()
{
	EVP_MD_CTX_init( &EVPContext );
}

//
// Function:	TSSLMessageDigest :: ~TSSLMessageDigest
// Description:
// Call final() if necessary to finish any transformation in progress,
// then free up any resources allocated by the EVP_MD_CTX_init() call in
// the constructor.
//
TSSLMessageDigest::~TSSLMessageDigest()
{
	int ret;

	// Clean up as a courtesy - not entirely necessary
	if( Initialised )
		final();

	ret = EVP_MD_CTX_cleanup( &EVPContext );
	if( ret != 1 )
		throw ssl_error( "EVP_MD_CTX_cleanup()" );
}

//
// Function:	TSSLMessageDigest :: init
// Description:
// Attach a particular digest function collection to our EVPContext
// structure.  This ties this TSSLMessageDigest object to a particular
// hash (the one returned by getMD()).
//
void TSSLMessageDigest::init()
{
	int ret;

	ret = EVP_DigestInit_ex( &EVPContext, getMD(), NULL );
	if( ret != 1 )
		throw ssl_error( "EVP_DigestInit_ex()" );

	TMessageDigest::init();
}

//
// Function:	TSSLMessageDigest :: transform
// Description:
// Return the hash for a completely available input string.
//
TByteArray TSSLMessageDigest::transform( const TByteArray &s )
{
	update(s);

	return final();
}

//
// Function:	TSSLMessageDigest :: update
// Description:
// Add the given string to the hash.  This doesn't return a hash value
// yet, it simply adds data to an ongoing hash session.
//
void TSSLMessageDigest::update( const TByteArray &s )
{
	int ret;

	if( !Initialised )
		init();

	ret = EVP_DigestUpdate( &EVPContext, (const void *)s.data(), s.size() );
	if( ret != 1 )
		throw ssl_error( "EVP_DigestUpdate()" );
}

//
// Function:	TSSLMessageDigest :: final
// Description:
// Complete the current ongoing hash.  No data is added to the hash with
// this function, use update() for that.  This function returns the
// actual hash.  It may be thought of as actually calculating the hash,
// but that isn't really the case as the update() function is
// maintaining a rolling hash as data is added, and this function
// completes it and returns the value to us.
//
TByteArray TSSLMessageDigest::final()
{
	TByteArray result;
	unsigned int returnSize;
	int ret;

	if( !Initialised )
		init();

	// Allocate space for the digest
	result.resize( EVP_MAX_MD_SIZE );

	// DigestFinal pushes any bytes remaining in the current block out.
	ret = EVP_DigestFinal_ex( &EVPContext, result, &returnSize );
	if( ret != 1 ) {
		deinit();
		throw ssl_error("EVP_DigestFinal_ex()");
	}

	// Copy the hash bytes into the result TByteArray
	result.resize( returnSize );

	// After a hash is finalised, no more calls to EVP_DigestUpdate()
	// are allowed without starting again with a EVP_DigestInit()
	deinit();

	return result;
}


// -------------- Explicit template instantiations

#define TEMPLATE_INSTANCE( fulltype, shorttype ) \
	template class fulltype;

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


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include <iomanip>
#include "logstream.h"

//
// Function:	safe
// Description:
//
string safe( const string &Source )
{
	string s;

	for( string::size_type i = 0; i < Source.size(); i++ ) {
		if( (unsigned char)(Source[i]) < 0x20 || (unsigned char)(Source[i]) > 0x7f ) {
			s += '.';
		} else {
			s += Source[i];
		}
	}

	return s;
}

// -------------- main()

int main( int argc, char *argv[] )
{

	try {
		TEllipticCurveKey ECKEY;
		TByteArray digest("1234567");
		TByteArray signature;

		// Generate a completely new ECKEY pair
		ECKEY.generate();

		signature = ECKEY.sign( digest );
		log() << "EC: Signature of \"" << digest << "\" (" << digest.size() << ") is ";
		TLog::hexify( log(), signature );

		// Signature is a DER encoded signature
		// 30 = multiple elements
		//  44 = sequence count (why 0x44?  should be 0x02)
		//  02 = bignum
		//   20 = size of bignum (32)
		//    6c 3a b8 bd a9 bc 16 5e 35 33 0f ed 17 3e 17 ab c3 e0 66 71 95 74 0a fc 00 4c a9 15 0a 70 d8 7b
		//  02 = bignum
		//   20 = size of bignum (32)
		//    04 35 7e c0 94 2c ce bb 01 57 25 ea bb c5 83 c2 28 ae d6 9d 17 a8 32 49 a6 69 58 d0 f8 05 79 78
		//  00 00 = don't know

		if( ECKEY.verify( digest, signature ) ) {
			log() << " : verifies" << endl;
		} else {
			log() << " : does not verify" << endl;
		}
	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	try {
		struct sTestSample {
			TMessageDigest *Hasher;
			const TByteArray Plaintext;
			const TByteArray ExpectedDigest;
		};

		log() << "MD: Creating message digest engines" << endl;
		THash_sha1 SHA1;
		THash_sha256 SHA256;
		THash_ripemd160 RIPEMD160;
		TDoubleHash SHASHA256( &SHA256, &SHA256 );
		TDoubleHash RIPESHA256( &RIPEMD160, &SHA256 );
		static const sTestSample Samples[] = {
			{ &SHA1, "PLAINTEXTMESSAGE", TByteArray(
					"\xbc\x91\x35\x01\x0e\xb5\x37\x7d"
					"\x43\xdd\x38\xbe\xf1\xa3\xf8\x34"
					"\xce\x6c\xbe\x43", 20 ) },
			{ &SHA256, "PLAINTEXTMESSAGE", TByteArray(
					"\xcb\xf7\xd5\x4c\x06\xcb\x69\xca"
					"\x92\xed\x73\x10\x90\x40\xb1\xd2"
					"\xcf\x5c\xc5\x5b\x32\x86\xbe\x76"
					"\x1d\x16\xa3\xfc\xa9\xa4\x3b\xd4", 32 ) },
			// Double hash samples from https://en.bitcoin.it/wiki/Protocol_specification
			{ &SHA256, "hello", TByteArray(
					"\x2c\xf2\x4d\xba\x5f\xb0\xa3\x0e"
					"\x26\xe8\x3b\x2a\xc5\xb9\xe2\x9e"
					"\x1b\x16\x1e\x5c\x1f\xa7\x42\x5e"
					"\x73\x04\x33\x62\x93\x8b\x98\x24", 32 ) },
			{ &SHASHA256, "hello", TByteArray(
					"\x95\x95\xc9\xdf\x90\x07\x51\x48"
					"\xeb\x06\x86\x03\x65\xdf\x33\x58"
					"\x4b\x75\xbf\xf7\x82\xa5\x10\xc6"
					"\xcd\x48\x83\xa4\x19\x83\x3d\x50", 32 ) },
			{ &RIPESHA256, "hello", TByteArray(
					"\xb6\xa9\xc8\xc2\x30\x72\x2b\x7c"
					"\x74\x83\x31\xa8\xb4\x50\xf0\x55"
					"\x66\xdc\x7d\x0f", 20 ) },
			// Sample addr message from https://en.bitcoin.it/wiki/Protocol_specification
			// Checksum of messages is the first four bytes of
			// SHA256(SHA256())
			{ &SHASHA256, TByteArray(
					"\x01\xe2\x15\x10\x4d\x01\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00"
					"\x00\x00\x00\x00\x00\x00\x00\xff"
					"\xff\x0a\x00\x00\x01\x20\x8d", 31), TByteArray(
					"\x7f\x85\x39\xc2"
					, 32 ) },
			{ NULL, TByteArray(), TByteArray() }
		};
		const sTestSample *p = Samples;

		log() << "MD: Checking samples hash as expected" << endl;
		while( p->Hasher != NULL ) {
			TByteArray ct;

			log() << "MD: Hashing \"" << safe(p->Plaintext) << "\" (" << p->Plaintext.size() << ")" << endl;
			ct = p->Hasher->transform( p->Plaintext );
			log() << "MD: Hash is ";
			TLog::hexify(log(), ct);
			log() << " (" << ct.size() << ")" << endl;
			if( ct != p->ExpectedDigest )
				throw runtime_error( "Hash is wrong" );

			p++;
		}
		log() << "MD: All samples completed successfully" << endl;
	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

