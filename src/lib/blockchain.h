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
#include "hashtypes.h"


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

	virtual void updateFromMessage( const TBitcoinHash &, const TMessage_block * ) = 0;

	virtual const TBitcoinHash &getHash() const = 0;
	virtual const TBitcoinHash &getParentHash() const = 0;
	virtual void registerChild( TBlock * );

	void fit();

//	bool hasChild( const TBigInteger &s ) const;
	bool hasChildren() const { return !ChildHashes.empty(); }
	unsigned int childCount() const { return ChildHashes.size(); }

	virtual ostream &printOn( ostream & ) const = 0;

  protected:
	TBlockPool *Pool;

	TBlock *Parent;
	set<TBitcoinHash> ChildHashes;
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

	void updateFromMessage( const TBitcoinHash &, const TMessage_block * );

	const TBitcoinHash &getHash() const;
	const TBitcoinHash &getParentHash() const;

	void flush() { cachedHash.invalidate(); }

	ostream &printOn( ostream & ) const;

  protected:
	TMessage_block *Message;
	mutable TBitcoinHash cachedHash;
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

	void updateFromMessage( const TBitcoinHash &, const TMessage_block * );

	const TBitcoinHash &getHash() const;
	const TBitcoinHash &getParentHash() const;

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

	void receiveBlock( const TBitcoinHash &, const TMessage_inv * );
	void receiveBlock( const TBitcoinHash &, const TMessage_block * );
	void receiveBlock( const TBitcoinHash &, const TMessage_headers * );

	virtual void putBlock( const TBitcoinHash &, TBlock * ) = 0;
	virtual TBlock *getBlock( const TBitcoinHash & ) const = 0;
	virtual bool blockExists( const TBitcoinHash & ) const = 0;

	virtual void scanForNewChildLinks() = 0;

	virtual TBlock *createBlock() = 0;

  protected:
	set<TBitcoinHash> Tips;
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

	void putBlock( const TBitcoinHash &, TBlock * );
	TBlock *getBlock( const TBitcoinHash & ) const;
	bool blockExists( const TBitcoinHash & ) const;

	void scanForNewChildLinks();

	TBlock *createBlock();

  protected:
	map<TBitcoinHash, TBlock*> Pool;
	typedef map<TBitcoinHash, TBlock*>::iterator iterator;
	typedef map<TBitcoinHash, TBlock*>::const_iterator const_iterator;
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

	void putBlock( const TBitcoinHash &, TBlock * );
	TBlock *getBlock( const TBitcoinHash & ) const;
	bool blockExists( const TBitcoinHash & ) const;

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
