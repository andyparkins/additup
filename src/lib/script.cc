// ----------------------------------------------------------------------------
// Project: additup
/// @file   script.cc
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
#include "script.h"

// -------------- Includes
// --- C
// --- C++
#include <iostream>
#include <memory>
#include <iterator>
#include <sstream>
// --- Qt
// --- OS
// --- Project libs
// --- Project
#include "logstream.h"
#include "crypto.h"


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Class declarations


// -------------- Class declarations

//
// Function:	TStackElement_t :: printOn
// Description:
//
template <typename t>
ostream &TStackElement_t<t>::printOn( ostream &s ) const
{
	s << Data;
	return s;
}

//
// Function:	TStackElement_t :: printOn
// Description:
//
template <>
ostream &TStackElement_t<string>::printOn( ostream &s ) const
{
	TLog::hexify( s, Data );
	return s;
}

// -----------

//
// Function:	TExecutionContext :: TExecutionContext
// Description:
//
TExecutionContext::TExecutionContext() :
	Transaction( NULL ),
	Invalid( false )
{
}

//
// Function:	TExecutionContext :: printOn
// Description:
//
ostream &TExecutionContext::printOn( ostream &s ) const
{
	list<TStackElement*>::const_iterator it;
	for( it = Stack.begin(); it != Stack.end(); it++ ) {
		s << " - ";
		(*it)->printOn(s);
		s << endl;
	}

	return s;
}

//
// Function:	TExecutionContext :: setTransaction
// Description:
//
void TExecutionContext::setTransaction( TTransaction *tx )
{
	Transaction = tx;
}

// -----------

//
// Function:	TBitcoinScriptBase :: TBitcoinScriptBase
// Description:
//
TBitcoinScriptBase::TBitcoinScriptBase() :
	Initialised( false )
{
}

//
// Function:	TBitcoinScriptBase :: TBitcoinScriptBase
// Description:
//
TBitcoinScriptBase::TBitcoinScriptBase( const TStackOperator **OperatorArray, unsigned int N ) :
	Initialised( false )
{
	for( unsigned int i = 0; i < N; i++ ) {
		// Note: we use push_back() not append() because this is the
		// only way to get explodable operators into a script.  This
		// constructor is intended to allow the creation of writable
		// scripts rather than runnable scripts.
		Program.push_back( OperatorArray[i]->clone() );
	}
}

//
// Function:	TBitcoinScriptBase :: ~TBitcoinScriptBase
// Description:
//
TBitcoinScriptBase::~TBitcoinScriptBase()
{
	while( !Program.empty() ) {
		delete Program.front();
		Program.erase( Program.begin() );
	}
}

//
// Function:	TBitcoinScriptBase :: init
// Description:
//
void TBitcoinScriptBase::init()
{
	Initialised = true;
}

//
// Function:	TBitcoinScriptBase :: execute
// Description:
//
void TBitcoinScriptBase::execute( TExecutionContext &Stack ) const
{
	tInstructionPointer it, itn;
	it = Program.begin();
	while( it != Program.end() ) {
		itn = (*it)->execute( Stack, it );
		log() << (*it)->className() << " executed, stack now:" << endl;
		Stack.printOn(log());

		// Catch infinite loop
		if( it == itn ) {
			it++;
		} else {
			it = itn;
		}
	}
}

//
// Function:	TBitcoinScriptBase :: append
// Description:
//
void TBitcoinScriptBase::append( TStackOperator *op )
{
	if( dynamic_cast<TStackOperatorFromCompoundOpcode*>(op) != NULL ) {
		// Don't append this operator, it's a compound code, instead
		// explode it, which will in turn call us again.
		dynamic_cast<TStackOperatorFromCompoundOpcode*>(op)->explode( this );

		// XXX: Who is going to delete "op" if we don't?
	} else {
		Program.push_back(op);
	}
}

//
// Function:	TBitcoinScriptBase :: printOn
// Description:
//
ostream &TBitcoinScriptBase::printOn( ostream &os ) const
{
	list<TStackOperator*>::const_iterator it;
	unsigned int i = 0;

	it = Program.begin();
	while( it != Program.end() ) {
		os << i << ". ";
		(*it)->printOn( os );
		os << endl;

		it++;
		i++;
	}
	return os;
}

// -----------

//
// Function:	TBitcoinScript :: TBitcoinScript
// Description:
//
TBitcoinScript::TBitcoinScript( const TStackOperator **a, unsigned int n ) :
	TBitcoinScriptBase(a,n)
{
}

//
// Function:	TBitcoinScript :: TBitcoinScript
// Description:
//
TBitcoinScript::TBitcoinScript( const string &s, eReadMode a )
{
	istringstream iss(s);
	read(iss, a);
}

//
// Function:	TBitcoinScript :: ~TBitcoinScript
// Description:
//
TBitcoinScript::~TBitcoinScript()
{
	while( !ClaimantTemplates.empty() ) {
		delete ClaimantTemplates.front();
		ClaimantTemplates.erase( ClaimantTemplates.begin() );
	}
	while( !AuthorisationTemplates.empty() ) {
		delete AuthorisationTemplates.front();
		AuthorisationTemplates.erase( AuthorisationTemplates.begin() );
	}
}

//
// Function:	TBitcoinScript :: read
// Description:
//
istream &TBitcoinScript::read( istream &is, eReadMode ReadMode )
{
	eScriptOp Opcode;
	list<const TStackOperatorFromStream *>::const_iterator it;
	list<const TStackOperatorFromStream *> *Templates;

	// Load the template list
	if( !Initialised )
		init();

	// Peek at each opcode, then ask each template if it will accept
	// that opcode.  Finding one that does, we clone it and let it read
	// itself from the stream.
	while( (Opcode = static_cast<eScriptOp>(is.peek()) ) != ios::traits_type::eof() ) {
		switch( ReadMode ) {
			case AuthorisationScript:
				Templates = &AuthorisationTemplates;
				break;
			case ClaimantScript:
				Templates = &ClaimantTemplates;
				break;
			default:
				Templates = NULL;
		}

		// We refuse to read
		if( Templates == NULL )
			break;

		for( it = Templates->begin(); it != Templates->end(); it++ ) {
			if( !(*it)->acceptOpcode( Opcode ) )
				continue;
			break;
		}
		if( it == Templates->end() ) {
//			log() << "script opcode (" << Opcode << ") read but not recognised" << endl;
			throw script_parse_error_not_found();
		}
//		log() << "Reading " << (*it)->className() << endl;
		(*it)->readAndAppend( this, is );

		// Should leave us pointing at next character
	}

//	list<TStackOperator *>::iterator it2;
//	log() << "Read script:" << endl;
//	for( it2 = Program.begin(); it2 != Program.end(); it2++ ) {
//		log() << " - " << (*it2)->className() << endl;
//	}

	return is;
}

