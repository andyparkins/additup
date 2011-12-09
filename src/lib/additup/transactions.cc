// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   transactions.cc
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
#include "transactions.h"

// -------------- Includes
// --- C
// --- C++
#include <sstream>
// --- Qt
// --- OS
// --- Project libs
#include <general/logstream.h>
// --- Project
#include "script.h"
#include "messages.h"
#include "peer.h"
#include "bitcoinnetwork.h"


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Template instantiations


// -------------- Class declarations


// -------------- Class member definitions

//
// Function:	TCoinTransfer :: TCoinTransfer
// Description:
//
TCoinTransfer::TCoinTransfer( const TTransaction *, unsigned int )
{
}

// -------------

//
// Function:	TTransaction :: ~TTransaction
// Description:
//
TTransaction::~TTransaction()
{
}

// -------------

//
// Function:	TTransaction :: TTransaction
// Description:
//
TTransaction::TTransaction( TTransactionPool *p ) :
	TransactionPool(p),
	Priority( 0 )
{
}

// -------------

//
// Static:	TMessageBasedTransaction :: NULL_REFERENCE
// Description:
// A transaction chain starts at a coinbase transaction; but a coinbase
// transaction has one input.  That input is a TMessageBasedTransaction object; but
// there is no transaction to which it can refer, that's what makes it a
// coinbase transaction.  This static member of TMessageBasedTransaction supplies
// a place for that coinbase transaction to point at to represent the
// transfer-from-nowhere that is a coinbase.
//
TOutputReference TMessageBasedTransaction::NULL_REFERENCE;

//
// Function:	TMessageBasedTransaction :: TMessageBasedTransaction
// Description:
//
TMessageBasedTransaction::TMessageBasedTransaction( TMemoryTransactionPool *p ) :
	TTransaction(p)
{
}

//
// Function:	TMessageBasedTransaction :: ~TMessageBasedTransaction
// Description:
//
TMessageBasedTransaction::~TMessageBasedTransaction()
{
	while( !Outputs.empty() ) {
		delete Outputs.begin()->second;
		Outputs.erase( Outputs.begin() );
	}
}

//
// Function:	TMessageBasedTransaction :: createOutput
// Description:
//
TCoinTransfer *TMessageBasedTransaction::createOutput( unsigned int n )
{
	Outputs[n] = new TMemoryCoinTransfer( this, n );
	return Outputs[n];
}

//
// Function:	TMessageBasedTransaction :: getHash
// Description:
//
const TBitcoinHash &TMessageBasedTransaction::getHash() const
{
	if( cachedHash.isValid() )
		return cachedHash;

	map<unsigned int, TCoinTransfer *>::const_iterator it;

	it = Inputs.begin();
	if( it == Inputs.end() ) {
		cachedHash.invalidate();
		// XXX: This isn't right, we need to calculate it when there are
		// no inputs
	} else {
		// All our inputs must point at us as their claimant, we can simply
		// return one of those hashes.
		cachedHash = it->second->getClaimantReference().TransactionHash;
	}

//	TTransactionElement Transaction;
//
//	Transaction.Version = 1;
//	Transaction.LockTime = 0;
//
//	// Our inputs array is a list of coin transfers; those transfers
//	// show a creator transaction and a claimant transaction.  This
//	// transaction is the claimant transaction, so for our inputs we are
//	// interested in where that input came from: the creator transaction
//	for( unsigned int i = 0; i < Inputs.size(); i++ ) {
//		TInputSplitElement &Input( Transaction.createInput() );
//		Input.Outpoint.TransactionHash = Inputs[i]->getCreatorReference().TransactionHash;
//		Input.Outpoint.TransactionHash = Inputs[i]->getCreatorReference().OutputIndex;
//		Input.SignatureScript = Inputs[i]->ClaimantScript;
//		Input.Sequence = Inputs[i]->SpentSequenceNumber;
//
//		// Should we choose to, we could check that the Input[i] coin
//		// transfer has us as its target.
//	}

	return cachedHash;
}

//
// Function:	TMessageBasedTransaction :: isCoinbase
// Description:
// A coinbase transaction has only one input, and that input is a NULL
// reference.  The output is the beneficiary of the coinbase.
//
bool TMessageBasedTransaction::isCoinbase() const
{
	return Inputs.size() == 1
		&& Inputs.begin()->second->getCreatorReference().isNull();
}

// -------------

//
// Function:	TMemoryCoinTransfer :: TMemoryCoinTransfer
// Description:
//
TMemoryCoinTransfer::TMemoryCoinTransfer( const TTransaction *t, unsigned int n ) :
	TCoinTransfer( t, n ),
	State(ScriptNotRun),
	Beneficiary( NULL )
{
	Creation.TransactionHash = t->getHash();
	Creation.OutputIndex = n;
}

