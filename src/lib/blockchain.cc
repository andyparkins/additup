// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   blockchain.cc
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
#include "blockchain.h"

// -------------- Includes
// --- C
// --- C++
// --- Qt
// --- OS
// --- Project libs
// --- Project
#include "messages.h"


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Template instantiations


// -------------- Class declarations

//
// Function:	TBlock :: TBlock
// Description:
//
TBlock::TBlock( TBlockPool *p ) :
	Pool( p ),
	Parent( NULL )
{
}

//
// Function:	TBlock :: ~TBlock
// Description:
//
TBlock::~TBlock()
{
}

//
// Function:	TBlock :: registerChild
// Description:
//
void TBlock::registerChild( TBlock *Child )
{
	ChildHashes.insert(Child->getHash());
}

//
// Function:	TBlock :: fit
// Description:
//
void TBlock::fit()
{
	// Sanity check
	if( Parent == NULL ) {
		Parent = Pool->getBlock( getParentHash() );
	} else if( Parent != Pool->getBlock( getParentHash() ) ) {
		throw runtime_error( "Block already has a parent; can't be different" );
	}

	// Our parent is not in the pool.  That's okay, it's possible for a
	// child to be seen before a parent.
	if( Parent == NULL ) {
		// The fact that this parent hash is even mentioned means that
		// we'll note its existence in the pool, and expect it to appear
		// later
		Pool->putBlock( getParentHash(), NULL );
	} else {
		// We are a child of our parent, tell it so
		Parent->registerChild( this );
	}
}

// ---------

//
// Function:	TMessageBasedBlock :: TMessageBasedBlock
// Description:
//
TMessageBasedBlock::TMessageBasedBlock( TBlockPool *p ) :
	TBlock( p ),
	Message( NULL )
{
}

//
// Function:	TMessageBasedBlock :: ~TMessageBasedBlock
// Description:
//
TMessageBasedBlock::~TMessageBasedBlock()
{
	delete Message;
}

//
// Function:	TMessageBasedBlock :: setMessage
// Description:
//
void TMessageBasedBlock::updateFromMessage( const string &hash, const TMessage_block *m )
{
	if( Message != NULL ) {
		// XXX: Merge incoming message into existing message?
		delete Message;
	}

	if( m == NULL ) {
		Message = NULL;
		return;
	}

	// Copy the block message.  It's important that we use clone() in
	// case there are multiple versions of TMessage_block in the future.
	// We will be given a TMessage_block_XXX but will only see it as a
	// TMessage_block; therefore we must use clone().  The type-abusing
	// reinterpret_cast<> is justified because we are cloning a
	// TMessage_block; it's just that clone() returns a TMessage, so
	// must be coerced back to TMessage_block.
	Message = reinterpret_cast<TMessage_block*>( m->clone() );

	// The above copy is our quickest way of getting the individual
	// fields out of the message; but we could just as easily have
	// copied every field out of the message into our own structure.
}

//
// Function:	TBlock :: getHash
// Description:
//
const string &TMessageBasedBlock::getHash() const
{
	// If we've already calculated it, then return that
	if( !cachedHash.empty() )
		return cachedHash;

	if( Message == NULL )
		throw runtime_error( "Can't calculate a hash on a NULL block" );

	// Messages are already version enabled; to enable future alteration
	// of block hashing method, we'll defer to the message itself for
	// this calculation, which saves us needing to implement another
	// versioned infrastructure
	cachedHash = Message->calculateHash();

	return cachedHash;
}

//
// Function:	TBlock :: getParentHash
// Description:
//
const string &TMessageBasedBlock::getParentHash() const
{
	return Message->blockHeader().PreviousBlock.getValue();
}

// ---------

//
// Function:	TDatabaseBlock :: TDatabaseBlock
// Description:
//
TDatabaseBlock::TDatabaseBlock( TDatabaseBlockPool *p ) :
	TBlock( p )
{
}

//
// Function:	TDatabaseBlock :: ~TDatabaseBlock
// Description:
//
TDatabaseBlock::~TDatabaseBlock()
{
}

//
// Function:	TDatabaseBlock :: setMessage
// Description:
//
void TDatabaseBlock::updateFromMessage( const string &hash, const TMessage_block *m )
{
	if( m == NULL ) {
		// XXX: Delete existing record?
		return;
	}

	// XXX: Copy message to database
}