//
// Function:	TBitcoinScript :: write
// Description:
//
ostream &TBitcoinScript::write( ostream &os ) const
{
	list<TStackOperator*>::const_iterator it;
	TStackOperatorFromStream *S;

	it = Program.begin();
	while( it != Program.end() ) {
		// We can't have non-streamable operators in the program, that's
		// an error
		S = dynamic_cast<TStackOperatorFromStream*>( (*it) );
		if( S == NULL ) {
			string err("TBitcoinScript: Can't write() non-streamable operator ");
			throw runtime_error( err + (*it)->className() );
		}

		S->write( os );

		it++;
	}
	return os;
}

// -----------

//
// Function:	TBitcoinScript_1 :: TBitcoinScript_1
// Description:
//
TBitcoinScript_1::TBitcoinScript_1()
{
}

//
// Function:	TBitcoinScript_1 :: TBitcoinScript_1
// Description:
//
TBitcoinScript_1::TBitcoinScript_1( const TStackOperator **a, unsigned int n ) :
	TBitcoinScript(a,n)
{
}

//
// Function:	TBitcoinScript_1 :: TBitcoinScript_1
// Description:
//
TBitcoinScript_1::TBitcoinScript_1( const string &s, eReadMode a ) :
	TBitcoinScript(s, a)
{
}

//
// Function:	TBitcoinScript_1 :: getMinimumAcceptedVersion
// Description:
//
uint32_t TBitcoinScript_1::getMinimumAcceptedVersion() const
{
	return 0;
}

//
// Function:	TBitcoinScript_1 :: init
// Description:
//
void TBitcoinScript_1::init()
{
	// Claimant scripts are limited
	ClaimantTemplates.push_back( new TStackOperator_OP_FALSE );
	ClaimantTemplates.push_back( new TStackOperator_OP_PUSHDATA1 );
	ClaimantTemplates.push_back( new TStackOperator_OP_PUSHDATA2 );
	ClaimantTemplates.push_back( new TStackOperator_OP_PUSHDATA4 );
	ClaimantTemplates.push_back( new TStackOperator_OP_1NEGATE );
	ClaimantTemplates.push_back( new TStackOperator_OP_TRUE );
	ClaimantTemplates.push_back( new TStackOperator_OP_NOP );
	ClaimantTemplates.push_back( new TStackOperator_OP_NOP_N );
	ClaimantTemplates.push_back( new TStackOperator_OP_N );
	ClaimantTemplates.push_back( new TStackOperator_PUSH_N );
	// Authorisation scripts can do a lot more
	AuthorisationTemplates.push_back( new TStackOperator_OP_FALSE );
	AuthorisationTemplates.push_back( new TStackOperator_OP_PUSHDATA1 );
	AuthorisationTemplates.push_back( new TStackOperator_OP_PUSHDATA2 );
	AuthorisationTemplates.push_back( new TStackOperator_OP_PUSHDATA4 );
	AuthorisationTemplates.push_back( new TStackOperator_OP_1NEGATE );
	AuthorisationTemplates.push_back( new TStackOperator_OP_TRUE );
	AuthorisationTemplates.push_back( new TStackOperator_OP_NOP );
	AuthorisationTemplates.push_back( new TStackOperator_OP_IF );
	AuthorisationTemplates.push_back( new TStackOperator_OP_NOTIF );
	AuthorisationTemplates.push_back( new TStackOperator_OP_ELSE );
	AuthorisationTemplates.push_back( new TStackOperator_OP_ENDIF );
	AuthorisationTemplates.push_back( new TStackOperator_OP_VERIFY );
	AuthorisationTemplates.push_back( new TStackOperator_OP_RETURN );
	AuthorisationTemplates.push_back( new TStackOperator_OP_TOALTSTACK );
	AuthorisationTemplates.push_back( new TStackOperator_OP_FROMALTSTACK );
	AuthorisationTemplates.push_back( new TStackOperator_OP_IFDUP );
	AuthorisationTemplates.push_back( new TStackOperator_OP_DEPTH );
	AuthorisationTemplates.push_back( new TStackOperator_OP_DROP );
	AuthorisationTemplates.push_back( new TStackOperator_OP_DUP );
	AuthorisationTemplates.push_back( new TStackOperator_OP_NIP );
	AuthorisationTemplates.push_back( new TStackOperator_OP_OVER );
	AuthorisationTemplates.push_back( new TStackOperator_OP_PICK );
	AuthorisationTemplates.push_back( new TStackOperator_OP_ROLL );
	AuthorisationTemplates.push_back( new TStackOperator_OP_ROT );
	AuthorisationTemplates.push_back( new TStackOperator_OP_SWAP );
	AuthorisationTemplates.push_back( new TStackOperator_OP_TUCK );
	AuthorisationTemplates.push_back( new TStackOperator_OP_2DROP );
	AuthorisationTemplates.push_back( new TStackOperator_OP_2DUP );
	AuthorisationTemplates.push_back( new TStackOperator_OP_3DUP );
	AuthorisationTemplates.push_back( new TStackOperator_OP_2OVER );
	AuthorisationTemplates.push_back( new TStackOperator_OP_2ROT );
	AuthorisationTemplates.push_back( new TStackOperator_OP_2SWAP );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_CAT );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_SUBSTR );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_LEFT );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_RIGHT );
	AuthorisationTemplates.push_back( new TStackOperator_OP_SIZE );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_INVERT );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_AND );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_OR );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_XOR );
	AuthorisationTemplates.push_back( new TStackOperator_OP_EQUAL );
	AuthorisationTemplates.push_back( new TStackOperator_OP_EQUALVERIFY );
	AuthorisationTemplates.push_back( new TStackOperator_OP_1ADD );
	AuthorisationTemplates.push_back( new TStackOperator_OP_1SUB );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_2MUL );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_2DIV );
	AuthorisationTemplates.push_back( new TStackOperator_OP_NEGATE );
	AuthorisationTemplates.push_back( new TStackOperator_OP_ABS );
	AuthorisationTemplates.push_back( new TStackOperator_OP_NOT );
	AuthorisationTemplates.push_back( new TStackOperator_OP_0NOTEQUAL );
	AuthorisationTemplates.push_back( new TStackOperator_OP_ADD );
	AuthorisationTemplates.push_back( new TStackOperator_OP_SUB );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_MUL );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_DIV );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_MOD );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_LSHIFT );
