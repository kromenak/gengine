/* This code is added to the top of sheep.tab.cc */
%code top {
#include <cstdlib>
#include <string>
#include "SheepScriptBuilder.h"
//#define YYSTYPE SheepNode*
}

/* Tell bison to generate C++ output */
%skeleton "lalr1.cc"

/* We use features that require Bison 3.0+ */
%require "3.0"

/* Tell bison to write tokens to .hh file */
%defines

/* Tell bison to output parser named Sheep::Parser, instead of yy::parser */
%define api.namespace {Sheep}
%define parser_class_name {Parser}

/* When enabled w/ variants option, creates an actual "symbol" internal class. */
%define api.token.constructor

/* Use variant implementation of semantic values - fancy! */
%define api.value.type variant

/* Helps catch invalid uses. */
%define parse.assert

/* This code is added to the top of sheep.tab.hh. */
%code requires 
{
	namespace Sheep
	{
		class Scanner;
		class Driver;
	}
	class SheepScriptBuilder;
}

%param { Sheep::Scanner& scanner }
%param { Sheep::Driver& driver }
%param { SheepScriptBuilder& builder }

/* This code is added to the top of the .cc file for the parser. */
%code
{
	#include "SheepDriver.h"

	// When requesting yylex, redirect to scanner.yylex.
	#undef yylex
	#define yylex scanner.yylex

	// The C++ yytext is const, but we need to modify to remove
	// quotes in some cases. So, this is a bit of a hack.
	#undef yytext
	#define yytext scanner.GetYYText()

	void Sheep::Parser::error(const location_type& loc, const std::string& msg)
	{
		driver.error(loc, msg);
	}
}

/* Causes location to be tracked, and yylloc value is populated for use. */
%locations
/* TODO: Add %initial-action block to set filename for location variable */

%token END 0 "end of file"

/* section keywords */
%token CODE SYMBOLS

/* logical branching */
%token IF ELSE GOTO RETURN

/* various other symbols for a c-like language */
%token DOLLAR COMMA COLON SEMICOLON QUOTE
%token OPENPAREN CLOSEPAREN OPENBRACKET CLOSEBRACKET

/* pretty critical keyword for scripting synchronization */
%token WAIT

/* Some other keywords outlined in the language doc, but not sure if they are used */
%token YIELD EXPORT BREAKPOINT SITNSPIN

/* variable types */
%token INTVAR FLOATVAR STRINGVAR

/* constant types */
%token <int> INT
%token <float> FLOAT
%token <std::string> STRING

/* symbol dictating a user-defined name (variable or function) */
%token <std::string> USERID
%token <std::string> SYSID

%type <int> int_expr
%type <float> float_expr

/* %left dictates left associativity, which means multiple operators at once will group
   to the left first. Ex: x OP y OP z with '%left OP' means ((x OP y) OP z).

   Also, the order here defines precendence of associativity. The ones declared later
   will group FIRST. Ex: NOT is the lowest on the list so that it will group before
   anything else.

   For pretty much ALL operators, we want left associativity 

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
script: %empty 								{ std::cout << "EMPTY SCRIPT" << std::endl; }
	| symbols_section						{ std::cout << "SYMBOLS ONLY SCRIPT" << std::endl; }
	| code_section 							{ std::cout << "CODE ONLY SCRIPT" << std::endl; }
	| symbols_section code_section 			{ std::cout << "SYMBOLS AND CODE SCRIPT" << std::endl; }
	;

/* symbols starts with "symbols {" and ends with "}"
   between brackets is zero or more symbols */
symbols_section: SYMBOLS OPENBRACKET symbol_decls CLOSEBRACKET { std::cout << "SYMBOLS" << std::endl; }

/* zero or more symbols */
symbol_decls: %empty 				{  }
	| symbol_decls symbol_decl      {  }
	;

/* all symbol declarations must start with a value type (int, float, string)
   and end with a semicolon. Between, there must be one or more 
   symbol names and default values specified */
symbol_decl: INTVAR symbol_decl_int SEMICOLON 		{ std::cout << "INT SYMBOL" << std::endl; }
	| FLOATVAR symbol_decl_float SEMICOLON 			{ std::cout << "FLOAT SYMBOL" << std::endl; }
	| STRINGVAR symbol_decl_string SEMICOLON 		{ std::cout << "STRING SYMBOL" << std::endl; }
	;

