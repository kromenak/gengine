/* This code is added to the top of sheep.tab.cc */
%code top 
{
	#include <cstdlib>
	#include <string>
}

/* Tell bison to generate C++ output */
%language "C++"
%skeleton "lalr1.cc"

/* We use features that require Bison 3.0+ */
/* As of writing, latest is v3.4, but macOS only has 3.0.2 available by default. */
%require "3.0"

/* Tell bison to write tokens to .hh file */
%defines

/* Tell bison to output parser named Sheep::Parser, instead of yy::parser */
%define api.namespace {Sheep}
%define api.parser.class {Parser}

/* When enabled w/ variants option, creates an actual "symbol" internal class. */
%define api.token.constructor

/* Use variant implementation of semantic values - fancy! */
%define api.value.type variant

/* Helps catch invalid uses. */
%define parse.assert

/* Output more verbose error messages. */
%define parse.error verbose

/* This code is added to the top of sheep.tab.hh. */
%code requires 
{
	class SheepScanner;
	class SheepCompiler;
	class SheepScriptBuilder;
	#include "SheepVM.h" // Declarations for SheepValue and SheepValueType
}

/* Specifies (in order) parameters that are passed in for us to use in our code. Note that this matches the YY_DECL from the Flex file. */
%param { SheepScanner& scanner }
%param { SheepCompiler& compiler }
%param { SheepScriptBuilder& builder }

/* This code is added to the top of the .cc file for the parser. */
%code
{
	#include "SheepCompiler.h"
	#include "SheepScriptBuilder.h"

	// When requesting yylex, redirect to scanner.yylex.
	#undef yylex
	#define yylex scanner.yylex

	// The C++ yytext is const, but we need to modify to remove
	// quotes in some cases. So, this is a bit of a hack.
	#undef yytext
	#define yytext scanner.GetYYText()

    // In case of error, just pass that back to our SheepCompiler class to handle.
    // This is probably the main reason we need to pass in the compiler reference!
	void Sheep::Parser::error(const location_type& loc, const std::string& msg)
	{
		compiler.Error(&builder, loc, msg);
	}

	//#define BUILDER_ERROR_CHECK if(builder.CheckError(yyla.location, *this)) { YYERROR; }
}

/* Causes location to be tracked, and yylloc value is populated for use. */
%locations
/* TODO: Add %initial-action block to set filename for location variable */

/* 
	TOKENS
   	Declare all the unique tokens that are used to generate/identify more complex types in the code.
*/
%token END 0 "end of file"

/* section keywords */
%token CODE SYMBOLS

/* logical branching */
%token IF ELSE GOTO RETURN

/* various other symbols for a c-like language */
%token COMMA ","
%token COLON ":"
%token SEMICOLON ";"
%token OPENPAREN "("
%token CLOSEPAREN ")"
%token OPENBRACKET "{"
%token CLOSEBRACKET "}"

/* TODO: These tokens are not used below, so do we really need them at all? */
%token DOLLAR QUOTE 

/* pretty critical keyword for scripting synchronization */
%token WAIT "wait"

/* Some other keywords outlined in the language doc, but not sure if they are used */
%token YIELD EXPORT BREAKPOINT SITNSPIN

/* variable types */
%token INTVAR FLOATVAR STRINGVAR

/* literals - we're also indicating the type for these guys */
%token <int> INT
%token <float> FLOAT
%token <std::string> STRING

/* symbol dictating a user-defined name (variable or function) or a system name (function) */
%token <std::string> USERID
%token <std::string> SYSID

/*
	TYPES
	Here we declare what the "type" is of certain complex/non-terminal symbols are.

	For example, an "expr" (e.g. 4 + 4) has a type as it's output.
	Similarly, a system function always returns a value (even void "under the hood" for simplicity).

	We use "SheepValue" as the type. This is defined in C++ as a union with int/float/string values.
	We are essentially saying that the type of these symbols is one of those, and we'll figure it out in code which is which.
*/
%type <SheepValue> expr
%type <SheepValue> sysfunc_call

/* 
   OPERATOR ASSOCIATIVITY
   %left dictates left associativity, which means multiple operators at once will group
   to the left first. Ex: x OP y OP z with '%left OP' means ((x OP y) OP z).

   Also, the order here defines precendence of associativity. The ones declared later
   will group FIRST. Ex: NOT is the lowest on the list so that it will group before
   anything else.

   For _most_ operators, we want left associativity.
   Since Sheep is C-like, we base the precendences off of C (http://en.cppreference.com/w/c/language/operator_precedence)
   */