//	AuthorisationTemplates.push_back( new TStackOperator_OP_RSHIFT );
	AuthorisationTemplates.push_back( new TStackOperator_OP_BOOLAND );
	AuthorisationTemplates.push_back( new TStackOperator_OP_BOOLOR );
	AuthorisationTemplates.push_back( new TStackOperator_OP_NUMEQUAL );
	AuthorisationTemplates.push_back( new TStackOperator_OP_NUMEQUALVERIFY );
	AuthorisationTemplates.push_back( new TStackOperator_OP_NUMNOTEQUAL );
	AuthorisationTemplates.push_back( new TStackOperator_OP_LESSTHAN );
	AuthorisationTemplates.push_back( new TStackOperator_OP_GREATERTHAN );
	AuthorisationTemplates.push_back( new TStackOperator_OP_LESSTHANOREQUAL );
	AuthorisationTemplates.push_back( new TStackOperator_OP_GREATERTHANOREQUAL );
	AuthorisationTemplates.push_back( new TStackOperator_OP_MIN );
	AuthorisationTemplates.push_back( new TStackOperator_OP_MAX );
	AuthorisationTemplates.push_back( new TStackOperator_OP_WITHIN );
	AuthorisationTemplates.push_back( new TStackOperator_OP_RIPEMD160 );
	AuthorisationTemplates.push_back( new TStackOperator_OP_SHA1 );
	AuthorisationTemplates.push_back( new TStackOperator_OP_SHA256 );
	AuthorisationTemplates.push_back( new TStackOperator_OP_HASH160 );
	AuthorisationTemplates.push_back( new TStackOperator_OP_HASH256 );
	AuthorisationTemplates.push_back( new TStackOperator_OP_CODESEPARATOR );
	AuthorisationTemplates.push_back( new TStackOperator_OP_CHECKSIG );
	AuthorisationTemplates.push_back( new TStackOperator_OP_CHECKSIGVERIFY );
	AuthorisationTemplates.push_back( new TStackOperator_OP_CHECKMULTISIG );
	AuthorisationTemplates.push_back( new TStackOperator_OP_CHECKMULTISIGVERIFY );
	AuthorisationTemplates.push_back( new TStackOperator_OP_PUBKEYHASH );
	AuthorisationTemplates.push_back( new TStackOperator_OP_PUBKEY );
	AuthorisationTemplates.push_back( new TStackOperator_OP_RESERVED );
	AuthorisationTemplates.push_back( new TStackOperator_OP_VER );
	AuthorisationTemplates.push_back( new TStackOperator_OP_VERIF );
	AuthorisationTemplates.push_back( new TStackOperator_OP_VERNOTIF );
	AuthorisationTemplates.push_back( new TStackOperator_OP_RESERVED1 );
	AuthorisationTemplates.push_back( new TStackOperator_OP_RESERVED2 );
	AuthorisationTemplates.push_back( new TStackOperator_OP_NOP_N );
	AuthorisationTemplates.push_back( new TStackOperator_OP_N );
	AuthorisationTemplates.push_back( new TStackOperator_PUSH_N );
	// OP_INVALIDOPCODE _must_ be last in the template list if it is
	// wanted, this is because it is a special case and has an
	// acceptOpcode() that always returns true.  Therefore, it will take
	// up any opcode not previously accepted by one of the above.  It
	// can be used in situations where you want to allow but ignore
	// invalid opcodes.
//	AuthorisationTemplates.push_back( new TStackOperator_OP_INVALIDOPCODE );

	TBitcoinScriptBase::init();
}

// -----------

//
// Function:	TStackOperator :: printOn
// Description:
//
ostream &TStackOperator::printOn( ostream &os ) const
{
	os << className();
	return os;
}

//
// Function:	TStackOperator :: createPUSH
// Description:
//
TStackOperator *TStackOperator::createPUSH( const string &Raw )
{
	if( Raw.size() < PUSH_1 ) {
		throw runtime_error( "Can't create a script PUSH for an empty string" );
	} else if( Raw.size() <= PUSH_75 ) {
		return new TStackOperator_PUSH_N( Raw );
	} else if( Raw.size() < (1 << 8) ) {
		return new TStackOperator_OP_PUSHDATA1( Raw );
	} else if( Raw.size() < (1 << 16) ) {
		return new TStackOperator_OP_PUSHDATA2( Raw );
	} else {
		return new TStackOperator_OP_PUSHDATA4( Raw );
	}
	return NULL;
}

// -----------

//
// Function:	TStackOperatorFromStream :: readAndAppend
// Description:
//
// We are the template, called because our acceptOpcode() returned true
//
// By default we simply clone ourself and get the clone to read itself.
//
istream &TStackOperatorFromStream::readAndAppend( TBitcoinScriptBase *Script, istream &is ) const
{
	// While we have matched the type, we haven't actually created an
	// operator that we can read to yet.
	TStackOperatorFromStream *Operator = clone();
	Operator->read(is);
	// Now we can push what's been created to the script
	Script->append( Operator );
	return is;
}

// -----------

//
// Function:  OP_FALSE
// Input:     Nothing
// Output:    False
// Operation:
//
TBitcoinScript::tInstructionPointer
TStackOperator_OP_FALSE::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.give( new TStackElementBoolean(false) );

	return ip;
}

//
// Function:  OP_PUSHDATAN
// Input:     
// Output:    
// Operation:
//
TBitcoinScript::tInstructionPointer
TStackOperator_OP_PUSHDATAN::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.give( new TStackElementString(Raw) );

	return ip;
}

//
// Function:  OP_1NEGATE
// Input:     
// Output:    
// Operation:
//
TBitcoinScript::tInstructionPointer
TStackOperator_OP_1NEGATE::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.give( new TStackElementBigInteger(-1) );

	return ip;
}

//
// Function:  OP_TRUE
// Input:     
// Output:    
// Operation:
//
TBitcoinScript::tInstructionPointer
TStackOperator_OP_TRUE::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.give( new TStackElementBoolean(true) );

	return ip;
}

//
// Function:  OP_IF
// <expression> if [statements] [else [statements]] endif
//
// Operation: If the top stack value is 1, the statements are executed.
// The top stack value is removed.
//
TBitcoinScript::tInstructionPointer
TStackOperator_OP_IF::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  OP_NOTIF
// <expression> if [statements] [else [statements]] endif
//
// If the top stack value is 0, the statements are executed. The top
// stack value is removed.
//
TBitcoinScript::tInstructionPointer
TStackOperator_OP_NOTIF::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  OP_ELSE
// <expression> if [statements] [else [statements]] endif
//
// If the preceding OP_IF or OP_NOTIF was not executed then these
// statements are.
//
TBitcoinScript::tInstructionPointer
TStackOperator_OP_ELSE::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  OP_ENDIF
// <expression> if [statements] [else [statements]] endif
//
// Ends an if/else block.
//
TBitcoinScript::tInstructionPointer
TStackOperator_OP_ENDIF::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  OP_VERIFY
// Input:     True / false
// Output:    Nothing / False
// Operation: Marks transaction as invalid if top stack value is not
// true. True is removed, but false is not.
//
TBitcoinScript::tInstructionPointer
TStackOperator_OP_VERIFY::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	TStackElement *Back = Stack().back();

	TStackElementInteger *I = dynamic_cast<TStackElementInteger*>(Back);
	if( I != NULL ) {
		Stack.Invalid = (I->Data == 0);
	}

	TStackElementBigInteger *BI = dynamic_cast<TStackElementBigInteger*>(Back);
	if( BI != NULL ) {
		Stack.Invalid = BI->Data.isZero();
	}

	TStackElementBoolean *B = dynamic_cast<TStackElementBoolean*>(Back);
	if( B != NULL ) {
		Stack.Invalid = (B->Data == false);
	}

	TStackElementString *S = dynamic_cast<TStackElementString*>(Back);
	if( S != NULL ) {
		Stack.Invalid = S->Data.empty();
	}

	if( !Stack.Invalid ) {
		// True is removed
		Stack.take();
		delete Back;
	} else {
		// The specs say carry on, but as the subsequent commands are
		// going to have a value on the stack that they didn't expect,
		// what is the point?
		throw script_run_verify_error();
	}

	return ip;
}

