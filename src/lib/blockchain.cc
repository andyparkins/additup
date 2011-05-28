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
#include <sstream>
// --- Qt
// --- OS
// --- Project libs
// --- Project
#include "messages.h"
#include "bitcoinnetwork.h"
#include "logstream.h"
#include "crypto.h"


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
// Function:	TBlock :: getChildOnBranch
// Description:
//
// Consider this chain:
//   1 -- 2 -- this -- 3 -- branch1
//         \       \    \                (not multiline comment)
//          \       \    4 -- branch3
//           \       \                   (not multiline comment)
//            \       5 -- branch2
//             \                         (not multiline comment)
//              6 -- 7 -- branch4
//
// this->getChildOnBranch( branch1 ) returns 3.
// this->getChildOnBranch( branch2 ) returns 5.
// this->getChildOnBranch( branch3 ) returns 3.
// this->getChildOnBranch( branch4 ) returns NULL.
//
const TBlock *TBlock::getChildOnBranch( const TBlock *branch ) const
{
	set<TBitcoinHash>::const_iterator it;

	// getCommonAncestor( 3, branch1 ) == 3
	// getCommonAncestor( 5, branch1 ) == this
	// getCommonAncestor( 3, branch2 ) == this
	// getCommonAncestor( 5, branch2 ) == 5
	// getCommonAncestor( 3, branch3 ) == 3
	// getCommonAncestor( 5, branch3 ) == this
	// getCommonAncestor( 3, branch4 ) == 2
	// getCommonAncestor( 5, branch4 ) == 2
	for( it = ChildHashes.begin(); it != ChildHashes.end(); it++ ) {
		const TBlock *child = Pool->getBlock( *it );
		const TBlock *commonAncestor = Pool->getCommonAncestor( child, branch );

		// If the child is the common ancestor of the branch, then it is
		// on that branch
		if( commonAncestor == child ) {
			return child;
		}
	}

	return NULL;
}

