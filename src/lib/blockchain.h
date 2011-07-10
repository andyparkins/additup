// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   blockchain.h
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
#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

// -------------- Includes
// --- C
// --- C++
#include <string>
#include <map>
#include <set>
// --- Qt
// --- OS
// --- Project lib
// --- Project
#include "extraint.h"


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
class TBlockPool;
class TDatabaseBlockPool;
class TMessage_block;
class TMessage_inv;
class TMessage_headers;


// -------------- Function pre-class prototypes


// -------------- Class declarations

//
// Class:	TBlock
// Description:
//
class TBlock
{
  public:
	TBlock( TBlockPool * );
	virtual ~TBlock();

	virtual void updateFromMessage( const TBigInteger &, const TMessage_block * ) = 0;

	virtual const TBigInteger &getHash() const = 0;
	virtual const TBigInteger &getParentHash() const = 0;
	virtual void registerChild( TBlock * );

	void fit();

	bool hasChildren() const { return !ChildHashes.empty(); }
	unsigned int childCount() const { return ChildHashes.size(); }

  protected:
	TBlockPool *Pool;

	TBlock *Parent;
	set<TBigInteger> ChildHashes;
};

//
// Class:	TMessageBasedBlock
// Description:
//
class TMessageBasedBlock : public TBlock
{
  public:
	TMessageBasedBlock( TBlockPool * );
	~TMessageBasedBlock();

	void updateFromMessage( const TBigInteger &, const TMessage_block * );

	const TBigInteger &getHash() const;
	const TBigInteger &getParentHash() const;

	void flush() { cachedHash.invalidate(); }

  protected:
	TMessage_block *Message;
	mutable TBigInteger cachedHash;
};

#if 0
//
// Class:	TDatabaseBlock
// Description:
//
class TDatabaseBlock : public TBlock
{
  public:
	TDatabaseBlock( TDatabaseBlockPool * );
	~TDatabaseBlock();

	void updateFromMessage( const TBigInteger &, const TMessage_block * );

	const TBigInteger &getHash() const;
	const TBigInteger &getParentHash() const;

  protected:
	TDatabaseBlockPool *Pool;
};
#endif

// ---------

//
// Class:	TBlockPool
// Description:
//
class TBlockPool
{
  public:
	TBlockPool();
	virtual ~TBlockPool();

	void receiveBlock( const TBigInteger &, const TMessage_inv * );
	void receiveBlock( const TBigInteger &, const TMessage_block * );
	void receiveBlock( const TBigInteger &, const TMessage_headers * );

	virtual void putBlock( const TBigInteger &, TBlock * ) = 0;
	virtual TBlock *getBlock( const TBigInteger & ) const = 0;
	virtual bool blockExists( const TBigInteger & ) const = 0;

	virtual void scanForNewChildLinks() = 0;

	virtual TBlock *createBlock() = 0;

  protected:
	set<TBigInteger> Tips;
};

//
// Class:	TBlockMemoryPool
// Description:
//
class TBlockMemoryPool : public TBlockPool
{
  public:
	TBlockMemoryPool();
	~TBlockMemoryPool();

	void putBlock( const TBigInteger &, TBlock * );
	TBlock *getBlock( const TBigInteger & ) const;
	bool blockExists( const TBigInteger & ) const;

	void scanForNewChildLinks();

	TBlock *createBlock();

  protected:
	map<TBigInteger, TBlock*> Pool;
	typedef map<TBigInteger, TBlock*>::iterator iterator;
	typedef map<TBigInteger, TBlock*>::const_iterator const_iterator;
};

#if 0
//
// Class:	TDatabaseBlockPool
// Description:
//
class TDatabaseBlockPool : public TBlockPool
{
  public:
	TDatabaseBlockPool();
	~TDatabaseBlockPool();

	void putBlock( const TBigInteger &, TBlock * );
	TBlock *getBlock( const TBigInteger & ) const;
	bool blockExists( const TBigInteger & ) const;

	void scanForNewChildLinks();

	TBlock *createBlock();

  protected:
};
#endif


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