symbol_decl_int: USERID 							{ builder.AddIntVariable($1, 0); std::cout << "SYMBOL " << $1 << std::endl; } /* myInt$ */
	| USERID ASSIGN INT 							{ builder.AddIntVariable($1, $3); std::cout << "SYMBOL " << $1 << " EQUALS " << $3 << std::endl; }	/* myInt$ = 100 */
	| symbol_decl_int COMMA USERID 					{ builder.AddIntVariable($3, 0); std::cout << "AND SYMBOL " << $3 << std::endl; }	/* <others>, myInt2$ */
	| symbol_decl_int COMMA USERID ASSIGN INT  		{ builder.AddIntVariable($3, $5); std::cout << "AND SYMBOL " << $3 << " EQUALS " << $5 << std::endl; }	/* <others>, myInt2$ = 10 */
	;

symbol_decl_float: USERID 							{ builder.AddFloatVariable($1, 0.0f); std::cout << "SYMBOL " << $1 << std::endl; }
	| USERID ASSIGN FLOAT 							{ builder.AddFloatVariable($1, $3); std::cout << "SYMBOL " << $1 << " EQUALS " << $3 << std::endl; }
	| symbol_decl_float COMMA USERID    			{ builder.AddFloatVariable($3, 0.0f); std::cout << "AND SYMBOL " << $3 << std::endl; }
	| symbol_decl_float COMMA USERID ASSIGN FLOAT   { builder.AddFloatVariable($3, $5); std::cout << "AND SYMBOL " << $3 << " EQUALS " << $5 << std::endl; }
	;

symbol_decl_string: USERID 							{ builder.AddStringVariable($1, ""); std::cout << "SYMBOL " << $1 << std::endl; }
	| USERID ASSIGN STRING 							{ builder.AddStringVariable($1, $3); std::cout << "SYMBOL " << $1 << " EQUALS " << $3 << std::endl; }
	| symbol_decl_string COMMA USERID 				{ builder.AddStringVariable($3, ""); std::cout << "AND SYMBOL " << $3 << std::endl; }
	| symbol_decl_string COMMA USERID ASSIGN STRING { builder.AddStringVariable($3, $5); std::cout << "AND SYMBOL " << $3 << " EQUALS " << $5 << std::endl; }
	;


/* code section starts with "code {" and ends with "}"
   between brackets is zero or more functions */
code_section: CODE OPENBRACKET functions CLOSEBRACKET { std::cout << "CODE" << std::endl; }
	;

/* functions in code section is either zero or more function */
functions: %empty 			{ }
	| functions function 	{ }
	;

/* all functions start with like "test$() {" and end with "}"
   the internals is zero or more statements */
function: USERID OPENPAREN CLOSEPAREN OPENBRACKET statements CLOSEBRACKET { builder.AddFunction($1); std::cout << "FUNCTION " << $1 << std::endl; }
	;

/* statements in a function is either zero or more statement */
statements: %empty
	| statements statement
	;

/* a statement is any individual line inside of a function */
statement: USERID ASSIGN expr SEMICOLON 				{ std::cout << "STMNT USERID ASSIGN expr" << std::endl; } /* myInt$ = 2 + 8; */
	| expr SEMICOLON									/* sys func call */
	| RETURN SEMICOLON 									{ builder.ReturnV(); } /* return; */
	| BREAKPOINT SEMICOLON 								{ builder.Breakpoint(); } /* breakpoint; */
	| SITNSPIN SEMICOLON 								{ builder.SitNSpin(); } /* sitnspin; */
	| GOTO USERID SEMICOLON 							/* goto blah$; */
	| USERID COLON										/* blah$: */
	| WAIT SEMICOLON 									{ builder.BeginWait(); builder.EndWait(); } /* wait; */
	| WAIT { builder.BeginWait(); } sysfunc_call 		{ builder.EndWait(); } /* wait WalkTo("Gab", "FR_25"); */
	| WAIT { builder.BeginWait(); } OPENBRACKET statements CLOSEBRACKET 	{ builder.EndWait(); } /* wait { // stuff } */
	| statements_block 									/* { // stuff } */
	| SEMICOLON 										{ std::cout << "SEMICOLON" << std::endl; } /* ; */
	| if_else_block										/* if(blah) { } else if(foo) { } else { } */
	;