// ---------

//
// Function:	TBlockPool :: TBlockPool
// Description:
//
TBlockPool::TBlockPool()
{
}

//
// Function:	TBlockPool :: ~TBlockPool
// Description:
//
TBlockPool::~TBlockPool()
{
}

//
// Function:	TBlockPool :: receiveBlock
// Description:
//
void TBlockPool::receiveBlock( const string &NetworkHash, TMessage_block *message )
{
	// Create a new block
	TBlock *thisBlock = createBlock();

	// Show it the message (this allows it to calculate its hash), and
	// throw an exception if the network hash doesn't equal the
	// calculated hash
	thisBlock->updateFromMessage( NetworkHash, message );

	// See if we already have this block
	TBlock *existingBlock = getBlock( thisBlock->getHash() );
	if( existingBlock == NULL ) {
		// If so, discard the received update, and use the pooled
		// version instead.  We do it this way around, as the already
		// stored version can have information we might not have to
		// hand; but we can easily supply that stored version the
		// information we _do_ have to hand.
		delete thisBlock;
		thisBlock = existingBlock;
		// Let the existing block have a look at the message as well
		thisBlock->updateFromMessage( NetworkHash, message );
	} else {
		// If not, store the new block in the pool
		putBlock( thisBlock->getHash(), thisBlock );
	}

	// Now try and fit this block into the chain, given the new
	// information supplied in the message
	thisBlock->fit();

	// Once the block has fitted into the chain, it's parents and
	// children can also have changed.  In particular:
	//
	// (1) This block's parent can definitely not be a chain tip
	Tips.erase( thisBlock->getParentHash() );
	// (2) If this block has no children, then it can be a chain tip (at
	// least until we find otherwise)
	if( !thisBlock->hasChildren() )
		Tips.insert( thisBlock->getHash() );

	// With these rules in place, the Tips array represents all blocks
	// in the pool that have no children.  Once the full block chain has
	// been downloaded this should be a very limited set.
}

// ---------

//
// Function:	TBlockMemoryPool :: TBlockMemoryPool
// Description:
//
TBlockMemoryPool::TBlockMemoryPool()
{
}

//
// Function:	TBlockMemoryPool :: ~TBlockMemoryPool
// Description:
//
TBlockMemoryPool::~TBlockMemoryPool()
{
	// delete the pool
	while( !Pool.empty() ) {
		delete Pool.begin()->second;
		Pool.erase( Pool.begin() );
	}
}

//
// Function:	TBlockMemoryPool :: createBlock
// Description:
//
TBlock *TBlockMemoryPool::createBlock()
{
	return new TMessageBasedBlock( this );
}

//
// Function:	TBlockMemoryPool :: putBlock
// Description:
//
void TBlockMemoryPool::putBlock( TBlock *Block )
{
	map<string, TBlock*>::iterator it;

	it = Pool.find( Block->getHash() );

	if( it == Pool.end() ) {
		Pool[Block->getHash()] = Block;
	} else {
		// Replace the existing block -- it's the caller's duty to
		// update rather than replace if that's their wish
		delete it->second;
		it->second = Block;
	}
}

//
// Function:	TBlockMemoryPool :: getBlock
// Description:
//
TBlock *TBlockMemoryPool::getBlock( const string &hash ) const
{
	map<string, TBlock*>::const_iterator it;

	it = Pool.find( hash );
	if( it == Pool.end() ) {
		return NULL;
	} else {
		return it->second;
	}
}

//
// Function:	TBlockMemoryPool :: blockExists
// Description:
//
bool TBlockMemoryPool::blockExists( const string &hash ) const
{
	map<string, TBlock*>::const_iterator it;

	it = Pool.find( hash );
	return (it != Pool.end());
}

// ---------

//
// Function:	TDatabaseBlockPool :: TDatabaseBlockPool
// Description:
//
TDatabaseBlockPool::TDatabaseBlockPool()
{
}

//
// Function:	TDatabaseBlockPool :: ~TDatabaseBlockPool
// Description:
//
TDatabaseBlockPool::~TDatabaseBlockPool()
{
}

//
// Function:	TDatabaseBlockPool :: createBlock
// Description:
//
TBlock *TDatabaseBlockPool::createBlock()
{
	return new TDatabaseBlock( this );
}


// -------------- Class member definitions


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
	} catch( exception &e ) {
		cerr << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