%right ASSIGN
%left OR
%left AND
%left EQUAL NOTEQUAL
%left LT LTE GT GTE
%left PLUS MINUS
%left MULTIPLY DIVIDE MOD
%right NOT NEGATE

%start script

%%
/* Grammer rules */

/* script can be empty, just symbols, just code, or both symbols and code */
script: %empty 								{ }	/* just an empty file :( */
	| symbols_section						{ } /* symbols { } */
	| code_section 							{ } /* code { } */
	| symbols_section code_section 			{ } /* symbols { } code { } */
	| OPENBRACKET statements CLOSEBRACKET   { } /* supports a sheep "snippet" or anonymous function, as used in NVC assets */
	;

/* symbols starts with "symbols {" and ends with "}"
   between brackets is zero or more symbols */
symbols_section: SYMBOLS OPENBRACKET { builder.BeginSymbols(); } symbol_decls CLOSEBRACKET { }

/* zero or more symbols */
symbol_decls: %empty 				{  }
	| symbol_decls symbol_decl      {  }
	;

/* all symbol declarations must start with a value type (int, float, string)
   and end with a semicolon. Between, there must be one or more 
   symbol names and default values specified */
symbol_decl: INTVAR symbol_decl_int SEMICOLON 		{ }
	| FLOATVAR symbol_decl_float SEMICOLON 			{ }
	| STRINGVAR symbol_decl_string SEMICOLON 		{ }
	;

symbol_decl_int: USERID 							{ builder.AddIntVariable($1, 0); } 		/* myInt$ */
	| USERID ASSIGN INT 							{ builder.AddIntVariable($1, $3); }		/* myInt$ = 100 */
	| symbol_decl_int COMMA USERID 					{ builder.AddIntVariable($3, 0); }		/* <others>, myInt2$ */
	| symbol_decl_int COMMA USERID ASSIGN INT  		{ builder.AddIntVariable($3, $5); }		/* <others>, myInt2$ = 10 */
	;

symbol_decl_float: USERID 							{ builder.AddFloatVariable($1, 0.0f); }
	| USERID ASSIGN FLOAT 							{ builder.AddFloatVariable($1, $3); }
	| symbol_decl_float COMMA USERID    			{ builder.AddFloatVariable($3, 0.0f); }
	| symbol_decl_float COMMA USERID ASSIGN FLOAT   { builder.AddFloatVariable($3, $5); }
	;

symbol_decl_string: USERID 							{ builder.AddStringVariable($1, ""); }
	| USERID ASSIGN STRING 							{ builder.AddStringVariable($1, $3); }
	| symbol_decl_string COMMA USERID 				{ builder.AddStringVariable($3, ""); }
	| symbol_decl_string COMMA USERID ASSIGN STRING { builder.AddStringVariable($3, $5); }
	;


/* code section starts with "code {" and ends with "}"
   between brackets is zero or more functions */
code_section: CODE OPENBRACKET { builder.BeginCode(); } functions CLOSEBRACKET { }
	;

/* functions in code section is either zero or more function */
functions: %empty 			{ }
	| functions function 	{ }
	;

/* all functions start with like "test$() {" and end with "}"
   the internals is zero or more statements */
function: USERID OPENPAREN CLOSEPAREN { builder.StartFunction($1); } OPENBRACKET statements CLOSEBRACKET { builder.EndFunction($1); }
	;

/* statements in a function is either zero or more statement */
statements: %empty
	| statements statement
	;

/* a statement is any individual line inside of a function */
statement: USERID ASSIGN expr SEMICOLON 				{ builder.Store($1, @$); } /* myInt$ = 2 + 8; */
	| expr 												{ } /* sys func call or pointless expression like (2+4); */
	| RETURN SEMICOLON 									{ builder.ReturnV(); } /* return; */
	| BREAKPOINT SEMICOLON 								{ builder.Breakpoint(); } /* breakpoint; */
	| SITNSPIN SEMICOLON 								{ builder.SitnSpin(); } /* sitnspin; */
	| GOTO USERID SEMICOLON 							{ builder.BranchGoto($2); } /* goto blah$; */
	| USERID COLON										{ builder.AddGoto($1, @$); } /* blah$: */
	| WAIT SEMICOLON 									{ builder.BeginWait(); builder.EndWait(); } /* wait; */
	| WAIT { builder.BeginWait(); } sysfunc_call 		{ builder.EndWait(); } /* wait WalkTo("Gab", "FR_25"); */
	| WAIT { builder.BeginWait(); } OPENBRACKET statements CLOSEBRACKET { builder.EndWait(); } /* wait { // stuff } */
	| statements_block 									/* { // stuff } */
	| SEMICOLON 										{ } /* ; */
	| { builder.BeginIfElseBlock(); } if_else_block 	{ builder.EndIfElseBlock(); } /* if(blah) { } else if(foo) { } else { } */
	;

