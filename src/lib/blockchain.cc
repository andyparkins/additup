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
#include "bitcoinnetwork.h"


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
// Function:	TBlock :: validate
// Description:
// Without reference to other blocks, see if the block looks reasonable.
//
void TBlock::validate() const
{
	// The way the proof of work system works is that the block makes a
	// claim of difficulty, which is essentially a 256-bit threshold.
	// The hash of the block must be less than this claimed difficulty
	// threshold for it to be valid.  The block hash is adjusted by the
	// generator by selection of the "nonce", of which the generator
	// tries billions until the block hash meets the difficulty target.
	//
	// The network imposes an additional requirement that blocks must
	// meet some minimum level of difficulty regardless of the currently
	// chosen difficulty level.  This is established by ensuring that
	// the block difficulty is less than a network-wide
	// ProofOfWorkLimit.
	//
	// Failing to meet either of these conditions means that the block
	// has an invalid proof of work.

	// The block hash must be lower than the claimed difficulty for the
	// proof of work to be valid.
	if( getHash() > getClaimedDifficulty() )
		throw block_chain_error_no_proof_of_work();

	// We only check the network limits if we have a network to check
	// against; it's possible to make blocks in isolation (the genesis
	// block templates).
	if( Pool != NULL && Pool->getNetwork() != NULL ) {
		const TBitcoinNetwork *Network = Pool->getNetwork();

		// TBitcoinHash can't be negative, so we only need to check the
		// upper limit
		if( getClaimedDifficulty() > Network->getNetworkParameters()->ProofOfWorkLimit )
			throw block_chain_error_too_easy();

		// The block can't be too far in the future
		if( getTimestamp() > Network->getNetworkTime() + Network->getNetworkParameters()->BLOCK_TIMESTAMP_WINDOW )
			throw block_chain_error_prescient();
	}

	// Check transactions
	// GetSigOpCount() > MAX_BLOCK_SIGOPS
	// Merkleroot
}

//
// Function:	TBlock :: fit
// Description:
// Like validate() but reference to other blocks is allowed
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
		// later.  This lets us record the child relationship in advance
		Parent = Pool->createBlock();
		Pool->putBlock( getParentHash(), Parent );
	}

	// We are a child of our parent, tell it so
	Parent->registerChild( this );

	// AcceptBlock
	//  - nBits != GetNextWorkRequired(pindexPrev)
	//  - GetBlockTime() <= pindexPrev->GetMedianTimePast()
	//  - Checkpoint
}

//
// Function:	TBlock :: getNextRequiredDifficulty
// Description:
//
TBitcoinHash TBlock::getNextRequiredDifficulty() const
{
	if( Pool == NULL || Pool->getNetwork() == NULL )
		throw logic_error( "Don't call TBlock::getNextTarget() on a detached block" );

	// Only change difficulty every N blocks; i.e. if the next block
	// number is divisible by the difficulty block interval
	if( getHeight() + 1 % Pool->getNetwork()->getNetworkParameters()->DifficultyUpdateInterval() != 0 ) {
		// No change means we should use our difficulty
		return getClaimedDifficulty();
	}

	// Follow the block chain backwards to find the first block in the
	// current difficulty chunk
	const TBlock *firstBlock = this;
	for( unsigned int i = Pool->getNetwork()->getNetworkParameters()->DifficultyUpdateInterval() - 1;
			firstBlock != NULL && i > 0; i-- ) {
		firstBlock = firstBlock->getParent();
	}
	// This should be impossible, our call to getHeight() should have
	// ensured that there are enough parents to follow
	if( firstBlock == NULL )
		throw logic_error( "TBlock::getNextTarget() failed to find start of difficulty chunk" );

	// Time between start and end
	time_t ObservedTimespan = getTimestamp() - firstBlock->getTimestamp();

	// Defer to network parameters to enforce rules
	ObservedTimespan = Pool->getNetwork()->getNetworkParameters()->limitDifficultyTimespan( ObservedTimespan );

	TBitcoinHash NextTarget( getClaimedDifficulty() );
	// Using the previous difficulty, that which should have taken
	// DIFFICULTY_TIMESPAN, actually took ObservedTimespan.  We therefore
	// adjust the current difficulty by ratio of the observed to the
	// target timespans.
	NextTarget *= static_cast<unsigned int>( ObservedTimespan );
	NextTarget /= Pool->getNetwork()->getNetworkParameters()->DIFFICULTY_TIMESPAN;

	// Limit to the smallest difficulty allowed
	if( NextTarget > Pool->getNetwork()->getNetworkParameters()->ProofOfWorkLimit )
		NextTarget = Pool->getNetwork()->getNetworkParameters()->ProofOfWorkLimit;

	return NextTarget;
}

//
// Function:	TBlock :: printOn
// Description:
//
ostream &TBlock::printOn( ostream &os ) const
{
	os << "Hash       : " << getHash() << endl;
	os << "Difficulty : " << getClaimedDifficulty() << endl;

	return os;
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
	cachedHash.invalidate();
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
void TMessageBasedBlock::updateFromMessage( const TMessage_block *m )
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

	// Invalidate any cached hash
	flush();

	validate();
}