//
// Function:  OP_RETURN
// Input:     Nothing
// Output:    Nothing
// Operation: Marks transaction as invalid.
//
TBitcoinScript::tInstructionPointer
TStackOperator_OP_RETURN::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.Invalid = true;
	throw script_run_verify_error();
}

// Function:  OP_TOALTSTACK
// Input:     x1
// Output:    (alt)x1
// Operation: Puts the input onto the top of the alt stack. Removes it
// from the main stack.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_TOALTSTACK::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	// Copy the last thing on the real stack to the alternate stack
	Stack.AltStack.push_back( Stack.Stack.back() );
	// Remove the last element on the real stack
	Stack.Stack.pop_back();

	return ip;
}

//
// Function:  OP_FROMALTSTACK
// Input:     (alt)x1
// Output:    x1
// Operation: Puts the input onto the top of the main stack. Removes it
// from the alt stack.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_FROMALTSTACK::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	// Copy the last thing on the alternate stack to the real stack
	Stack.give( Stack.AltStack.back() );
	// Remove the last element on the alternate stack
	Stack.AltStack.pop_back();

	return ip;
}

//
// Function:  OP_IFDUP
// Input:     x
// Output:    x / x x
// Operation: If the input is true or false, duplicate it.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_IFDUP::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  OP_DEPTH
// Input:     Nothing
// Output:    <Stack size>
// Operation: Puts the number of stack items onto the stack.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_DEPTH::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.give( new TStackElementBigInteger( Stack.Stack.size() ) );

	return ip;
}

//
// Function:  OP_DROP
// Input:     x
// Output:    Nothing
// Operation: Removes the top stack item.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_DROP::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	delete Stack.take();

	return ip;
}

//
// Function:  OP_DUP
// Input:     x
// Output:    x x
// Operation: Duplicates the top stack item.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_DUP::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.give( Stack().back()->clone() );

	return ip;
}

//
// Function:  OP_NIP
// Input:     x1 x2
// Output:    x2
// Operation: Removes the second-to-top stack item.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_NIP::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	TExecutionContext::iterator it = Stack.Stack.end();

	// Point at last element
	it--;
	// Point at second to last element
	it--;

	Stack.Stack.erase( it );

	return ip;
}

//
// Function:  OP_OVER
// Input:     x1 x2
// Output:    x1 x2 x1
// Operation: Copies the second-to-top stack item to the top.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_OVER::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	TExecutionContext::iterator it = Stack.Stack.end();

	// Point at last element
	it--;
	// Point at second to last element
	it--;

	// Copy it
	Stack.give( (*it)->clone() );

	return ip;
}

//
// Function:  OP_PICK
// Input:     xn ... x2 x1 x0 <n>
// Output:    xn ... x2 x1 x0 xn
// Operation: The item n back in the stack is copied to the top.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_PICK::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> n( Stack.take() );
	TStackElementInteger *N = dynamic_cast<TStackElementInteger*>(n.get());

	if( N == NULL )
		throw script_run_parameter_type_error();

	TExecutionContext::iterator it = Stack.Stack.end();
	// Point at the last element
	it--;
	// Point at the Nth to last element
	advance( it, -N->Data );

	// Copy it
	Stack.give( (*it)->clone() );

	return ip;
}

//
// Function:  OP_ROLL
// Input:     xn ... x2 x1 x0 <n>
// Output:    ... x2 x1 x0 xn
// Operation: The item n back in the stack is moved to the top.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_ROLL::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> n( Stack.take() );
	TStackElementInteger *N = dynamic_cast<TStackElementInteger*>(n.get());

	if( N == NULL )
		throw script_run_parameter_type_error();

	TExecutionContext::iterator it = Stack.Stack.end();
	// Point at the last element
	it--;
	// Point at the Nth to last element
	advance( it, -N->Data );

	TStackElement *Element = (*it);

	// Move it
	Stack.Stack.erase( it );
	Stack.give( Element );

	return ip;
}

//
// Function:  OP_ROT
// Input:     x1 x2 x3
// Output:    x2 x3 x1
// Operation: The top three items on the stack are rotated to the left.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_ROT::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	TStackElement *x3 = Stack.take();
	TStackElement *x2 = Stack.take();
	TStackElement *x1 = Stack.take();

	Stack.give(x2);
	Stack.give(x3);
	Stack.give(x1);

	return ip;
}

//
// Function:  OP_SWAP
// Input:     x1 x2
// Output:    x2 x1
// Operation: The top two items on the stack are swapped.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_SWAP::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	TStackElement *x2 = Stack.take();
	TStackElement *x1 = Stack.take();

	Stack.give(x2);
	Stack.give(x1);

	return ip;
}

//
// Function:  OP_TUCK
// Input:     x1 x2
// Output:    x2 x1 x2
// Operation: The item at the top of the stack is copied and inserted
// before the second-to-top item.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_TUCK::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	// XXX: Total cheat, this should be done with an iterator, but I
	// suspect it wouldn't be that much quicker, as we're only
	// manipulating pointers
	TStackElement *x2 = Stack.take();
	TStackElement *x1 = Stack.take();

	Stack.give(x2);
	Stack.give(x1);
	Stack.give(x2->clone());

	return ip;
}

//
// Function:  OP_2DROP
// Input:     x1 x2
// Output:    Nothing
// Operation: Removes the top two stack items.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_2DROP::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	delete Stack.take();
	delete Stack.take();

	return ip;
}

//
// Function:  OP_2DUP
// Input:     x1 x2
// Output:    x1 x2 x1 x2
// Operation: Duplicates the top two stack items.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_2DUP::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	TExecutionContext::iterator it = Stack.Stack.end();

	// Point at second to last element
	advance( it, -2 );

	// Copy them
	Stack.give( (*it)->clone() );
	it++;
	Stack.give( (*it)->clone() );

	return ip;
}

//
// Function:  OP_3DUP
// Input:     x1 x2 x3
// Output:    x1 x2 x3 x1 x2 x3
// Operation: Duplicates the top three stack items.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_3DUP::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	TExecutionContext::iterator it = Stack.Stack.end();

	// Point at second to last element
	advance( it, -3 );

	// Copy them
	Stack.give( (*it)->clone() );
	it++;
	Stack.give( (*it)->clone() );
	it++;
	Stack.give( (*it)->clone() );

	return ip;
}

//
// Function:  OP_2OVER
// Input:     x1 x2 x3 x4
// Output:    x1 x2 x3 x4 x1 x2
// Operation: Copies the pair of items two spaces back in the stack to
// the front.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_2OVER::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	TExecutionContext::iterator it = Stack.Stack.end();

	// Point at fourth from last element
	advance( it, -4 );

	// Copy them
	Stack.give( (*it)->clone() );
	it++;
	Stack.give( (*it)->clone() );

	return ip;
}

//
// Function:  OP_2ROT
// Input:     x1 x2 x3 x4 x5 x6
// Output:    x3 x4 x5 x6 x1 x2
// Operation: The fifth and sixth items back are moved to the top of the
// stack.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_2ROT::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	TExecutionContext::iterator it = Stack.Stack.end();

	// Point at sixth from last element
	advance( it, -6 );

	// Move them
	Stack.give( (*it)->clone() );
	it = Stack.Stack.erase(it);
	Stack.give( (*it)->clone() );
	Stack.Stack.erase(it);

	return ip;
}

