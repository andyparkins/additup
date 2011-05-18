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
// --- Project
#include "script.h"


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Template instantiations


// -------------- Class declarations


// -------------- Class member definitions

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
// Function:	TMessageBasedTransaction :: createTransfer
// Description:
//
TCoinTransfer *TMessageBasedTransaction::createTransfer() const
{
	return new TMemoryCoinTransfer();
}

//
// Function:	TMessageBasedTransaction :: getHash
// Description:
//
const TBitcoinHash &TMessageBasedTransaction::getHash() const
{
	return cachedHash;
}

// -------------

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

//
// Function:	TMemoryCoinTransfer :: isCoinbase
// Description:
//
bool TMemoryCoinTransfer::isCoinbase() const
{
//	return Creation.TransactionHash == Network->getParameters()->COINBASE_REFERENCE_HASH
//		&& Creation.SplitIndex == Network->getParameters()->COINBASE_REFERENCE_INDEX

	return false;
}

// ------

//
// Function:	TTransactionPool :: TTransactionPool
// Description:
//
TTransactionPool::TTransactionPool()
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
// Function:	TTransactionPool :: receiveFromMessage
// Description:
//
void TTransactionPool::receiveFromMessage( const TMessage_inv *Message )
{
}

//
// Function:	TTransactionPool :: receiveFromMessage
// Description:
//
void TTransactionPool::receiveFromMessage( const TMessage_block *Message )
{
}

//
// Function:	TTransactionPool :: receiveFromMessage
// Description:
//
void TTransactionPool::receiveFromMessage( const TBitcoinHash &Hash, const TMessage_tx *Message )
{
}

// --------

//
// Function:	TMemoryTransactionPool :: TMemoryTransactionPool
// Description:
//
TMemoryTransactionPool::TMemoryTransactionPool()
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
TTransaction *TMemoryTransactionPool::createTransaction() const
{
	return new TMessageBasedTransaction();
}

//
// Function:	TMemoryTransactionPool :: putTransaction
// Description:
//
void TMemoryTransactionPool::putTransaction( const TBitcoinHash &NetworkHash, TTransaction *Transaction )
{
	map<TBitcoinHash, TTransaction*>::iterator it;

	it = TransactionPool.find( Transaction->getHash() );

	if( it == TransactionPool.end() ) {
		TransactionPool[Transaction->getHash()] = Transaction;
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

