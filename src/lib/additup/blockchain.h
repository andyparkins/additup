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
#include <queue>
// --- Qt
// --- OS
// --- Project lib
// --- Project
#include "hashtypes.h"
#include "messageelements.h"


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
class TBitcoinNetwork;
class TBitcoinPeer;
class TBlockHeaderElement;
class TTransaction;


// -------------- Function pre-class prototypes


// -------------- Class declarations

//
// Class: block_chain_error
// Description:
//
class block_chain_error : public runtime_error
{
  public:
	explicit block_chain_error( const string &s ) :
		runtime_error(s) {}
};

class block_chain_error_hash : public block_chain_error
{
  public:
	block_chain_error_hash() :
		block_chain_error("block hash mismatch") {}
};

class block_chain_error_too_easy : public block_chain_error
{
  public:
	block_chain_error_too_easy() :
		block_chain_error("block hash was too easy to find") {}
};

class block_chain_error_no_proof_of_work : public block_chain_error
{
  public:
	block_chain_error_no_proof_of_work() :
		block_chain_error("block hash was easier than claimed difficulty") {}
};

class block_chain_error_prescient : public block_chain_error
{
  public:
	block_chain_error_prescient() :
		block_chain_error("block is timestamped in the future") {}
};

class block_chain_error_type : public block_chain_error
{
  public:
	block_chain_error_type() :
		block_chain_error("block type wrong") {}
};

class block_chain_error_version : public block_chain_error
{
  public:
	block_chain_error_version() :
		block_chain_error("block version too old") {}
};

// -------------

//
// Class:	TBlock
// Description:
//
class TBlock
{
  public:
	TBlock( TBlockPool * );
	virtual ~TBlock();
	virtual TBlock *clone() const = 0;

	const TBlock *getParent() const { return Parent; }

	virtual void updateFromHeader( const TBlockHeaderElement & ) = 0;
	virtual void writeToHeader( TBlockHeaderElement & ) const = 0;

	virtual unsigned int getHeight() const = 0;
	virtual const TBitcoinHash &getHash() const = 0;
	virtual const TBitcoinHash &getParentHash() const = 0;
	virtual TBitcoinHash getClaimedDifficulty() const = 0;
	virtual time_t getTimestamp() const = 0;
	virtual void registerChild( TBlock * );
//	virtual TBitcoinHash getCumulativeDifficulty() const = 0;

	void fit();

//	bool hasChild( const TBigInteger &s ) const;
	bool hasChildren() const { return !ChildHashes.empty(); }
	unsigned int childCount() const { return ChildHashes.size(); }
	const TBlock *getChildOnBranch( const TBlock * ) const;
	bool isAncestorOf( const TBlock * ) const;

	TBitcoinHash getNextRequiredDifficulty() const;

	virtual ostream &printOn( ostream & ) const = 0;

  protected:
	void validate() const;

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
	virtual TBlock *clone() const { return new TMessageBasedBlock(*this); }

	void updateFromHeader( const TBlockHeaderElement & );
	void writeToHeader( TBlockHeaderElement & ) const;

	unsigned int getHeight() const;
	const TBitcoinHash &getHash() const;
	const TBitcoinHash &getParentHash() const;
	TBitcoinHash getClaimedDifficulty() const;
	time_t getTimestamp() const;

	void flush() { cachedHash.invalidate(); }

	ostream &printOn( ostream & ) const;

  protected:
	TBlockHeaderElement Header;
	mutable TBitcoinHash cachedHash;
	mutable unsigned int cachedHeight;

  protected:
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

	void updateFromHeader( const TBlockHeaderElement & );

	const TBitcoinHash &getHash() const;
	const TBitcoinHash &getParentHash() const;

  protected:
	TDatabaseBlockPool *Pool;
};
#endif

// ---------

//
// Class:	TBranch
// Description:
//
class TBranch
{
  public:
	TBranch() : Pool(NULL), MiningBlock(NULL) {}
	TBranch( const TBlockPool *p ) : Pool(p), MiningBlock(NULL) {}
	~TBranch() { delete MiningBlock; }

  public:
	const TBlockPool *Pool;
	priority_queue<TTransaction*> QueuedTransactions;
	set<TTransaction*> IncompleteTransactions;
	set<TTransaction*> AdoptedTransactions;
	TBlock *MiningBlock;
};

//
// Class:	TBlockPool
// Description:
//
class TBlockPool
{
  public:
	TBlockPool( const TBitcoinNetwork * );
	virtual ~TBlockPool();

	void receiveInventory( TMessage_inv * );
	void receiveBlock( const TMessage_block * );
	void receiveHeaders( const TMessage_headers * );
	void receiveHeader( const TBlockHeaderElement & );
	void queueBlock( TBitcoinPeer *, const TBitcoinHash & ) const;
	void queueBlock( TBitcoinPeer *, const TBlock * ) const;
	const TBlock *getCommonAncestor( const TBlock *, const TBlock * ) const;

	virtual unsigned int size() const = 0;
	virtual void putBlock( const TBitcoinHash &, TBlock * ) = 0;
	virtual TBlock *getBlock( const TBitcoinHash & ) const = 0;
	virtual bool blockExists( const TBitcoinHash & ) const = 0;

	virtual void scanForNewChildLinks() = 0;

	virtual TBlock *createBlock() = 0;

	const TBitcoinNetwork *getNetwork() { return Network; }

	const TBlock *getBestBranch() const { return getBlock( NominatedBranch ); }

  protected:
	const TBitcoinNetwork *Network;

	map<TBitcoinHash, TBranch> Branches;
	TBitcoinHash NominatedBranch;
};

//
// Class:	TBlockMemoryPool
// Description:
//
class TBlockMemoryPool : public TBlockPool
{
  public:
	TBlockMemoryPool( const TBitcoinNetwork * );
	~TBlockMemoryPool();

	unsigned int size() const { return Pool.size(); }
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