//
// Function:  OP_2SWAP
// Input:     x1 x2 x3 x4
// Output:    x3 x4 x1 x2
// Operation: Swaps the top two pairs of items.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_2SWAP::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	TExecutionContext::iterator it = Stack.Stack.end();

	// Point at fourth from last element
	advance( it, -4 );

	// Move them
	Stack.give( (*it)->clone() );
	it = Stack.Stack.erase(it);
	Stack.give( (*it)->clone() );
	Stack.Stack.erase(it);

	return ip;
}

//
// Function:  OP_CAT
// Input:     x1 x2
// Output:    out
// Operation: Concatenates two strings. Currently disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_CAT::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> x2( Stack.take() );
	auto_ptr<TStackElement> x1( Stack.take() );
	TStackElementString *S2 = dynamic_cast<TStackElementString*>(x2.get());
	TStackElementString *S1 = dynamic_cast<TStackElementString*>(x1.get());

	if( S1 == NULL || S2 == NULL )
		throw script_run_parameter_type_error();

	// XXX: S1+S2 or S2+S1?
	Stack.give( new TStackElementString( S2->Data + S1->Data ) );

	return ip;
}

//
// Function:  OP_SUBSTR
// Input:     in begin size
// Output:    out
// Operation: Returns a section of a string. Currently disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_SUBSTR::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> size( Stack.take() );
	auto_ptr<TStackElement> begin( Stack.take() );
	auto_ptr<TStackElement> in( Stack.take() );
	TStackElementInteger *SIZE = dynamic_cast<TStackElementInteger*>(size.get());
	TStackElementInteger *BEGIN = dynamic_cast<TStackElementInteger*>(begin.get());
	TStackElementString *IN = dynamic_cast<TStackElementString*>(in.get());

	if( IN == NULL || BEGIN == NULL || SIZE == NULL )
		throw script_run_parameter_type_error();

	if( BEGIN->Data + SIZE->Data > IN->Data.size() )
		throw script_run_parameter_invalid();

	Stack.give( new TStackElementString( IN->Data.substr(BEGIN->Data, SIZE->Data) ) );

	return ip;
}

//
// Function:  OP_LEFT
// Input:     in size
// Output:    out
// Operation: Keeps only characters left of the specified point in a
// string. Currently disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_LEFT::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> size( Stack.take() );
	auto_ptr<TStackElement> in( Stack.take() );
	TStackElementInteger *SIZE = dynamic_cast<TStackElementInteger*>(size.get());
	TStackElementString *IN = dynamic_cast<TStackElementString*>(in.get());

	if( IN == NULL || SIZE == NULL )
		throw script_run_parameter_type_error();

	if( SIZE->Data > IN->Data.size() )
		throw script_run_parameter_invalid();

	Stack.give( new TStackElementString( IN->Data.substr(0, SIZE->Data) ) );

	return ip;
}

//
// Function:  OP_RIGHT
// Input:     in size
// Output:    out
// Operation: Keeps only characters right of the specified point in a
// string. Currently disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_RIGHT::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> size( Stack.take() );
	auto_ptr<TStackElement> in( Stack.take() );
	TStackElementInteger *SIZE = dynamic_cast<TStackElementInteger*>(size.get());
	TStackElementString *IN = dynamic_cast<TStackElementString*>(in.get());

	if( IN == NULL || SIZE == NULL )
		throw script_run_parameter_type_error();

	if( SIZE->Data > IN->Data.size() )
		throw script_run_parameter_invalid();

	Stack.give( new TStackElementString( IN->Data.substr(IN->Data.size() - SIZE->Data, SIZE->Data) ) );

	return ip;
}

//
// Function:  OP_SIZE
// Input:     in
// Output:    in size
// Operation: Returns the length of the input string.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_SIZE::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	TStackElement *in( Stack.Stack.back() );
	TStackElementString *IN = dynamic_cast<TStackElementString*>(in);

	if( IN == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( IN->Data.size() ) );

	return ip;
}

//
// Function:  OP_INVERT
// Input:     in
// Output:    out
// Operation: Flips all of the bits in the input. Currently disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_INVERT::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( ~A->Data ) );

	return ip;
}

//
// Function:  OP_AND
// Input:     x1 x2
// Output:    out
// Operation: Boolean and between each bit in the inputs. Currently
// disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_AND::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( A->Data & B->Data ) );

	return ip;
}

//
// Function:  OP_OR
// Input:     x1 x2
// Output:    out
// Operation: Boolean or between each bit in the inputs. Currently
// disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_OR::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( A->Data | B->Data ) );

	return ip;
}

//
// Function:  OP_XOR
// Input:     x1 x2
// Output:    out
// Operation: Boolean exclusive or between each bit in the inputs.
// Currently disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_XOR::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( A->Data ^ B->Data ) );

	return ip;
}

//
// Function:  OP_EQUAL
// Input:     x1 x2
// Output:    True / false
// Operation: Returns 1 if the inputs are exactly equal, 0 otherwise.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_EQUAL::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> x1( Stack.take() );
	auto_ptr<TStackElement> x2( Stack.take() );

	if( dynamic_cast<TStackElementString*>(x1.get()) != NULL
			&& dynamic_cast<TStackElementString*>(x2.get()) != NULL ) {
		TStackElementString *S1 = dynamic_cast<TStackElementString*>(x1.get());
		TStackElementString *S2 = dynamic_cast<TStackElementString*>(x2.get());
		Stack.give( new TStackElementBoolean(S1->Data == S2->Data) );
	} else if( dynamic_cast<TStackElementInteger*>(x1.get()) != NULL
			&& dynamic_cast<TStackElementInteger*>(x2.get()) != NULL ) {
		TStackElementInteger *S1 = dynamic_cast<TStackElementInteger*>(x1.get());
		TStackElementInteger *S2 = dynamic_cast<TStackElementInteger*>(x2.get());
		Stack.give( new TStackElementBoolean(S1->Data == S2->Data) );
	} else if( dynamic_cast<TStackElementBigInteger*>(x1.get()) != NULL
			&& dynamic_cast<TStackElementBigInteger*>(x2.get()) != NULL ) {
		TStackElementBigInteger *S1 = dynamic_cast<TStackElementBigInteger*>(x1.get());
		TStackElementBigInteger *S2 = dynamic_cast<TStackElementBigInteger*>(x2.get());
		Stack.give( new TStackElementBoolean(S1->Data == S2->Data) );
	} else if( dynamic_cast<TStackElementBoolean*>(x1.get()) != NULL
			&& dynamic_cast<TStackElementBoolean*>(x2.get()) != NULL ) {
		TStackElementBoolean *S1 = dynamic_cast<TStackElementBoolean*>(x1.get());
		TStackElementBoolean *S2 = dynamic_cast<TStackElementBoolean*>(x2.get());
		Stack.give( new TStackElementBoolean(S1->Data == S2->Data) );
	} else {
		throw script_run_error( "Invalid type mismatch in OP_EQUAL" );
	}

	return ip;
}