//
// Function:	TMemoryCoinTransfer :: validate
// Description:
//
void TMemoryCoinTransfer::validate()
{
	istringstream AScript( AuthorisationScript );
	istringstream CScript( ClaimantScript );
	TBitcoinScript *Script = NULL;
	TExecutionContext Stack;

	// The script we execute is made from two components, the claim
	// script comes first, and generally supplies the arguments that the
	// authorisation script requires
	Script->read( CScript, TBitcoinScript::ClaimantScript );
	Script->read( AScript, TBitcoinScript::AuthorisationScript );
}

// ------

//
// Function:	TTransactionPool :: TTransactionPool
// Description:
//
TTransactionPool::TTransactionPool( const TBitcoinNetwork *n ) :
	Network( n )
{
}

//
// Function:	TTransactionPool :: ~TTransactionPool
// Description:
//
TTransactionPool::~TTransactionPool()
{
}

//
// Function:	TTransactionPool :: receiveInventory
// Description:
//
void TTransactionPool::receiveInventory( TMessage_inv *inv )
{
	TMessage_getdata *getdata = new TMessage_getdata;

	for( unsigned int i = 0; i < inv->size(); i++ ) {
		if( (*inv)[i].ObjectType != TInventoryElement::MSG_TX )
			continue;

		// RX< inv
		if( transactionExists( (*inv)[i].Hash.get() ) )
			continue;

		log() << "[NETW] Transaction " << (*inv)[i].Hash.get()
			<< " not found in pool, requesting" << endl;

		// TX> getdata
		TInventoryElement &elem( getdata->appendInventory() );

		// Request the inventory item we were offered
		elem = (*inv)[i];

		// RX< tx
		// Make space for it
		putTransaction( (*inv)[i].Hash.get(), NULL );
	}

	// Only send the request if it's got any requests in it
	if( getdata->size() > 0 ) {
		inv->getPeer()->queueOutgoing( getdata );
	} else {
		delete getdata;
	}
}

//
// Function:	TTransactionPool :: receiveBlock
// Description:
//
void TTransactionPool::receiveBlock( const TMessage_block *Message )
{
}

//
// Function:	TTransactionPool :: receiveTransaction
// Description:
//
void TTransactionPool::receiveTransaction( const TMessage_tx *tx )
{

}

// --------

//
// Function:	TMemoryTransactionPool :: TMemoryTransactionPool
// Description:
//
TMemoryTransactionPool::TMemoryTransactionPool( const TBitcoinNetwork *n ) :
	TTransactionPool( n )
{
}

//
// Function:	TMemoryTransactionPool :: ~TMemoryTransactionPool
// Description:
//
TMemoryTransactionPool::~TMemoryTransactionPool()
{
	// delete the pools
	while( !TransactionPool.empty() ) {
		delete TransactionPool.begin()->second;
		TransactionPool.erase( TransactionPool.begin() );
	}
}

//
// Function:	TMemoryTransactionPool :: createTransaction
// Description:
//
TTransaction *TMemoryTransactionPool::createTransaction()
{
	return new TMessageBasedTransaction( this );
}

//
// Function:	TMemoryTransactionPool :: putTransaction
// Description:
//
void TMemoryTransactionPool::putTransaction( const TBitcoinHash &NetworkHash, TTransaction *Transaction )
{
	map<TBitcoinHash, TTransaction*>::iterator it;

	it = TransactionPool.find( NetworkHash );

	if( it == TransactionPool.end() ) {
		TransactionPool[NetworkHash] = Transaction;
	} else {
		// Replace the existing transaction -- it's the caller's duty to
		// update rather than replace if that's their wish
		delete it->second;
		it->second = Transaction;
	}
}

//
// Function:	TMemoryTransactionPool :: getTransaction
// Description:
//
TTransaction *TMemoryTransactionPool::getTransaction( const TBitcoinHash &hash ) const
{
	map<TBitcoinHash, TTransaction*>::const_iterator it;

	it = TransactionPool.find( hash );
	if( it == TransactionPool.end() ) {
		return NULL;
	} else {
		return it->second;
	}
}

//
// Function:	TMemoryTransactionPool :: transactionExists
// Description:
//
bool TMemoryTransactionPool::transactionExists( const TBitcoinHash &hash ) const
{
	map<TBitcoinHash, TTransaction*>::const_iterator it;

	it = TransactionPool.find( hash );
	return (it != TransactionPool.end());
}


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

