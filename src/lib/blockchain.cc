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
	// The type-abusing reinterpret_cast<> is justified because we were
	// given a TMessage_block as a parameter; it's just that clone()
	// returns a TMessage, so must be coerced back to TMessage_block.
	Message = reinterpret_cast<TMessage_block*>( m->clone() );
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

	// Show it the message (this allows it to calculate its hash)
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
}

// ---------

//
// Function:	TMemoryBlockPool :: TMemoryBlockPool
// Description:
//
TMemoryBlockPool::TMemoryBlockPool()
{
}

//
// Function:	TMemoryBlockPool :: ~TMemoryBlockPool
// Description:
//
TMemoryBlockPool::~TMemoryBlockPool()
{
	// delete the pool
	while( !Pool.empty() ) {
		delete Pool.begin()->second;
		Pool.erase( Pool.begin() );
	}
}

//
// Function:	TMemoryBlockPool :: createBlock
// Description:
//
TBlock *TMemoryBlockPool::createBlock()
{
	return new TMessageBasedBlock( this );
}

//
// Function:	TMemoryBlockPool :: putBlock
// Description:
//
void TMemoryBlockPool::putBlock( TBlock *Block )
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
// Function:	TMemoryBlockPool :: getBlock
// Description:
//
TBlock *TMemoryBlockPool::getBlock( const string &hash ) const
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
// Function:	TMemoryBlockPool :: blockExists
// Description:
//
bool TMemoryBlockPool::blockExists( const string &hash ) const
{
	map<string, TBlock*>::const_iterator it;

	it = Pool.find( hash );
	return (it != Pool.end());
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