//
// Function:  OP_EQUALVERIFY
// Input:     x1 x2
// Output:    True / false
// Operation: Same as OP_EQUAL, but runs OP_VERIFY afterward.
void TStackOperator_OP_EQUALVERIFY::explode( TBitcoinScriptBase *Script ) const
{
	Script->append( new TStackOperator_OP_EQUAL );
	Script->append( new TStackOperator_OP_VERIFY );
}

//
// Function:  OP_1ADD
// Input:     in
// Output:    out
// Operation: 1 is added to the input.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_1ADD::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> in( Stack.take() );
	TStackElementBigInteger *IN = dynamic_cast<TStackElementBigInteger*>(in.get());

	if( IN == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( IN->Data + 1 ) );

	return ip;
}

//
// Function:  OP_1SUB
// Input:     in
// Output:    out
// Operation: 1 is subtracted from the input.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_1SUB::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> in( Stack.take() );
	TStackElementBigInteger *IN = dynamic_cast<TStackElementBigInteger*>(in.get());

	if( IN == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( IN->Data - 1 ) );

	return ip;
}

//
// Function:  OP_2MUL
// Input:     in
// Output:    out
// Operation: The input is multiplied by 2. Currently disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_2MUL::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> in( Stack.take() );
	TStackElementBigInteger *IN = dynamic_cast<TStackElementBigInteger*>(in.get());

	if( IN == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( IN->Data * 2 ) );

	return ip;
}

//
// Function:  OP_2DIV
// Input:     in
// Output:    out
// Operation: The input is divided by 2. Currently disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_2DIV::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> in( Stack.take() );
	TStackElementBigInteger *IN = dynamic_cast<TStackElementBigInteger*>(in.get());

	if( IN == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( IN->Data / 2 ) );

	return ip;
}

//
// Function:  OP_NEGATE
// Input:     in
// Output:    out
// Operation: The sign of the input is flipped.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_NEGATE::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> in( Stack.take() );
	TStackElementBigInteger *IN = dynamic_cast<TStackElementBigInteger*>(in.get());

	if( IN == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( -IN->Data ) );

	return ip;
}

//
// Function:  OP_ABS
// Input:     in
// Output:    out
// Operation: The input is made positive.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_ABS::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> in( Stack.take() );
	TStackElementBigInteger *IN = dynamic_cast<TStackElementBigInteger*>(in.get());

	if( IN == NULL )
		throw script_run_parameter_type_error();

	if( IN->Data < 0 ) {
		Stack.give( new TStackElementBigInteger( -IN->Data ) );
	} else {
		// XXX: We could have saved ourselves the trouble by checking
		// for this condition earlier
		Stack.give( new TStackElementBigInteger( IN->Data ) );
	}

	return ip;
}

//
// Function:  OP_NOT
// Input:     in
// Output:    out
// Operation: If the input is 0 or 1, it is flipped. Otherwise the
// output will be 0.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_NOT::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> in( Stack.take() );
	TStackElementBoolean *IN = dynamic_cast<TStackElementBoolean*>(in.get());

	if( IN != NULL && IN->Data == false ) {
		Stack.give( new TStackElementBoolean( true ) );
	} else {
		Stack.give( new TStackElementBoolean( false ) );
	}

	return ip;
}

//
// Function:  OP_0NOTEQUAL
// Input:     in
// Output:    out
// Operation: Returns 1 if the input is 0. 0 otherwise.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_0NOTEQUAL::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> in( Stack.take() );
	TStackElementInteger *IN = dynamic_cast<TStackElementInteger*>(in.get());

	// This is identical to OP_NOT isn't it?

	if( IN != NULL && IN->Data == 0 ) {
		Stack.give( new TStackElementBoolean( true ) );
	} else {
		Stack.give( new TStackElementBoolean( false ) );
	}

	return ip;
}

//
// Function:  OP_ADD
// Input:     a b
// Output:    out
// Operation: a is added to b.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_ADD::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( A->Data + B->Data ) );

	return ip;
}

//
// Function:  OP_SUB
// Input:     a b
// Output:    out
// Operation: b is subtracted from a.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_SUB::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( A->Data - B->Data ) );

	return ip;
}

//
// Function:  OP_MUL
// Input:     a b
// Output:    out
// Operation: a is multiplied by b. Currently disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_MUL::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( A->Data * B->Data ) );

	return ip;
}

//
// Function:  OP_DIV
// Input:     a b
// Output:    out
// Operation: a is divided by b. Currently disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_DIV::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL || B->Data == 0 )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( A->Data / B->Data ) );

	return ip;
}

//
// Function:  OP_MOD
// Input:     a b
// Output:    out
// Operation: Returns the remainder after dividing a by b. Currently
// disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_MOD::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL || B->Data == 0 )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( A->Data % B->Data ) );

	return ip;
}

//
// Function:  OP_LSHIFT
// Input:     a b
// Output:    out
// Operation: Shifts a left b bits, preserving sign. Currently disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_LSHIFT::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( A->Data << B->Data.getBlock(0) ) );

	return ip;
}

//
// Function:  OP_RSHIFT
// Input:     a b
// Output:    out
// Operation: Shifts a right b bits, preserving sign. Currently disabled.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_RSHIFT::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBigInteger( A->Data >> B->Data.getBlock(0) ) );

	return ip;
}

//
// Function:  OP_BOOLAND
// Input:     a b
// Output:    out
// Operation: If both a and b are not 0, the output is 1. Otherwise 0.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_BOOLAND::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBoolean *B = dynamic_cast<TStackElementBoolean*>(b.get());
	TStackElementBoolean *A = dynamic_cast<TStackElementBoolean*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBoolean( A->Data && B->Data ) );

	return ip;
}

//
// Function:  OP_BOOLOR
// Input:     a b
// Output:    out
// Operation: If a or b is not 0, the output is 1. Otherwise 0.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_BOOLOR::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBoolean *B = dynamic_cast<TStackElementBoolean*>(b.get());
	TStackElementBoolean *A = dynamic_cast<TStackElementBoolean*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBoolean( A->Data || B->Data ) );

	return ip;
}

//
// Function:  OP_NUMEQUAL
// Input:     a b
// Output:    out
// Operation: Returns 1 if the numbers are equal, 0 otherwise.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_NUMEQUAL::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementInteger *B = dynamic_cast<TStackElementInteger*>(b.get());
	TStackElementInteger *A = dynamic_cast<TStackElementInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBoolean( A->Data == B->Data ) );

	return ip;
}

//
// Function:  OP_NUMEQUALVERIFY
// Input:     a b
// Output:    out
// Operation: Same as OP_NUMEQUAL, but runs OP_VERIFY afterward.
void TStackOperator_OP_NUMEQUALVERIFY::explode( TBitcoinScriptBase *Script ) const
{
	Script->append( new TStackOperator_OP_NUMEQUAL );
	Script->append( new TStackOperator_OP_VERIFY );
}

