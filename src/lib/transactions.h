// ----------------------------------------------------------------------------
// Project: bitcoin
/// @file   transactions.h
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
#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

// -------------- Includes
// --- C
// --- C++
#include <map>
// --- Qt
// --- OS
// --- Project lib
// --- Project
#include "messageelements.h"
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
class TCoinTransfer;
class TMessage_inv;
class TMessage_block;
class TMessage_tx;


// -------------- Function pre-class prototypes


// -------------- Class declarations

//
// Class:	TTransferBeneficiary
// Description:
//
// There are different ways that the destination of a transaction can be
// specified.
//
// This class's job is represent any of them in an abstract manner.
// This will enable us to distinguish between targeting a bitcoin
// address or an IP address, or any other that comes up in the future.
//
class TTransferBeneficiary
{
  public:
	TTransferBeneficiary() {}
};

//
// Class:	TCoinTransfer
// Description:
// Object representing transaction outputs, and what happened to them.
//
// Every transaction has some outputs, each output is represented by
// an object of this type.  Eventually that output will be used, when it
// is, that is recorded in this object.  Note: this is specifically not
// recording inputs and outputs -- there is no such thing as an input;
// a transaction input is only a reference to an earlier output.
//
// The authorisation for use is done with two scripts.  The first is on
// the creation side.  It specifies what conditions must be met by the
// claimant to allow the expenditure.  The second is the supply of
// whatever that authorisation script requires.  The most common case is
// that the AuthorisationScript specifies a public key and requires a
// signature using that public key; then the ClaimantScript supplies
// that signature.
//
class TCoinTransfer
{
  public:
	struct sSplitReference {
		TBitcoinHash TransactionHash;
		unsigned int SplitIndex;
	};
  public:
	TCoinTransfer();

	virtual sSplitReference getCreatorReference() const = 0;
	virtual sSplitReference getClaimantReference() const = 0;
	virtual const TTransferBeneficiary *getBeneficiary() const = 0;
	virtual bool confirmationAvailable() const = 0;
	virtual bool confirmed() const = 0;

	virtual bool isCoinbase() const = 0;

	virtual void validate() = 0;

//	bool spendAttempt( TTransactionElement & );
//	void clearSpend();

  protected:
};

//
// Class:	TMemoryCoinTransfer
// Description:
//
class TMemoryCoinTransfer : public TCoinTransfer
{
  public:
	TMemoryCoinTransfer() : State(ScriptNotRun), Beneficiary( NULL ) {}

	sSplitReference getCreatorReference() const { return Creation; }
	sSplitReference getClaimantReference() const { return Claim; }
	const TTransferBeneficiary *getBeneficiary() const { return Beneficiary; }

	bool confirmationAvailable() const { return State != ScriptNotRun; }
	bool confirmed() const { return State == ConfirmedValid; }

	bool isCoinbase() const;

	void validate();

  protected:
	// The output we represent (we are an "output" of this transaction)
	sSplitReference Creation;
	// The script that gives the conditions for our use
	string AuthorisationScript;

	// The transaction that "claims" us (we are an "input" of this
	// transaction)
	sSplitReference Claim;
	// The script that purports to meet the conditions specified in
	// AuthorisationScript.  If it doesn't meet the conditions, then
	// these members will get cleared pretty quickly.  This script is
	// essentially the input to the AuthorisationScript.
	string ClaimantScript;

	// ... details of the spend
	unsigned int SpentSequenceNumber;
	TCoinsElement Coins;

	enum {
		ScriptNotRun,
		ConfirmedValid,
		ConfirmedInvalid
	} State;

	TTransferBeneficiary *Beneficiary;
};

// --------------

//
// Class:	TTransaction
// Description:
//
class TTransaction
{
  public:
	virtual ~TTransaction();

	virtual const TBitcoinHash &getHash() const = 0;

//	virtual void putOutput( unsigned int, TCoinTransfer * ) = 0;
//	virtual TCoinTransfer *getOutput( unsigned int ) const = 0;
//	virtual bool outputExists( unsigned int ) const = 0;

	virtual TCoinTransfer *createTransfer() const = 0;

//	virtual TCoinElement sumInputs() const = 0;
//	virtual TCoinElement sumOutputs() const = 0;
};

//
// Class:	TMessageBasedTransaction
// Description:
//
class TMessageBasedTransaction : public TTransaction
{
  public:
	~TMessageBasedTransaction();

	const TBitcoinHash &getHash() const;

	TCoinTransfer *createTransfer() const;

  protected:
	mutable TBitcoinHash cachedHash;

	map<unsigned int, TCoinTransfer *> Inputs;
	map<unsigned int, TCoinTransfer *> Outputs;
};

// --------------

//
// Class:	TTransactionPool
// Description:
//
// Despite the fact that this is called a TTransactionPool, the more
// important structure here is the transfer pool.  The transactions are
// really a way of grouping together transfers, but it is the transfers
// that form a chain, not the transactions.
//
// Each transfer input to a transaction must be the output of an earlier
// transfer. There is one special case, the coinbase outputs which
// reference no other transaction and are the root of any particular
// chain.
//
class TTransactionPool
{
  public:
	TTransactionPool();
	virtual ~TTransactionPool();

	virtual void putTransaction( const TBitcoinHash &, TTransaction * ) = 0;
	virtual TTransaction *getTransaction( const TBitcoinHash & ) const = 0;
	virtual bool transactionExists( const TBitcoinHash & ) const = 0;

	void receiveFromMessage( const TMessage_inv * );
	void receiveFromMessage( const TMessage_block * );
	void receiveFromMessage( const TBitcoinHash &, const TMessage_tx * );

  protected:
	virtual TTransaction *createTransaction() const = 0;
};

//
// Class:	TMemoryTransactionPool
// Description:
//
class TMemoryTransactionPool
{
  public:
	TMemoryTransactionPool();
	~TMemoryTransactionPool();

	void putTransaction( const TBitcoinHash &, TTransaction * );
	TTransaction *getTransaction( const TBitcoinHash & ) const;
	bool transactionExists( const TBitcoinHash & ) const;

	void validate();

  protected:
	virtual TTransaction *createTransaction() const;

  protected:
	map<TBitcoinHash, TTransaction*> TransactionPool;
};


// -------------- Constants


// -------------- Inline Functions


// -------------- Function prototypes


// -------------- Template instantiations


// -------------- World globals ("extern"s only)


// End of conditional compilation
#endif
