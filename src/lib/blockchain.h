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

	virtual void updateFromMessage( const string &, const TMessage_block * ) = 0;

	virtual const string &getHash() const = 0;
	virtual const string &getParentHash() const = 0;
	virtual void registerChild( TBlock * );

	void fit();

  protected:
	TBlockPool *Pool;

	TBlock *Parent;
	set<string> ChildHashes;
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

	void updateFromMessage( const string &, const TMessage_block * );

	const string &getHash() const;
	const string &getParentHash() const;

	void setNetworkHash( const string &s ) { NetworkHash = s; }
	const string &getNetworkHash() const { return NetworkHash; }

	void flush() { cachedHash.clear(); }

  protected:
	string NetworkHash;
	TMessage_block *Message;
	mutable string cachedHash;
};

//
// Class:	TDatabaseBlock
// Description:
//
class TDatabaseBlock : public TBlock
{
  public:
	TDatabaseBlock( TDatabaseBlockPool * );
	~TDatabaseBlock();

	void updateFromMessage( const string &, const TMessage_block * );

	const string &getHash() const;
	const string &getParentHash() const;

  protected:
	TDatabaseBlockPool *Pool;
};

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

	void receiveBlock( const string &, TMessage_block * );

	virtual void putBlock( const string &, TBlock * ) = 0;
	virtual TBlock *getBlock( const string & ) const = 0;
	virtual bool blockExists( const string & ) const = 0;

  protected:
	virtual TBlock *createBlock() = 0;
};

//
// Class:	TMemoryBlockPool
// Description:
//
class TMemoryBlockPool : public TBlockPool
{
  public:
	TMemoryBlockPool();
	~TMemoryBlockPool();

	void putBlock( TBlock * );
	TBlock *getBlock( const string & ) const;
	bool blockExists( const string & ) const;

  protected:
	TBlock *createBlock();

  protected:
	map<string, TBlock*> Pool;
};

//
// Class:	TDatabaseBlockPool
// Description:
//
class TDatabaseBlockPool : public TBlockPool
{
  public:
	TDatabaseBlockPool();
	~TDatabaseBlockPool();

	void putBlock( TBlock * );
	TBlock *getBlock( const string & ) const;
	bool blockExists( const string & ) const;

  protected:
	TBlock *createBlock();

  protected:
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif