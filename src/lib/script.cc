// ----------------------------------------------------------------------------
// Project: bitcoin
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
// --- Qt
// --- OS
// --- Project libs
// --- Project
#include "logstream.h"


// -------------- Namespace


// -------------- Module Globals


// -------------- World Globals (need "extern"s in header)


// -------------- Template instantiations


// -------------- Class declarations

//
// Function:	TExecutionStack :: TExecutionStack
// Description:
//
TExecutionStack::TExecutionStack() :
	Invalid( false )
{
}

//
// Function:	TExecutionStack :: push
// Description:
//
void TExecutionStack::push( int )
{
}

//
// Function:	TExecutionStack :: push
// Description:
//
void TExecutionStack::push( const string & )
{
}

// -----------

//
// Function:	TBitcoinProgram :: TBitcoinProgram
// Description:
//
TBitcoinProgram::TBitcoinProgram() :
	Initialised( false )
{
}

//
// Function:	TBitcoinProgram :: ~TBitcoinProgram
// Description:
//
TBitcoinProgram::~TBitcoinProgram()
{
	while( !Program.empty() ) {
		delete Program.front();
		Program.erase( Program.begin() );
	}
	while( !Templates.empty() ) {
		delete Templates.front();
		Templates.erase( Templates.begin() );
	}
}

//
// Function:	TBitcoinProgram :: init
// Description:
//
void TBitcoinProgram::init()
{
	Initialised = true;
}

//
// Function:	TBitcoinProgram :: read
// Description:
//
istream &TBitcoinProgram::read( istream &is )
{
	eScriptOp Opcode;
	list<const TStackOperatorFromStream *>::const_iterator it;

	if( !Initialised )
		init();

	while( (Opcode = static_cast<eScriptOp>(is.peek()) ) != ios::traits_type::eof() ) {
		for( it = Templates.begin(); it != Templates.end(); it++ ) {
			if( !(*it)->acceptOpcode( Opcode ) )
				continue;
			break;
		}
		if( it == Templates.end() ) {
			log() << "script opcode (" << Opcode << ") found" << endl;
			throw script_parse_error_not_found();
		}

		log() << (*it)->className() << " script opcode ("
			<< Opcode << ") found" << endl;
		TStackOperatorFromStream *Operator = (*it)->clone();

		Operator->read( is );
	}

	return is;
}

// -----------

//
// Function:	TBitcoinProgram_0 :: TBitcoinProgram_0
// Description:
//
TBitcoinProgram_0::TBitcoinProgram_0()
{
}

//
// Function:	TBitcoinProgram_0 :: getMinimumAcceptedVersion
// Description:
//
uint32_t TBitcoinProgram_0::getMinimumAcceptedVersion() const
{
	return 0;
}

