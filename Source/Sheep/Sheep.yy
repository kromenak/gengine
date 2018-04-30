/* This code is added to the top of sheep.tab.cc */
%code top {
#include <cstdlib>
#include <string>
#include "SheepScript.h"
//#define YYSTYPE SheepNode*

char* removeQuotes(char* str)
{
	if(str[0] == '"')
	{
		str[strlen(str)-1] = 0;
		return str+1;
	}
	else // assume "|< >|" strings
	{
		str[strlen(str)-2] = 0;
		return str+2;
	}
}
%}

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
}

%param { Sheep::Scanner& scanner }
%param { Sheep::Driver& driver }

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

%type <int> int_exp
%type <float> float_exp

%type <int> expression

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
script: %empty
	| symbols
	| code
	| symbols code { }
	;

symbols: SYMBOLS OPENBRACKET variable_decls CLOSEBRACKET { }

variable_decls: variable_decls
	| variable_decl { }
	;

variable_decl: INTVAR variable_decl_int SEMICOLON
	| FLOATVAR variable_decl_float SEMICOLON
	| STRINGVAR variable_decl_string SEMICOLON { }
	;

variable_decl_int: %empty
	| USERID
	| USERID COMMA variable_decl_int
	| USERID ASSIGN INT
	| USERID ASSIGN INT COMMA variable_decl_int { }
	;

variable_decl_float: %empty
	| USERID
	| USERID COMMA variable_decl_float
	| USERID ASSIGN INT
	| USERID ASSIGN INT COMMA variable_decl_float { }

variable_decl_string: %empty
	| USERID
	| USERID COMMA variable_decl_string
	| USERID ASSIGN INT
	| USERID ASSIGN INT COMMA variable_decl_string { }

code: CODE OPENBRACKET functions CLOSEBRACKET { }

functions: %empty
	| functions
	| function { }
	;

function: USERID OPENPAREN CLOSEPAREN OPENBRACKET statements CLOSEBRACKET { }
	;

statements: %empty
	| statement { }
	;

statement: if_else_block
	| USERID ASSIGN expr SEMICOLON
	| expr SEMICOLON
	| RETURN SEMICOLON
	| BREAKPOINT SEMICOLON
	| SITNSPIN SEMICOLON
	| GOTO USERID SEMICOLON
	| USERID COLON
	| WAIT SEMICOLON
	| WAIT function_call
	| WAIT OPENBRACKET function_calls CLOSEBRACKET
	| block_statement { }
	;

if_else_block: if_statement
	| if_statement else_statement { }
	;

if_statement: IF OPENPAREN expr CLOSEPAREN block_statement { }
	;

else_statement: ELSE block_statement 
	| ELSE if_else_block { }
	;

block_statement: OPENBRACKET statements CLOSEBRACKET { }
	;

function_call: SYSID OPENPAREN expr CLOSEPAREN SEMICOLON
	| SYSID OPENPAREN expr OPENPAREN CLOSEPAREN SEMICOLON
	| SYSID OPENPAREN expr OPENPAREN expr CLOSEPAREN SEMICOLON
	;

expr: function_call
	| USERID
	| INT
	| FLOAT
	| STRING
	| OPENPAREN expr CLOSEPAREN
	| NEGATE expr
	| NOT expr
	| expr PLUS expr
	| expr MINUS expr
	| expr DIVIDE expr
	| expr MULTIPLY expr
	| expr MOD expr
	| expr LT expr
	| expr GT expr
	| expr LTE expr
	| expr GTE expr
	| expr NOTEQUAL expr
	| expr EQUAL expr
	| expr OR expr
	| expr AND expr { } 
	;

%%

/* No epilogue is needed. */