//
// Function:  OP_NUMNOTEQUAL
// Input:     a b
// Output:    out
// Operation: Returns 1 if the numbers are not equal, 0 otherwise.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_NUMNOTEQUAL::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBoolean( A->Data != B->Data ) );

	return ip;
}

//
// Function:  OP_LESSTHAN
// Input:     a b
// Output:    out
// Operation: Returns 1 if a is less than b, 0 otherwise.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_LESSTHAN::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBoolean( A->Data < B->Data ) );

	return ip;
}

//
// Function:  OP_GREATERTHAN
// Input:     a b
// Output:    out
// Operation: Returns 1 if a is greater than b, 0 otherwise.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_GREATERTHAN::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBoolean( A->Data > B->Data ) );

	return ip;
}

//
// Function:  OP_LESSTHANOREQUAL
// Input:     a b
// Output:    out
// Operation: Returns 1 if a is less than or equal to b, 0 otherwise.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_LESSTHANOREQUAL::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBoolean( A->Data <= B->Data ) );

	return ip;
}

//
// Function:  OP_GREATERTHANOREQUAL
// Input:     a b
// Output:    out
// Operation: Returns 1 if a is greater than or equal to b, 0 otherwise.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_GREATERTHANOREQUAL::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBoolean( A->Data >= B->Data ) );

	return ip;
}

//
// Function:  OP_MIN
// Input:     a b
// Output:    out
// Operation: Returns the smaller of a and b.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_MIN::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	// XXX: We can probably do this better by selectively deleting
	// rather than copying
	if( A->Data < B->Data ) {
		Stack.give( new TStackElementBigInteger( A->Data ) );
	} else {
		Stack.give( new TStackElementBigInteger( B->Data ) );
	}

	return ip;
}

//
// Function:  OP_MAX
// Input:     a b
// Output:    out
// Operation: Returns the larger of a and b.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_MAX::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> b( Stack.take() );
	auto_ptr<TStackElement> a( Stack.take() );
	TStackElementBigInteger *B = dynamic_cast<TStackElementBigInteger*>(b.get());
	TStackElementBigInteger *A = dynamic_cast<TStackElementBigInteger*>(a.get());

	if( A == NULL || B == NULL )
		throw script_run_parameter_type_error();

	// XXX: We can probably do this better by selectively deleting
	// rather than copying
	if( A->Data > B->Data ) {
		Stack.give( new TStackElementBigInteger( A->Data ) );
	} else {
		Stack.give( new TStackElementBigInteger( B->Data ) );
	}

	return ip;
}

//
// Function:  OP_WITHIN
// Input:     x min max
// Output:    out
// Operation: Returns 1 if x is within the specified range
// (left-inclusive), 0 otherwise.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_WITHIN::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> x( Stack.take() );
	auto_ptr<TStackElement> min( Stack.take() );
	auto_ptr<TStackElement> max( Stack.take() );
	TStackElementBigInteger *X = dynamic_cast<TStackElementBigInteger*>(x.get());
	TStackElementBigInteger *MIN = dynamic_cast<TStackElementBigInteger*>(min.get());
	TStackElementBigInteger *MAX = dynamic_cast<TStackElementBigInteger*>(max.get());

	if( X == NULL || MIN == NULL || MAX == NULL )
		throw script_run_parameter_type_error();

	Stack.give( new TStackElementBoolean( X->Data >= MIN->Data && X->Data < MAX->Data ) );

	return ip;
}

//
// Function:	TStackOperator_CryptographicDigest
// Description:
// The hashing operators are all identical except in the hash function
// they use, rather than implement the same code multiple times, this
// class implements the common code and simply asks the child class what
// hash it wants us to use.
//
TBitcoinScript::tInstructionPointer
TStackOperator_CryptographicDigest::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	auto_ptr<TStackElement> in( Stack.take() );
	TStackElementString *IN = dynamic_cast<TStackElementString*>(in.get());

	if( IN == NULL )
		throw script_run_parameter_type_error();

	// Push the hashed version back onto the stack
	TMessageDigest *Hasher = createHasher();
	Stack.give( new TStackElementString(Hasher->transform( IN->Data ) ) );
	delete Hasher;

	return ip;
}

//
// Function:  OP_RIPEMD160
// Input:     in
// Output:    hash
// Operation: The input is hashed using RIPEMD-160.
TMessageDigest *TStackOperator_OP_RIPEMD160::createHasher() const
{
	return new THash_ripemd160;
}

//
// Function:  OP_SHA1
// Input:     in
// Output:    hash
// Operation: The input is hashed using SHA-1.
TMessageDigest *TStackOperator_OP_SHA1::createHasher() const
{
	return new THash_sha1;
}

//
// Function:  OP_SHA256
// Input:     in
// Output:    hash
// Operation: The input is hashed using SHA-256.
TMessageDigest *TStackOperator_OP_SHA256::createHasher() const
{
	return new THash_sha256;
}

//
// Function:  OP_HASH160
// Input:     in
// Output:    hash
// Operation: The input is hashed twice: first with SHA-256 and then
// with RIPEMD-160.
TMessageDigest *TStackOperator_OP_HASH160::createHasher() const
{
	return new TDoubleHash( new THash_ripemd160, new THash_sha256 );
}

//
// Function:  OP_HASH256
// Input:     in
// Output:    hash
// Operation: The input is hashed two times with SHA-256.
TMessageDigest *TStackOperator_OP_HASH256::createHasher() const
{
	return new TDoubleHash( new THash_sha256, new THash_sha256 );
}

//
// Function:  OP_CODESEPARATOR
// Input:     Nothing
// Output:    Nothing
// Operation: All of the signature checking words will only match
// signatures to the data after the most recently-executed
// OP_CODESEPARATOR.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_CODESEPARATOR::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:	OP_CHECKSIG
// Description:
//
//  - Input:  sig pubkey
//  - Output: True / false
//
// The entire transaction's outputs, inputs, and script (from the most
// recently-executed OP_CODESEPARATOR to the end) are hashed.  The
// signature used by OP_CHECKSIG must be a valid signature for this hash
// and public key. If it is, 1 is returned, 0 otherwise.
//
// 1. The public key and the signature are popped from the stack, in
//    that order.
// 2. A new subscript is created from the instruction from the most
//    recent OP_CODESEPARATOR to the end of the script. If there is no
//    OP_CODESEPARATOR the entire script becomes the subscript (hereby
//    referred to as subScript)
// 3. The sig is deleted from subScript.
// 4. The hashtype is removed from the last byte of the sig and stored
// 5. A deep copy is made of the current transaction (hereby referred to
//    txCopy)
// 6. All OP_CODESEPARATORS are removed from subScript
// 7. The scripts for all transaction inputs in txCopy are set to empty
//    scripts
// 8. The script for the current transaction input in txCopy is set to
//    subScript
//
void TStackOperator_OP_CHECKSIG::explode( TBitcoinScriptBase *Script ) const
{
	Script->append( new TStackOperator_INTOP_PUSHSUBSCRIPT );
	Script->append( new TStackOperator_INTOP_DELETESIG );
	Script->append( new TStackOperator_INTOP_REMOVEHASHTYPE );
	Script->append( new TStackOperator_INTOP_PUSHCOPYTRANSACTION );
	Script->append( new TStackOperator_INTOP_REMOVECODESEPARATORS );
	Script->append( new TStackOperator_INTOP_REMOVETXSCRIPTS );
	Script->append( new TStackOperator_INTOP_REPLACETXSCRIPT );
	Script->append( new TStackOperator_INTOP_SIGHASH );
	Script->append( new TStackOperator_INTOP_FINALSIGNATURE );
}