//
// Function:	TBitcoinProgram_0 :: init
// Description:
//
void TBitcoinProgram_0::init()
{
	Templates.push_back( new TStackOperator_OP_FALSE );
	Templates.push_back( new TStackOperator_OP_PUSHDATA1 );
	Templates.push_back( new TStackOperator_OP_PUSHDATA2 );
	Templates.push_back( new TStackOperator_OP_PUSHDATA4 );
	Templates.push_back( new TStackOperator_OP_1NEGATE );
	Templates.push_back( new TStackOperator_OP_TRUE );
	Templates.push_back( new TStackOperator_OP_NOP );
	Templates.push_back( new TStackOperator_OP_IF );
	Templates.push_back( new TStackOperator_OP_NOTIF );
	Templates.push_back( new TStackOperator_OP_ELSE );
	Templates.push_back( new TStackOperator_OP_ENDIF );
	Templates.push_back( new TStackOperator_OP_VERIFY );
	Templates.push_back( new TStackOperator_OP_RETURN );
	Templates.push_back( new TStackOperator_OP_TOALTSTACK );
	Templates.push_back( new TStackOperator_OP_FROMALTSTACK );
	Templates.push_back( new TStackOperator_OP_IFDUP );
	Templates.push_back( new TStackOperator_OP_DEPTH );
	Templates.push_back( new TStackOperator_OP_DROP );
	Templates.push_back( new TStackOperator_OP_DUP );
	Templates.push_back( new TStackOperator_OP_NIP );
	Templates.push_back( new TStackOperator_OP_OVER );
	Templates.push_back( new TStackOperator_OP_PICK );
	Templates.push_back( new TStackOperator_OP_ROLL );
	Templates.push_back( new TStackOperator_OP_ROT );
	Templates.push_back( new TStackOperator_OP_SWAP );
	Templates.push_back( new TStackOperator_OP_TUCK );
	Templates.push_back( new TStackOperator_OP_2DROP );
	Templates.push_back( new TStackOperator_OP_2DUP );
	Templates.push_back( new TStackOperator_OP_3DUP );
	Templates.push_back( new TStackOperator_OP_2OVER );
	Templates.push_back( new TStackOperator_OP_2ROT );
	Templates.push_back( new TStackOperator_OP_2SWAP );
//	Templates.push_back( new TStackOperator_OP_CAT );
//	Templates.push_back( new TStackOperator_OP_SUBSTR );
//	Templates.push_back( new TStackOperator_OP_LEFT );
//	Templates.push_back( new TStackOperator_OP_RIGHT );
	Templates.push_back( new TStackOperator_OP_SIZE );
//	Templates.push_back( new TStackOperator_OP_INVERT );
//	Templates.push_back( new TStackOperator_OP_AND );
//	Templates.push_back( new TStackOperator_OP_OR );
//	Templates.push_back( new TStackOperator_OP_XOR );
	Templates.push_back( new TStackOperator_OP_EQUAL );
	Templates.push_back( new TStackOperator_OP_EQUALVERIFY );
	Templates.push_back( new TStackOperator_OP_1ADD );
	Templates.push_back( new TStackOperator_OP_1SUB );
//	Templates.push_back( new TStackOperator_OP_2MUL );
//	Templates.push_back( new TStackOperator_OP_2DIV );
	Templates.push_back( new TStackOperator_OP_NEGATE );
	Templates.push_back( new TStackOperator_OP_ABS );
	Templates.push_back( new TStackOperator_OP_NOT );
	Templates.push_back( new TStackOperator_OP_0NOTEQUAL );
	Templates.push_back( new TStackOperator_OP_ADD );
	Templates.push_back( new TStackOperator_OP_SUB );
//	Templates.push_back( new TStackOperator_OP_MUL );
//	Templates.push_back( new TStackOperator_OP_DIV );
//	Templates.push_back( new TStackOperator_OP_MOD );
//	Templates.push_back( new TStackOperator_OP_LSHIFT );
//	Templates.push_back( new TStackOperator_OP_RSHIFT );
	Templates.push_back( new TStackOperator_OP_BOOLAND );
	Templates.push_back( new TStackOperator_OP_BOOLOR );
	Templates.push_back( new TStackOperator_OP_NUMEQUAL );
	Templates.push_back( new TStackOperator_OP_NUMEQUALVERIFY );
	Templates.push_back( new TStackOperator_OP_NUMNOTEQUAL );
	Templates.push_back( new TStackOperator_OP_LESSTHAN );
	Templates.push_back( new TStackOperator_OP_GREATERTHAN );
	Templates.push_back( new TStackOperator_OP_LESSTHANOREQUAL );
	Templates.push_back( new TStackOperator_OP_GREATERTHANOREQUAL );
	Templates.push_back( new TStackOperator_OP_MIN );
	Templates.push_back( new TStackOperator_OP_MAX );
	Templates.push_back( new TStackOperator_OP_WITHIN );
	Templates.push_back( new TStackOperator_OP_RIPEMD160 );
	Templates.push_back( new TStackOperator_OP_SHA1 );
	Templates.push_back( new TStackOperator_OP_SHA256 );
	Templates.push_back( new TStackOperator_OP_HASH160 );
	Templates.push_back( new TStackOperator_OP_HASH256 );
	Templates.push_back( new TStackOperator_OP_CODESEPARATOR );
	Templates.push_back( new TStackOperator_OP_CHECKSIG );
	Templates.push_back( new TStackOperator_OP_CHECKSIGVERIFY );
	Templates.push_back( new TStackOperator_OP_CHECKMULTISIG );
	Templates.push_back( new TStackOperator_OP_CHECKMULTISIGVERIFY );
	Templates.push_back( new TStackOperator_OP_PUBKEYHASH );
	Templates.push_back( new TStackOperator_OP_PUBKEY );
	Templates.push_back( new TStackOperator_OP_RESERVED );
	Templates.push_back( new TStackOperator_OP_VER );
	Templates.push_back( new TStackOperator_OP_VERIF );
	Templates.push_back( new TStackOperator_OP_VERNOTIF );
	Templates.push_back( new TStackOperator_OP_RESERVED1 );
	Templates.push_back( new TStackOperator_OP_RESERVED2 );
	Templates.push_back( new TStackOperator_OP_NOP_N );
	Templates.push_back( new TStackOperator_OP_N );
	Templates.push_back( new TStackOperator_PUSH_N );
	// OP_INVALIDOPCODE _must_ be last in the template list, this is
	// because it is a special case and has an acceptOpcode() that
	// always returns true.  Therefore, it will take up any opcode not
	// previously accepted by one of the above.
	Templates.push_back( new TStackOperator_OP_INVALIDOPCODE );

	TBitcoinProgram::init();
}