//
// Function:	TBlock :: isAncestorOf
// Description:
// See if the given block is a child of us.
//
// In this chain:
//   o -- this -- o -- o -- o -- that
// this _is_ an ancestor of that.
//
// In this chain:
//   o -- that -- o -- o -- o -- this
// this is _not_ an ancestor of that.
//
// In this chain:
//   o -- o -- o -- this
//         \                    (not a multiline comment)
//          o -- o -- that
// neither this nor that is an ancestor of the other.
//
bool TBlock::isAncestorOf( const TBlock *that ) const
{
	// We can use getCommonAncestor() to sort this out, since the only
	// case where we return true is if the common ancestor is ourselves.
	const TBlock *commonAncestor = Pool->getCommonAncestor( this, that );
	if( commonAncestor == this )
		return true;

	return false;
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
	TBlock( p )
{
	cachedHash.invalidate();
}

//
// Function:	TMessageBasedBlock :: ~TMessageBasedBlock
// Description:
//
TMessageBasedBlock::~TMessageBasedBlock()
{
}

//
// Function:	TMessageBasedBlock :: setMessage
// Description:
//
void TMessageBasedBlock::updateFromHeader( const TBlockHeaderElement &H )
{
	Header = H;

	// Invalidate any cached hash
	flush();

	validate();
}

//
// Function:	TMessageBasedBlock :: writeToHeader
// Description:
//
void TMessageBasedBlock::writeToHeader( TBlockHeaderElement &H ) const
{
	H = Header;
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

	ostringstream oss;

	// "The SHA256 hash that identifies each block (and which must have
	// a run of 0 bits) is calculated from the first 6 fields of this
	// structure (version, prev_block, merkle_root, timestamp, bits,
	// nonce, and standard SHA256 padding, making two 64-byte chunks in
	// all) and not from the complete block. To calculate the hash, only
	// two chunks need to be processed by the SHA256 algorithm. Since
	// the nonce  field is in the second chunk, the first chunk stays
	// constant during mining and therefore only the second chunk needs
	// to be processed. However, a Bitcoin hash is the hash of the hash,
	// so two SHA256 rounds are needed for each mining iteration."
	Header.write(oss);

	// Field sizes: 4 + 32 + 32 + 4 + 4 + 4 = 80
	// OpenSSL should pad on its own...

	if( Pool == NULL )
		return cachedHash;

//	log() << "TMessage_block = ";
//	TLog::hexify( log(), oss.str() );
//	log() << endl;
	cachedHash.fromBytes(
			Pool->getNetwork()->getNetworkParameters()->blockHasher()->transform( oss.str() ) );

	// For an unknown reason, bitcoin calculates the hash, then reverses
	// the byte order, and that reversed form is then treated as the
	// hash
	cachedHash = cachedHash.reversedBytes();

//	log() << "TMessage_block.hash = ";
//	TLog::hexify( log(), hash.toBytes() );
//	log() << endl;

	return cachedHash;
}

//
// Function:	TMessageBasedBlock :: getParentHash
// Description:
//
const TBitcoinHash &TMessageBasedBlock::getParentHash() const
{
	return Header.PreviousBlock;
}

//
// Function:	TMessageBasedBlock :: getClaimedDifficulty
// Description:
//
TBitcoinHash TMessageBasedBlock::getClaimedDifficulty() const
{
	return Header.DifficultyBits.getTarget();
}

//
// Function:	TMessageBasedBlock :: getTimestamp
// Description:
//
time_t TMessageBasedBlock::getTimestamp() const
{
	return Header.Timestamp.getValue();
}

//
// Function:	TMessageBasedBlock :: printOn
// Description:
//
ostream &TMessageBasedBlock::printOn( ostream &os ) const
{
	TBlock::printOn(os);
//	os << "Message    : ";
//	os << *Message;
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
void TDatabaseBlock::updateFromHeader( const TBitcoinHash &hash, const TMessage_block *m )
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
// Function:	TBlockPool :: getCommonAncestor
// Description:
//
// Given the following pool:
//
//   o -- o -- o -- A -- o -- o -- b1 -- o -- o
//                   \                             (not multiline comment)
//                    o -- o -- b2 -- o -- o
//
// This function will return A, given b1 and b2.
//
// Given the following pool:
//
//   o -- o -- o -- b2 -- o -- o -- b1 -- o
//
// This function will return b2.
//
const TBlock *TBlockPool::getCommonAncestor( const TBlock *b1, const TBlock *b2 ) const
{
	set<const TBlock*> Seenpool;

	// Consider
	//
	//  1 -- 2 -- 3 -- 4 -- 5
	//        \                           (not multiline comment)
	//         6 -- 7 -- 8 -- 9
	//
	// We'll add blocks to a set until we find one we've already seen
	//   +5 +9
	//   +4 +8
	//   +3 +7
	//   +2 +6
	//    1  2  (have already seen 2, so that is the common ancestor)
	//

	// Keep pushing b1 and b2 parents until one is found in the other
	while( b1 != NULL || b2 != NULL ) {
		// If b1 is already in the pool, then b1 is the common root
		if( Seenpool.count( b1 ) != 0 )
			return b1;
		if( Seenpool.count( b2 ) != 0 )
			return b2;

		// Insert the current blocks into the sub pool
		if( b1 != NULL )
			Seenpool.insert( b1 );
		if( b2 != NULL )
			Seenpool.insert( b2 );

		// Move back a parent
		if( b1 != NULL )
			b1 = b1->getParent();
		if( b2 != NULL )
			b2 = b2->getParent();
	}

	return NULL;
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
		// Add blank to blockchain
		putBlock( (*inv)[i].Hash.get(), NULL );
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
// Function:	TBlockPool :: receiveHeader
// Description:
//
void TBlockPool::receiveHeader( const TBlockHeaderElement &Header )
{
	// Create a new block
	TBlock *thisBlock = createBlock();

	// Show it the message (this allows it to calculate its hash), and
	// throw an exception if the network hash doesn't equal the
	// calculated hash
	try {
		thisBlock->updateFromHeader( Header );
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
		thisBlock->updateFromHeader( Header );
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
	Branches.erase( thisBlock->getParentHash() );
	// (2) If this block has no children, then it can be a chain tip (at
	// least until we find otherwise)
	if( !thisBlock->hasChildren() )
		Branches.insert( thisBlock->getHash() );

	// With these rules in place, the Branches array represents all blocks
	// in the pool that have no children.  Once the full block chain has
	// been downloaded this should be a very limited set.
}

//
// Function:	TBlockPool :: receiveBlock
// Description:
//
void TBlockPool::receiveBlock( const TMessage_block *message )
{
	receiveHeader( message->blockHeader() );
}

//
// Function:	TBlockPool :: queueBlock
// Description:
//
void TBlockPool::queueBlock( TBitcoinPeer *Peer, const TBlock *Block ) const
{
	TMessage_block *msgblock;
	msgblock = new TMessage_block;
	msgblock->setPeer( Peer );

	Block->writeToHeader( msgblock->blockHeader() );

	// XXX: That's the header done, but how do we get the transactions
	// in?

	Peer->queueOutgoing( msgblock );
}

//
// Function:	TBlockPool :: queueBlock
// Description:
//
void TBlockPool::queueBlock( TBitcoinPeer *Peer, const TBitcoinHash &Hash ) const
{
	queueBlock( Peer, getBlock( Hash ) );
}

//
// Function:	TBlockPool :: receiveHeaders
// Description:
//
void TBlockPool::receiveHeaders( const TMessage_headers *headers )
{
	for( unsigned int i = 0; i < headers->size(); i++ ) {
		receiveHeader( (*headers)[i] );
	}
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
		Pool[Hash] = Block;
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
#include "constants.h"
#include "messageelements.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
	} catch( exception &e ) {
		cerr << e.what() << endl;
		return 255;
	}

	try {
		// Force KNOWN_NETWORKS creation
		KNOWN_NETWORKS::create();

		log() << "--- Hash speed test" << endl;
		TMessageBasedBlock *testblock;

		testblock = dynamic_cast<TMessageBasedBlock*>( NETWORK_PRODNET->GenesisBlock->clone() );

		log() << "Loaded testblock" << endl;
		testblock->printOn( log() );

		log() << "Hashing";

		static const unsigned int LOOPS = 1 << 18;
		struct timeval start, end;
		unsigned int i;
		TBlockHeaderElement TestBlockHeader;
		testblock->writeToHeader( TestBlockHeader );
		TBitcoinHash hash;
		gettimeofday( &start, NULL );
		for( i = LOOPS; i > 0; i-- ) {
			testblock->flush();
			hash = testblock->getHash();
			if( (i & 0xfff) == 0 )
				log() << "." << flush;
		}
		gettimeofday( &end, NULL );

		log() << endl;

//		log() << LOOPS << " loops in " << (end.tv_sec - start.tv_sec)
//			<< "s and " << (end.tv_usec - start.tv_usec) << "us" << endl;

		log() << "Hash rate is " << LOOPS / (end.tv_sec - start.tv_sec) << " h/s (approx)" << endl;

		TBitcoinHash Target(1);
		Target <<= (256 - 19);
		Target -= 1;

		TestBlockHeader.Nonce = 0;

		log() << "Mining";
		gettimeofday( &start, NULL );
		i = 0;
		while( true ) {
			testblock->flush();
			hash = testblock->getHash();
			if( (i & 0xfff) == 0 )
				log() << "." << flush;
			if( hash <= Target )
				break;
			i++;
			TestBlockHeader.Nonce++;
		}
		gettimeofday( &end, NULL );

		log() << endl;
		log() << "Found hash  " << hash << endl
			<< " less than  " << Target << endl;

	} catch( std::exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