/* just an indented section with MORE statements */
statements_block: OPENBRACKET statements CLOSEBRACKET
	;

/* an expression is any constant value, math operation, system function call */ 
expr: sysfunc_call 						{ $$ = $1; } 												/* PrintString("Ahhh") */
	| USERID 							{ auto type = builder.Load($1, @$); $$ = SheepValue(type); } 	/* foo$ */
	| INT								{ builder.PushI($1); $$ = SheepValue($1); }
	| FLOAT 							{ builder.PushF($1); $$ = SheepValue($1); }
	| STRING 							{ builder.AddStringConst($1); builder.PushS($1); $$ = SheepValue(""); }

	| expr PLUS expr					{ auto type = builder.Add($1, $3, @$); $$ = SheepValue(type); }
	| expr MINUS expr					{ auto type = builder.Subtract($1, $3, @$); $$ = SheepValue(type); }
	| expr MULTIPLY expr				{ auto type = builder.Multiply($1, $3, @$); $$ = SheepValue(type); }
	| expr DIVIDE expr					{ auto type = builder.Divide($1, $3, @$); $$ = SheepValue(type); }
	| expr MOD expr						{ builder.Modulo($1, $3, @$); $$ = SheepValue(SheepValueType::Int); } 
	| NEGATE expr 						{ builder.Negate($2, @$); }

	| expr LT expr 						{ auto type = builder.IsLess($1, $3, @$); $$ = SheepValue(type); }
	| expr GT expr						{ auto type = builder.IsGreater($1, $3, @$); $$ = SheepValue(type); }
	| expr LTE expr 					{ auto type = builder.IsLessEqual($1, $3, @$); $$ = SheepValue(type); }
	| expr GTE expr 					{ auto type = builder.IsGreaterEqual($1, $3, @$); $$ = SheepValue(type); }
	| expr EQUAL expr 					{ auto type = builder.IsEqual($1, $3, @$); $$ = SheepValue(type); }
	| expr NOTEQUAL expr 				{ auto type = builder.IsNotEqual($1, $3, @$); $$ = SheepValue(type); }
	| expr OR expr 						{ builder.Or($1, $3, @$); $$ = SheepValue(SheepValueType::Int); }
	| expr AND expr 					{ builder.And($1, $3, @$); $$ = SheepValue(SheepValueType::Int); }
	| NOT expr 							{ builder.Not(); $$ = $2; }

	| OPENPAREN expr CLOSEPAREN 		{ $$ = $2; } /* (10 + 12) */
	;

/* a sysfunc call is like GetEgoName(); or PrintString("Ahh");
   it can have zero or more arguments */
sysfunc_call: SYSID OPENPAREN sysfunc_call_args CLOSEPAREN { auto type = builder.CallSysFunc($1, @$); $$ = SheepValue(type); }
	;

/* sysfunc call args are either empty, or a list or args */
sysfunc_call_args: %empty				{ } /* No arg */
	| expr 								{ builder.AddSysFuncArg($1, @$); } /* 5 + 2 */
	| sysfunc_call_args COMMA expr 		{ builder.AddSysFuncArg($3, @$); } /* 5 + 2, "Gab", GetCameraFov() */
	;

if_else_block: if_statement
	| if_statement else_statement
	;

if_statement: IF OPENPAREN expr CLOSEPAREN { builder.BeginIfBlock(); } statements_block { builder.EndIfBlock(); } /* if(1) { // stuff } */
	;

else_statement: ELSE { builder.BeginElseBlock(); } statements_block { builder.EndElseBlock(); } /* else { // stuff } */
	| ELSE if_else_block 				{  } /* else if(1) { // stuff } */
	;

%%

/* No epilogue is needed. */