// -----------

//
// Function:  OP_IF
// <expression> if [statements] [else [statements]] endif
//
// Operation: If the top stack value is 1, the statements are executed.
// The top stack value is removed.
//
void TStackOperator_OP_IF::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_NOTIF
// <expression> if [statements] [else [statements]] endif
//
// If the top stack value is 0, the statements are executed. The top
// stack value is removed.
//
void TStackOperator_OP_NOTIF::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_ELSE
// <expression> if [statements] [else [statements]] endif
//
// If the preceding OP_IF or OP_NOTIF was not executed then these
// statements are.
//
void TStackOperator_OP_ELSE::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_ENDIF
// <expression> if [statements] [else [statements]] endif
//
// Ends an if/else block.
//
void TStackOperator_OP_ENDIF::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_RETURN
// Input:     True / false
// Output:    Nothing / False
// Operation: Marks transaction as invalid if top stack value is not
// true. True is removed, but false is not.
//
void TStackOperator_OP_VERIFY::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_RETURN
// Input:     Nothing
// Outpu:     Nothing
// Operation: Marks transaction as invalid.
//
void TStackOperator_OP_RETURN::execute( TExecutionStack &Stack ) const
{
}

// Function:  OP_TOALTSTACK
// Input:     x1
// Output:    (alt)x1
// Operation: Puts the input onto the top of the alt stack. Removes it
// from the main stack.
void TStackOperator_OP_TOALTSTACK::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_FROMALTSTACK
// Input:     (alt)x1
// Output:    x1
// Operation: Puts the input onto the top of the main stack. Removes it
// from the alt stack.
void TStackOperator_OP_FROMALTSTACK::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_IFDUP
// Input:     x
// Output:    x / x x
// Operation: If the input is true or false, duplicate it.
void TStackOperator_OP_IFDUP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_DEPTH
// Input:     Nothing
// Output:    <Stack size>
// Operation: Puts the number of stack items onto the stack.
void TStackOperator_OP_DEPTH::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_DROP
// Input:     x
// Output:    Nothing
// Operation: Removes the top stack item.
void TStackOperator_OP_DROP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_DUP
// Input:     x
// Output:    x x
// Operation: Duplicates the top stack item.
void TStackOperator_OP_DUP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_NIP
// Input:     x1 x2
// Output:    x2
// Operation: Removes the second-to-top stack item.
void TStackOperator_OP_NIP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_OVER
// Input:     x1 x2
// Output:    x1 x2 x1
// Operation: Copies the second-to-top stack item to the top.
void TStackOperator_OP_OVER::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_PICK
// Input:     xn ... x2 x1 x0 <n>
// Output:    xn ... x2 x1 x0 xn
// Operation: The item n back in the stack is copied to the top.
void TStackOperator_OP_PICK::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_ROLL
// Input:     xn ... x2 x1 x0 <n>
// Output:    ... x2 x1 x0 xn
// Operation: The item n back in the stack is moved to the top.
void TStackOperator_OP_ROLL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_ROT
// Input:     x1 x2 x3
// Output:    x2 x3 x1
// Operation: The top three items on the stack are rotated to the left.
void TStackOperator_OP_ROT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_SWAP
// Input:     x1 x2
// Output:    x2 x1
// Operation: The top two items on the stack are swapped.
void TStackOperator_OP_SWAP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_TUCK
// Input:     x1 x2
// Output:    x2 x1 x2
// Operation: The item at the top of the stack is copied and inserted
// before the second-to-top item.
void TStackOperator_OP_TUCK::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2DROP
// Input:     x1 x2
// Output:    Nothing
// Operation: Removes the top two stack items.
void TStackOperator_OP_2DROP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2DUP
// Input:     x1 x2
// Output:    x1 x2 x1 x2
// Operation: Duplicates the top two stack items.
void TStackOperator_OP_2DUP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_3DUP
// Input:     x1 x2 x3
// Output:    x1 x2 x3 x1 x2 x3
// Operation: Duplicates the top three stack items.
void TStackOperator_OP_3DUP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2OVER
// Input:     x1 x2 x3 x4
// Output:    x1 x2 x3 x4 x1 x2
// Operation: Copies the pair of items two spaces back in the stack to
// the front.
void TStackOperator_OP_2OVER::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2ROT
// Input:     x1 x2 x3 x4 x5 x6
// Output:    x3 x4 x5 x6 x1 x2
// Operation: The fifth and sixth items back are moved to the top of the
// stack.
void TStackOperator_OP_2ROT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2SWAP
// Input:     x1 x2 x3 x4
// Output:    x3 x4 x1 x2
// Operation: Swaps the top two pairs of items.
void TStackOperator_OP_2SWAP::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_CAT
// Input:     x1 x2
// Output:    out
// Operation: Concatenates two strings. Currently disabled.
void TStackOperator_OP_CAT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_SUBSTR
// Input:     in begin size
// Output:    out
// Operation: Returns a section of a string. Currently disabled.
void TStackOperator_OP_SUBSTR::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_LEFT
// Input:     in size
// Output:    out
// Operation: Keeps only characters left of the specified point in a
// string. Currently disabled.
void TStackOperator_OP_LEFT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_RIGHT
// Input:     in size
// Output:    out
// Operation: Keeps only characters right of the specified point in a
// string. Currently disabled.
void TStackOperator_OP_RIGHT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_SIZE
// Input:     in
// Output:    in size
// Operation: Returns the length of the input string.
void TStackOperator_OP_SIZE::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_INVERT
// Input:     in
// Output:    out
// Operation: Flips all of the bits in the input. Currently disabled.
void TStackOperator_OP_INVERT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_AND
// Input:     x1 x2
// Output:    out
// Operation: Boolean and between each bit in the inputs. Currently
// disabled.
void TStackOperator_OP_AND::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_OR
// Input:     x1 x2
// Output:    out
// Operation: Boolean or between each bit in the inputs. Currently
// disabled.
void TStackOperator_OP_OR::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_XOR
// Input:     x1 x2
// Output:    out
// Operation: Boolean exclusive or between each bit in the inputs.
// Currently disabled.
void TStackOperator_OP_XOR::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_EQUAL
// Input:     x1 x2
// Output:    True / false
// Operation: Returns 1 if the inputs are exactly equal, 0 otherwise.
void TStackOperator_OP_EQUAL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_EQUALVERIFY
// Input:     x1 x2
// Output:    True / false
// Operation: Same as OP_EQUAL, but runs OP_VERIFY afterward.
void TStackOperator_OP_EQUALVERIFY::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_1ADD
// Input:     in
// Output:    out
// Operation: 1 is added to the input.
void TStackOperator_OP_1ADD::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_1SUB
// Input:     in
// Output:    out
// Operation: 1 is subtracted from the input.
void TStackOperator_OP_1SUB::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2MUL
// Input:     in
// Output:    out
// Operation: The input is multiplied by 2. Currently disabled.
void TStackOperator_OP_2MUL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_2DIV
// Input:     in
// Output:    out
// Operation: The input is divided by 2. Currently disabled.
void TStackOperator_OP_2DIV::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_NEGATE
// Input:     in
// Output:    out
// Operation: The sign of the input is flipped.
void TStackOperator_OP_NEGATE::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_ABS
// Input:     in
// Output:    out
// Operation: The input is made positive.
void TStackOperator_OP_ABS::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_NOT
// Input:     in
// Output:    out
// Operation: If the input is 0 or 1, it is flipped. Otherwise the
// output will be 0.
void TStackOperator_OP_NOT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_0NOTEQUAL
// Input:     in
// Output:    out
// Operation: Returns 1 if the input is 0. 0 otherwise.
void TStackOperator_OP_0NOTEQUAL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_ADD
// Input:     a b
// Output:    out
// Operation: a is added to b.
void TStackOperator_OP_ADD::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_SUB
// Input:     a b
// Output:    out
// Operation: b is subtracted from a.
void TStackOperator_OP_SUB::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_MUL
// Input:     a b
// Output:    out
// Operation: a is multiplied by b. Currently disabled.
void TStackOperator_OP_MUL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_DIV
// Input:     a b
// Output:    out
// Operation: a is divided by b. Currently disabled.
void TStackOperator_OP_DIV::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_MOD
// Input:     a b
// Output:    out
// Operation: Returns the remainder after dividing a by b. Currently
// disabled.
void TStackOperator_OP_MOD::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_LSHIFT
// Input:     a b
// Output:    out
// Operation: Shifts a left b bits, preserving sign. Currently disabled.
void TStackOperator_OP_LSHIFT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_RSHIFT
// Input:     a b
// Output:    out
// Operation: Shifts a right b bits, preserving sign. Currently disabled.
void TStackOperator_OP_RSHIFT::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_BOOLAND
// Input:     a b
// Output:    out
// Operation: If both a and b are not 0, the output is 1. Otherwise 0.
void TStackOperator_OP_BOOLAND::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_BOOLOR
// Input:     a b
// Output:    out
// Operation: If a or b is not 0, the output is 1. Otherwise 0.
void TStackOperator_OP_BOOLOR::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_NUMEQUAL
// Input:     a b
// Output:    out
// Operation: Returns 1 if the numbers are equal, 0 otherwise.
void TStackOperator_OP_NUMEQUAL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_NUMEQUALVERIFY
// Input:     a b
// Output:    out
// Operation: Same as OP_NUMEQUAL, but runs OP_VERIFY afterward.
void TStackOperator_OP_NUMEQUALVERIFY::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_NUMNOTEQUAL
// Input:     a b
// Output:    out
// Operation: Returns 1 if the numbers are not equal, 0 otherwise.
void TStackOperator_OP_NUMNOTEQUAL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_LESSTHAN
// Input:     a b
// Output:    out
// Operation: Returns 1 if a is less than b, 0 otherwise.
void TStackOperator_OP_LESSTHAN::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_GREATERTHAN
// Input:     a b
// Output:    out
// Operation: Returns 1 if a is greater than b, 0 otherwise.
void TStackOperator_OP_GREATERTHAN::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_LESSTHANOREQUAL
// Input:     a b
// Output:    out
// Operation: Returns 1 if a is less than or equal to b, 0 otherwise.
void TStackOperator_OP_LESSTHANOREQUAL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_GREATERTHANOREQUAL
// Input:     a b
// Output:    out
// Operation: Returns 1 if a is greater than or equal to b, 0 otherwise.
void TStackOperator_OP_GREATERTHANOREQUAL::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_MIN
// Input:     a b
// Output:    out
// Operation: Returns the smaller of a and b.
void TStackOperator_OP_MIN::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_MAX
// Input:     a b
// Output:    out
// Operation: Returns the larger of a and b.
void TStackOperator_OP_MAX::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_WITHIN
// Input:     x min max
// Output:    out
// Operation: Returns 1 if x is within the specified range
// (left-inclusive), 0 otherwise.
void TStackOperator_OP_WITHIN::execute( TExecutionStack &Stack ) const
{
}