/* just an indented section with MORE statements */
statements_block: OPENBRACKET statements CLOSEBRACKET
	;

/* a sysfunc call is like GetEgoName(); or PrintString("Ahh");
   it can have zero or more arguments */
sysfunc_call: SYSID OPENPAREN sysfunc_call_args CLOSEPAREN { std::cout << "SYSFUNC CALL" << std::endl; }
	;

/* sysfunc call args are either empty, or a list or args */
sysfunc_call_args: %empty				/* No arg */
	| expr 								{ std::cout << "SYSFUNC ARG " << std::endl; } /* 5 + 2 */
	| sysfunc_call_args COMMA expr 		/* 5 + 2, "Gab", GetCameraFov() */
	;

/* an expression is any constant value, math operation, system function call */ 
expr: sysfunc_call 						{ std::cout << "EXPR SYSFUNC CALL" << std::endl; }  /* PrintString("Ahhh") */
	| USERID 							{ builder.Load($1); std::cout << "EXPR USERID " << $1 << std::endl; } 
	| int_expr 							{ }
	| float_expr 						{ }
	| STRING 							{ builder.AddStringConst($1); builder.PushS($1); $$ = $1; std::cout << "STRING " << $1 << std::endl; }
	| OPENPAREN expr CLOSEPAREN 		{ std::cout << "EXPR (EXPR)" << std::endl; } /* (10 + 12) */
	;

int_expr: INT 							{ builder.PushI($1); $$ = $1; std::cout << "INT " << $1 << std::endl; }
	| int_expr PLUS int_expr			{ builder.AddI(); }
	| int_expr MINUS int_expr			{ builder.SubtractI(); }
	| int_expr MULTIPLY int_expr 		{ builder.MultiplyI(); }
	| int_expr DIVIDE int_expr 			{ builder.DivideI(); }
	| int_expr MOD int_expr 			{ builder.Modulo(); }
	| NEGATE int_expr					{ builder.NegateI(); }

	| int_expr LT int_expr 				{ builder.IsLessThanI(); }
	| int_expr GT int_expr 				{ builder.IsGreaterThanI(); }
	| int_expr LTE int_expr 			{ builder.IsLessThanEqualI(); }
	| int_expr GTE int_expr 			{ builder.IsGreaterThanEqualI(); }
	| int_expr NOTEQUAL int_expr      	{ builder.IsNotEqualI(); }
	| int_expr EQUAL int_expr			{ builder.IsEqualI(); }
	| int_expr OR int_expr				{ builder.Or(); }
	| int_expr AND int_expr  			{ builder.And(); }
	| NOT int_expr  					{ builder.Not(); }
	;

float_expr: FLOAT 						{ builder.PushF($1); $$ = $1; std::cout << "FLOAT " << $1 << std::endl; }
	| float_expr PLUS float_expr       	{ builder.AddF(); }
	| float_expr MINUS float_expr		{ builder.SubtractF(); }
	| float_expr MULTIPLY float_expr	{ builder.MultiplyF(); }
	| float_expr DIVIDE float_expr		{ builder.DivideF(); }
	| NEGATE float_expr 				{ builder.NegateF(); }

	| float_expr LT float_expr			{ builder.IsLessThanF(); }
	| float_expr GT float_expr			{ builder.IsGreaterThanF(); }
	| float_expr LTE float_expr			{ builder.IsLessThanEqualF(); }
	| float_expr GTE float_expr			{ builder.IsGreaterThanEqualF(); }
	| float_expr NOTEQUAL float_expr	{ builder.IsNotEqualF(); }
	| float_expr EQUAL float_expr		{ builder.IsEqualF(); }
	| float_expr OR float_expr			{ builder.Or(); }
	| float_expr AND float_expr			{ builder.And(); }
	| NOT float_expr					{ builder.Not(); }
	;

if_else_block: if_statement
	| if_statement else_statement
	;

if_statement: IF OPENPAREN expr CLOSEPAREN statements_block 	/* if(1) { // stuff } */
	;

else_statement: ELSE statements_block 	/* else { // stuff } */
	| ELSE if_else_block { } 			/* else if(1) { // stuff } */
	;

%%

/* No epilogue is needed. */

