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
	// delete the pool
	while( !Pool.empty() ) {
		delete *(Pool.begin());
		Pool.erase( Pool.begin() );
	}
}

//
// Function:	TBlockPool :: block
// Description:
//
void TBlockPool::block( TMessage_block *message )
{
	TBlock *newblock;
	newblock = new TBlock( this );

	// Store the block against its own hash in the map
	Pool[newblock->getCalculatedHash()] = newblock;

	// The map makes it easier for us to do lookups, so we can find a
	// parent and children easily

	// Find our parent
	map<string, TBlock*>::iterator it;
	it = Pool.find( newblock->getParentHash() );

	if( it != Pool.end() ) {
		// If it's found then we set it to the new block's parent
		newblock->Parent = (*it);
		// ... and add ourselves as its child
		Parent->addChild( newblock );
	}
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

