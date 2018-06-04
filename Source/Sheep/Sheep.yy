/* This code is added to the top of sheep.tab.cc */
%code top {
	#include <cstdlib>
	#include <string>
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
	class SheepScanner;
	class SheepCompiler;
	#include "SheepScriptBuilder.h"
	#include "SheepVM.h"
}

%param { SheepScanner& scanner }
%param { SheepCompiler& compiler }
%param { SheepScriptBuilder& builder }

/* This code is added to the top of the .cc file for the parser. */
%code
{
	#include "SheepCompiler.h"

	// When requesting yylex, redirect to scanner.yylex.
	#undef yylex
	#define yylex scanner.yylex

	// The C++ yytext is const, but we need to modify to remove
	// quotes in some cases. So, this is a bit of a hack.
	#undef yytext
	#define yytext scanner.GetYYText()

	void Sheep::Parser::error(const location_type& loc, const std::string& msg)
	{
		compiler.error(loc, msg);
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

%type <SheepValue> expr
%type <SheepValue> sysfunc_call

/*
%type <SheepValue> num_expr
%type <SheepValue> int_expr
%type <SheepValue> float_expr
*/

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
script: %empty 								{ }	/* just an empty file :( */
	| symbols_section						{ } /* symbols { } */
	| code_section 							{ } /* code { } */
	| symbols_section code_section 			{ } /* symbols { } code { } */
	| OPENBRACKET statements CLOSEBRACKET   { } /* supports a sheep "snippet" or anonymous function, as used in NVC assets */
	;

/* symbols starts with "symbols {" and ends with "}"
   between brackets is zero or more symbols */
symbols_section: SYMBOLS OPENBRACKET symbol_decls CLOSEBRACKET { }

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
code_section: CODE OPENBRACKET functions CLOSEBRACKET { }
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
statement: USERID ASSIGN expr SEMICOLON 				{ builder.Store($1); } /* myInt$ = 2 + 8; */
	| expr 												{ } /* sys func call or pointless expression like (2+4); */
	| RETURN SEMICOLON 									{ builder.ReturnV(); } /* return; */
	| BREAKPOINT SEMICOLON 								{ builder.Breakpoint(); } /* breakpoint; */
	| SITNSPIN SEMICOLON 								{ builder.SitnSpin(); } /* sitnspin; */
	| GOTO USERID SEMICOLON 							{ builder.BranchGoto($2); } /* goto blah$; */
	| USERID COLON										{ builder.AddGoto($1); } /* blah$: */
	| WAIT SEMICOLON 									{ builder.BeginWait(); builder.EndWait(); } /* wait; */
	| WAIT { builder.BeginWait(); } sysfunc_call 		{ builder.EndWait(); } /* wait WalkTo("Gab", "FR_25"); */
	| WAIT { builder.BeginWait(); } OPENBRACKET statements CLOSEBRACKET { builder.EndWait(); } /* wait { // stuff } */
	| statements_block 									/* { // stuff } */
	| SEMICOLON 										{ } /* ; */
	| if_else_block										/* if(blah) { } else if(foo) { } else { } */
	;

/* just an indented section with MORE statements */
statements_block: OPENBRACKET statements CLOSEBRACKET
	;

/* an expression is any constant value, math operation, system function call */ 
expr: sysfunc_call 						{ $$ = $1; } 												/* PrintString("Ahhh") */
	| USERID 							{ auto type = builder.Load($1); $$ = SheepValue(type); } 	/* foo$ */
	| INT								{ builder.PushI($1); $$ = SheepValue($1); }
	| FLOAT 							{ builder.PushF($1); $$ = SheepValue($1); }
	/* | int_expr 							{ } */
	/* | float_expr 						{ } */
	| STRING 							{ builder.AddStringConst($1); builder.PushS($1); $$ = SheepValue(""); }

	| expr PLUS expr					{ auto type = builder.Add($1, $3); $$ = SheepValue(type); }
	| expr MINUS expr					{ auto type = builder.Subtract($1, $3); $$ = SheepValue(type); }
	| expr MULTIPLY expr				{ auto type = builder.Multiply($1, $3); $$ = SheepValue(type); }
	| expr DIVIDE expr					{ auto type = builder.Divide($1, $3); $$ = SheepValue(type); }
	| expr MOD expr						{ builder.Modulo($1, $3); $$ = SheepValue(SheepValueType::Int); } 
	| NEGATE expr 						{ builder.Negate($2); }

	| expr LT expr 						{ auto type = builder.IsLess($1, $3); $$ = SheepValue(type); }
	| expr GT expr						{ auto type = builder.IsGreater($1, $3); $$ = SheepValue(type); }
	| expr LTE expr 					{ auto type = builder.IsLessEqual($1, $3); $$ = SheepValue(type); }
	| expr GTE expr 					{ auto type = builder.IsGreaterEqual($1, $3); $$ = SheepValue(type); }
	| expr EQUAL expr 					{ auto type = builder.IsEqual($1, $3); $$ = SheepValue(type); }
	| expr NOTEQUAL expr 				{ auto type = builder.IsNotEqual($1, $3); $$ = SheepValue(type); }
	| expr OR expr 						{ }
	| expr AND expr 					{ }
	| NOT expr 							{ }

	| OPENPAREN expr CLOSEPAREN 		{ $$ = $2; } /* (10 + 12) */
	;

/* a sysfunc call is like GetEgoName(); or PrintString("Ahh");
   it can have zero or more arguments */
sysfunc_call: SYSID OPENPAREN sysfunc_call_args CLOSEPAREN { auto type = builder.CallSysFunction($1); $$ = SheepValue(type); }
	;

/* sysfunc call args are either empty, or a list or args */
sysfunc_call_args: %empty				{ } /* No arg */
	| expr 								{ } /* 5 + 2 */
	| sysfunc_call_args COMMA expr 		{ } /* 5 + 2, "Gab", GetCameraFov() */
	;

/*
num_expr: INT 							{ builder.PushI($1); $$ = SheepValue($1); }
	| FLOAT 							{ builder.PushF($1); $$ = SheepValue($1); }
	| num_expr PLUS num_expr			{ }
	| num_expr MINUS num_expr			{ }
	| num_expr MULTIPLY num_expr		{ }
	| num_expr DIVIDE num_expr			{ }
	| num_expr MOD num_expr				{ } 
	| NEGATE num_expr 					{ }

	| num_expr LT num_expr 				{ }
	| num_expr GT num_expr				{ }
	| num_expr LTE num_expr 			{ }
	| num_expr GTE num_expr 			{ }
	| num_expr EQUAL num_expr 			{ }
	| num_expr NOTEQUAL num_expr 		{ }
	| num_expr OR num_expr 				{ }
	| num_expr AND num_expr 			{ }
	| NOT num_expr 						{ }
	;

int_expr: INT 							{ builder.PushI($1); $$ = SheepValue($1); }
	| int_expr PLUS int_expr			{ builder.AddI(); }
	| int_expr MINUS int_expr			{ builder.SubtractI(); }
	| int_expr MULTIPLY int_expr 		{ builder.MultiplyI(); }
	| int_expr DIVIDE int_expr 			{ builder.DivideI(); }
	| int_expr MOD int_expr 			{ builder.Modulo(); }
	| NEGATE int_expr					{ builder.NegateI(); }

	| int_expr LT int_expr 				{ builder.IsLessI(); }
	| int_expr GT int_expr 				{ builder.IsGreaterI(); }
	| int_expr LTE int_expr 			{ builder.IsLessEqualI(); }
	| int_expr GTE int_expr 			{ builder.IsGreaterEqualI(); }
	| int_expr NOTEQUAL int_expr      	{ builder.IsNotEqualI(); }
	| int_expr EQUAL int_expr			{ builder.IsEqualI(); }
	| int_expr OR int_expr				{ builder.Or(); }
	| int_expr AND int_expr  			{ builder.And(); }
	| NOT int_expr  					{ builder.Not(); }
	;

float_expr: FLOAT 						{ builder.PushF($1); $$ = SheepValue($1); }
	| float_expr PLUS float_expr       	{ builder.AddF(); }
	| float_expr MINUS float_expr		{ builder.SubtractF(); }
	| float_expr MULTIPLY float_expr	{ builder.MultiplyF(); }
	| float_expr DIVIDE float_expr		{ builder.DivideF(); }
	| NEGATE float_expr 				{ builder.NegateF(); }

	| float_expr LT float_expr			{ builder.IsLessF(); }
	| float_expr GT float_expr			{ builder.IsGreaterF(); }
	| float_expr LTE float_expr			{ builder.IsLessEqualF(); }
	| float_expr GTE float_expr			{ builder.IsGreaterEqualF(); }
	| float_expr NOTEQUAL float_expr	{ builder.IsNotEqualF(); }
	| float_expr EQUAL float_expr		{ builder.IsEqualF(); }
	| float_expr OR float_expr			{ builder.Or(); }
	| float_expr AND float_expr			{ builder.And(); }
	| NOT float_expr					{ builder.Not(); }
	;
*/

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