//
// Function:  OP_RIPEMD160
// Input:     in
// Output:    hash
// Operation: The input is hashed using RIPEMD-160.
void TStackOperator_OP_RIPEMD160::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_SHA1
// Input:     in
// Output:    hash
// Operation: The input is hashed using SHA-1.
void TStackOperator_OP_SHA1::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_SHA256
// Input:     in
// Output:    hash
// Operation: The input is hashed using SHA-256.
void TStackOperator_OP_SHA256::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_HASH160
// Input:     in
// Output:    hash
// Operation: The input is hashed twice: first with SHA-256 and then
// with RIPEMD-160.
void TStackOperator_OP_HASH160::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_HASH256
// Input:     in
// Output:    hash
// Operation: The input is hashed two times with SHA-256.
void TStackOperator_OP_HASH256::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_CODESEPARATOR
// Input:     Nothing
// Output:    Nothing
// Operation: All of the signature checking words will only match
// signatures to the data after the most recently-executed
// OP_CODESEPARATOR.
void TStackOperator_OP_CODESEPARATOR::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_CHECKSIG
// Input:     sig pubkey
// Output:    True / false
// Operation: The entire transaction's outputs, inputs, and script (from
// the most recently-executed OP_CODESEPARATOR to the end) are hashed.
// The signature used by OP_CHECKSIG must be a valid signature for this
// hash and public key. If it is, 1 is returned, 0 otherwise.
void TStackOperator_OP_CHECKSIG::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_CHECKSIGVERIFY
// Input:     sig pubkey
// Output:    True / false
// Operation: Same as OP_CHECKSIG, but OP_VERIFY is executed afterward.
void TStackOperator_OP_CHECKSIGVERIFY::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_CHECKMULTISIG
// Input:     sig1 sig2 ... <number of signatures> pub1 pub2 <number of public keys>
// Output:    True / False
// Operation: For each signature and public key pair, OP_CHECKSIG is
// executed. If more public keys than signatures are listed, some
// key/sig pairs can fail. All signatures need to match a public key. If
// all signatures are valid, 1 is returned, 0 otherwise.
void TStackOperator_OP_CHECKMULTISIG::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_CHECKMULTISIGVERIFY
// Input:     sig1 sig2 ... <number of signatures> pub1 pub2 ... <number of public keys>
// Output:    True / False
// Operation: Same as OP_CHECKMULTISIG, but OP_VERIFY is executed afterward.
void TStackOperator_OP_CHECKMULTISIGVERIFY::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_PUBKEYHASH
// Operation: Represents a public key hashed with OP_HASH160.
void TStackOperator_OP_PUBKEYHASH::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_PUBKEY
// Operation: Represents a public key compatible with OP_CHECKSIG.
void TStackOperator_OP_PUBKEY::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_INVALIDOPCODE
// Operation: Matches any opcode that is not yet assigned.
void TStackOperator_OP_INVALIDOPCODE::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_RESERVED
// Operation: Transaction is invalid
void TStackOperator_OP_RESERVED::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_VER
// Operation: Transaction is invalid
void TStackOperator_OP_VER::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_VERIF
// Operation: Transaction is invalid
void TStackOperator_OP_VERIF::execute( TExecutionStack &Stack ) const
{
}

//
// OP_VERNOTIF
// Transaction is invalid
void TStackOperator_OP_VERNOTIF::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_RESERVED1
// Operation: Transaction is invalid
void TStackOperator_OP_RESERVED1::execute( TExecutionStack &Stack ) const
{
}

//
// Function:  OP_RESERVED2
// Operation: Transaction is invalid
void TStackOperator_OP_RESERVED2::execute( TExecutionStack &Stack ) const
{
}


// -------------- Class member definitions


// -------------- Function definitions


#ifdef UNITTEST
#include <iostream>
#include <sstream>
#include "unittest.h"

// -------------- main()

int main( int argc, char *argv[] )
{
	try {
		cerr << "--- Testing parser" << endl;

		const string *p = UNITTESTSampleScripts;
		while( !p->empty() ) {
			istringstream iss(*p);
			TBitcoinProgram_0 BCP;

			BCP.read(iss);

			p++;
		}
	} catch( exception &e ) {
		cerr << e.what() << endl;
		return 255;
	}

	return 0;
}
#endif

