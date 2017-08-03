%{
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

/* This code is added to the top of the .hh file for the parser. */
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

/* pretty critical keyword for scripting synchronization (like a Unity coroutine?) */
%token WAIT

/* Some other keywords outlined in the language doc, but not sure if they are used */
%token YIELD EXPORT BREAKPOINT SITNSPIN

/* variable types */
%token INTVAR FLOATVAR STRINGVAR

/* constant types */
%token <int> INT
%token <float> FLOAT
%token <std::string> STRING
/*%token INT FLOAT STRING*/

/* symbol dictating a user-defined name (variable or function) */
%token <std::string> USERNAME
%token <std::string> SYSNAME
/*%token USERNAME SYSNAME*/

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

script: /* empty */
	| expression_list { }
	;

expression_list: expression { std::cout << $1 << std::endl; }
	| expression_list expression { std::cout << $2 << std::endl; }
	;

expression: int_exp { $$ = $1; }
	| float_exp { $$ = $1; }
	;

int_exp: INT { $$ = $1; }
	| int_exp PLUS int_exp { $$ = $1 + $3; }
	;

float_exp: FLOAT { $$ = $1; }
	| float_exp PLUS float_exp { $$ = $1 + $3; }
	;

/* All possible constants: int, float, string */
/*
constant: INT { $$ = $1; }
	| FLOAT { $$ = $1; }
	| STRING { $$ = removeQuotes($1); }
	;
*/

/*
user_name: USERNAME { $$ = SheepNode::CreateNameRef(yytext, false); }
sys_name: SYSNAME { $$ = SheepNode::CreateNameRef(yytext, true); }
*/

/* All possible symbol types: int, float, string */
/*
symbol_type: INTVAR { $$ = SheepNode::CreateTypeRef(SheepReferenceType::Int); }
	| FLOATVAR { $$ = SheepNode::CreateTypeRef(SheepReferenceType::Float); }
	| STRINGVAR { $$ = SheepNode::CreateTypeRef(SheepReferenceType::String); }
	;
*/