//
// Function:	TMessageBasedBlock :: getHeight
// Description:
//
unsigned int TMessageBasedBlock::getHeight() const
{
	// If we've already calculated it, then return that
	if( cachedHeight != 0 )
		return cachedHeight;

	// If we are a genesis block, then our height is defined as zero
	if( getParentHash() == 0 ) {
		cachedHeight = 0;
	} else {
		// XXX: WARNING! This sort of recursion is going to fill the stack
		// up if we aren't pretty sure that most of our ancestors already
		// know their own height.
		cachedHeight = Parent->getHeight() + 1;
	}

	return cachedHeight;
}

//
// Function:	TMessageBasedBlock :: getHash
// Description:
//
const TBitcoinHash &TMessageBasedBlock::getHash() const
{
	// If we've already calculated it, then return that
	if( cachedHash.isValid() )
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
// Function:	TMessageBasedBlock :: getParentHash
// Description:
//
const TBitcoinHash &TMessageBasedBlock::getParentHash() const
{
	return Message->blockHeader().PreviousBlock;
}

//
// Function:	TMessageBasedBlock :: getClaimedDifficulty
// Description:
//
TBitcoinHash TMessageBasedBlock::getClaimedDifficulty() const
{
	return Message->blockHeader().DifficultyBits.getTarget();
}

//
// Function:	TMessageBasedBlock :: getTimestamp
// Description:
//
time_t TMessageBasedBlock::getTimestamp() const
{
	return Message->blockHeader().Timestamp.getValue();
}

//
// Function:	TMessageBasedBlock :: printOn
// Description:
//
ostream &TMessageBasedBlock::printOn( ostream &os ) const
{
	TBlock::printOn(os);
	os << "Message    : ";
	os << *Message;
	os << endl;

	return os;
}

// ---------

#if 0
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
void TDatabaseBlock::updateFromMessage( const TBitcoinHash &hash, const TMessage_block *m )
{
	if( m == NULL ) {
		// XXX: Delete existing record?
		return;
	}

	// XXX: Copy message to database
}
#endif

// ---------

//
// Function:	TBlockPool :: TBlockPool
// Description:
//
TBlockPool::TBlockPool( const TBitcoinNetwork *n ) :
	Network(n)
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
// Function:	TBlockPool :: receiveInventory
// Description:
//
void TBlockPool::receiveInventory( TMessage_inv *inv )
{
	TMessage_getdata *getdata = new TMessage_getdata;

	for( unsigned int i = 0; i < inv->size(); i++ ) {
		if( (*inv)[i].ObjectType != TInventoryElement::MSG_BLOCK )
			continue;

		// RX< inv
		if( blockExists( (*inv)[i].Hash.get() ) )
			continue;

		log() << "[BLKC] Block " << (*inv)[i].Hash.get()
			<< " not found in pool, requesting" << endl;

		// TX> getdata
		TInventoryElement &elem( getdata->appendInventory() );

		// Request the inventory item we were offered
		elem = (*inv)[i];

		// RX< block
		// Add blank to blockchain?
	}

	// Only send the request if it's got any requests in it
	if( getdata->size() > 0 ) {
//		log() << "[NETW] NTX> " << *getdata << endl;
		inv->getPeer()->queueOutgoing( getdata );
	} else {
		delete getdata;
	}
}

//
// Function:	TBlockPool :: receiveBlock
// Description:
//
void TBlockPool::receiveBlock( const TMessage_block *message )
{
	// Create a new block
	TBlock *thisBlock = createBlock();

	// Show it the message (this allows it to calculate its hash), and
	// throw an exception if the network hash doesn't equal the
	// calculated hash
	try {
		thisBlock->updateFromMessage( message );
	} catch( ... ) {
		delete thisBlock;
		throw;
	}

	// See if we already have this block
	TBlock *existingBlock = getBlock( thisBlock->getHash() );
	if( existingBlock != NULL ) {
		// If so, discard the received update, and use the pooled
		// version instead.  We do it this way around, as the already
		// stored version can have information we might not have to
		// hand; but we can easily supply that stored version the
		// information we _do_ have to hand.
		delete thisBlock;
		thisBlock = existingBlock;
		// Let the existing block have a look at the message as well
		thisBlock->updateFromMessage( message );
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
TBlockMemoryPool::TBlockMemoryPool( const TBitcoinNetwork *n ) :
	TBlockPool(n)
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
void TBlockMemoryPool::putBlock( const TBitcoinHash &Hash, TBlock *Block )
{
	iterator it;

	it = Pool.find( Hash );

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
TBlock *TBlockMemoryPool::getBlock( const TBitcoinHash &hash ) const
{
	const_iterator it;

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
bool TBlockMemoryPool::blockExists( const TBitcoinHash &hash ) const
{
	return (Pool.count( hash ) != 0);
}

//
// Function:	TBlockMemoryPool :: scanForNewChildLinks
// Description:
//
void TBlockMemoryPool::scanForNewChildLinks()
{
	iterator it;

	for( it = Pool.begin(); it != Pool.end(); it++ ) {
		TBlock *CurrentBlock = it->second;
		if( CurrentBlock == NULL )
			continue;

		TBlock *ParentBlock = getBlock( CurrentBlock->getParentHash() );
		if( ParentBlock != NULL )
			continue;

		// Tell the parent about the child
		ParentBlock->registerChild( CurrentBlock );
	}
}

// ---------

#if 0
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

//
// Function:	TDatabaseBlockPool :: scanForNewChildLinks
// Description:
//
void TDatabaseBlockPool::scanForNewChildLinks()
{
}
#endif


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