//
// Function:  OP_CHECKSIGVERIFY
// Input:     sig pubkey
// Output:    True / false
// Operation: Same as OP_CHECKSIG, but OP_VERIFY is executed afterward.
void TStackOperator_OP_CHECKSIGVERIFY::explode( TBitcoinScriptBase *Script ) const
{
	Script->append( new TStackOperator_OP_CHECKSIG );
	Script->append( new TStackOperator_OP_VERIFY );
}

//
// Function:  OP_CHECKMULTISIG
// Input:     sig1 sig2 ... <number of signatures> pub1 pub2 <number of public keys>
// Output:    True / False
// Operation: For each signature and public key pair, OP_CHECKSIG is
// executed. If more public keys than signatures are listed, some
// key/sig pairs can fail. All signatures need to match a public key. If
// all signatures are valid, 1 is returned, 0 otherwise.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_CHECKMULTISIG::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  OP_CHECKMULTISIGVERIFY
// Input:     sig1 sig2 ... <number of signatures> pub1 pub2 ... <number of public keys>
// Output:    True / False
// Operation: Same as OP_CHECKMULTISIG, but OP_VERIFY is executed afterward.
void TStackOperator_OP_CHECKMULTISIGVERIFY::explode( TBitcoinScriptBase *Script ) const
{
	Script->append( new TStackOperator_OP_CHECKMULTISIGVERIFY );
	Script->append( new TStackOperator_OP_VERIFY );
}

//
// Function:  OP_INVALIDOPCODE
// Operation: Matches any opcode that is not yet assigned.
TBitcoinScript::tInstructionPointer
TStackOperator_OP_INVALIDOPCODE::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.Invalid = true;
	throw script_run_verify_error();
}

//
// Function:  OP_N
TBitcoinScript::tInstructionPointer
TStackOperator_OP_N::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.give( new TStackElementBigInteger( OP-OP_2 ) );

	return ip;
}

//
// Function:	TStackOperator_PUSH_N :: printOn
// Description:
//
ostream &TStackOperator_PUSH_N::printOn( ostream &os ) const
{
	os << "TStackOperator_PUSH_" << Raw.size() << "(";
	TLog::hexify(os, Raw);
	os << ")";
	return os;
}

//
// Function:  PUSH_N
TBitcoinScript::tInstructionPointer
TStackOperator_PUSH_N::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.give( new TStackElementString( Raw ) );

	return ip;
}

//
// Function:  OP_RESERVED
// Operation: Transaction is invalid
TBitcoinScript::tInstructionPointer
TStackOperator_OP_RESERVED::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.Invalid = true;
	throw script_run_verify_error();
}

//
// Function:  OP_VER
// Operation: Transaction is invalid
TBitcoinScript::tInstructionPointer
TStackOperator_OP_VER::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.Invalid = true;
	throw script_run_verify_error();
}

//
// Function:  OP_VERIF
// Operation: Transaction is invalid
TBitcoinScript::tInstructionPointer
TStackOperator_OP_VERIF::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.Invalid = true;
	throw script_run_verify_error();
}

//
// OP_VERNOTIF
// Transaction is invalid
TBitcoinScript::tInstructionPointer
TStackOperator_OP_VERNOTIF::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.Invalid = true;
	throw script_run_verify_error();
}

//
// Function:  OP_RESERVED1
// Operation: Transaction is invalid
TBitcoinScript::tInstructionPointer
TStackOperator_OP_RESERVED1::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.Invalid = true;
	throw script_run_verify_error();
}

//
// Function:  OP_RESERVED2
// Operation: Transaction is invalid
TBitcoinScript::tInstructionPointer
TStackOperator_OP_RESERVED2::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	Stack.Invalid = true;
	throw script_run_verify_error();
}

// ---------

//
// Function:  TStackOperator_INTOP_PUSHSUBSCRIPT
// Operation:
TBitcoinScript::tInstructionPointer
TStackOperator_INTOP_PUSHSUBSCRIPT::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  TStackOperator_INTOP_DELETESIG
// Operation:
TBitcoinScript::tInstructionPointer
TStackOperator_INTOP_DELETESIG::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  TStackOperator_INTOP_REMOVEHASHTYPE
// Operation:
TBitcoinScript::tInstructionPointer
TStackOperator_INTOP_REMOVEHASHTYPE::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  TStackOperator_INTOP_PUSHCOPYTRANSACTION
// Operation:
TBitcoinScript::tInstructionPointer
TStackOperator_INTOP_PUSHCOPYTRANSACTION::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  TStackOperator_INTOP_REMOVECODESEPARATORS
// Operation:
TBitcoinScript::tInstructionPointer
TStackOperator_INTOP_REMOVECODESEPARATORS::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  TStackOperator_INTOP_REMOVETXSCRIPTS
// Operation:
TBitcoinScript::tInstructionPointer
TStackOperator_INTOP_REMOVETXSCRIPTS::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  TStackOperator_INTOP_REPLACETXSCRIPT
// Operation:
TBitcoinScript::tInstructionPointer
TStackOperator_INTOP_REPLACETXSCRIPT::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  TStackOperator_INTOP_SIGHASH
// Operation:
TBitcoinScript::tInstructionPointer
TStackOperator_INTOP_SIGHASH::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{

	return ip;
}

//
// Function:  TStackOperator_INTOP_FINALSIGNATURE
// Operation:
TBitcoinScript::tInstructionPointer
TStackOperator_INTOP_FINALSIGNATURE::execute( TExecutionContext &Stack, const TBitcoinScript::tInstructionPointer &ip ) const
{
	// XXX: COMPLETELY WRONG

	auto_ptr<TStackElement> pk( Stack.take() );
	auto_ptr<TStackElement> s( Stack.take() );
	TStackElementString *PublicKey = dynamic_cast<TStackElementString*>(pk.get());
	TStackElementString *Signature = dynamic_cast<TStackElementString*>(s.get());

	if( PublicKey == NULL || Signature == NULL )
		throw script_run_error( "Invalid parameter type given to OP_CHECKSIG" );

	bool Verified = false;

	Stack.give( new TStackElementBoolean( Verified ) );

	return ip;
}


// -------------- Class member definitions


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include <sstream>
#include "logstream.h"
#include "unittest.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
		log() << "--- Testing parser" << endl;

		const string *p = UNITTESTSampleScripts;
		while( !p->empty() ) {
			istringstream iss(*p);
			TExecutionContext S;
			TBitcoinScript_1 BCP;

			BCP.read(iss, TBitcoinScript::AuthorisationScript );

			BCP.execute( S );

			p++;
		}
	} catch( exception &e ) {
		log() << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

