%{
// Generated code will use SheepScanner symbols (b/c of YY_DECL and using keywords).
#include "SheepScanner.h"

// SheepScanner is a class that inherits yyFlexLexer. See SheepScanner.h for this function declaration.
// This allows our compiler/manager classes to treat SheepScanner as a "facade" for the entire compiler process.
// It also allows us to pass in a bunch of extra stuff used during compilation (scanner, compiler, builder). 
#undef YY_DECL
#define YY_DECL Sheep::Parser::symbol_type SheepScanner::yylex(SheepScanner& scanner, SheepCompiler& compiler, SheepScriptBuilder& builder)

// Since we're using namespaces, alias to just token and parser for later code.
using token = Sheep::Parser::token;
using parser = Sheep::Parser;

// Row/column tracking for error reporting.
// This code is run every time yylex is called.
#define YY_USER_ACTION loc.step(); loc.columns(static_cast<int>(yyleng));

// If scanner terminates (end of code or end of file), always put an END statement in the bytecode.
#define yyterminate() return parser::make_END(loc)

// Since this is cross-platform (including Windows), we can't use unistd.h
#define YY_NO_UNISTD_H
%}

/* We're making a C++ scanner (aka it is contained within a class) */
%option c++

/* Since we're using C++ with a custom subclass, tell code generator to put classes in SheepScanner:: instead of yyFlexLexer:: */
%option yyclass="SheepScanner"

/* yywrap function returns 1 if there's no more input upon reaching EOF, 0 if there is more input at EOF. */
/* Since we always want to say "no more input" at EOF, we can just use this option to specify that. */
%option noyywrap

/* If specified, current line number of input is tracked in variable yylineno. */
%option yylineno

/* Specify that we want to use start condition stacks. Allows us to use different scopes for "normal code" and "comments" for example. */
%option stack

/* We don't need an interactive scanner/parser. We are using it from our own code. */
%option never-interactive

/* We don't care about case (code == CODE == CoDe) */
%option caseless

/* Define all "start conditions" used in the scanner here. Besides the implicit INITIAL condition, we only use COMMENT right now.*/
%x COMMENT

%%

[ \t\n\r]+	{ } 	/* Ignore 1 or more spaces, tabs, or newlines. */
"//".* { } 			/* Ignore any single-line comments. */

"symbols"	return parser::make_SYMBOLS(loc); /* Symbols section contains variable declarations. */
"code"		return parser::make_CODE(loc);    /* Code section contains functions and operations on variables. */

"int"		return parser::make_INTVAR(loc);	/* Sheep only allows three types of variables: int, float, and string. */
"float"		return parser::make_FLOATVAR(loc);
"string"	return parser::make_STRINGVAR(loc);

"return" 	return parser::make_RETURN(loc); /* Returns from a function before the end is reached. */
"wait"		return parser::make_WAIT(loc);   /* Sheep's async operation support - some commands can be "waited on" before continuing script execution many frames later. */
"if"		return parser::make_IF(loc);	 /* if/else are your standard logic control statements. Combine to "else if" as well. */
"else"		return parser::make_ELSE(loc);
"goto"		return parser::make_GOTO(loc);   /* Sheep supports labels and gotos...for those who like to live dangerously. */

":"		return parser::make_COLON(loc);		 /* Used for goto labels (e.g. MyLabel:) */
";"		return parser::make_SEMICOLON(loc);	 /* Each statement in Sheep is ended by a semicolon - similar to C-style languages. */
","		return parser::make_COMMA(loc);      /* Separates parameters in function parameter lists. */

"$"		return parser::make_DOLLAR(loc);	 /* Dollar symbol is used to identify user-specified symbols. All user-defined symbols must end with $. */

"("		return parser::make_OPENPAREN(loc);  /* Open/close parentheses are used for function calls and math operation precedence. */
")"		return parser::make_CLOSEPAREN(loc);

"{"		return parser::make_OPENBRACKET(loc); /* Open/close braces are used for section/function/logic scopes. */
"}"		return parser::make_CLOSEBRACKET(loc);

"\""	return parser::make_QUOTE(loc); 	/* Signifies start/end of a string literal. */

"=="	return parser::make_EQUAL(loc);		/* Equality and inequality comparisons. */
"!="	return parser::make_NOTEQUAL(loc);

"="		return parser::make_ASSIGN(loc); 	/* Assignment to a variable. */

"+"		return parser::make_PLUS(loc);		/* Basic math operators. Sheep doesn't support +=, -=, etc. */
"-"		return parser::make_MINUS(loc);
"*"		return parser::make_MULTIPLY(loc);
"/"		return parser::make_DIVIDE(loc);

">="	return parser::make_GTE(loc); 		/* Logical comparisons. */
"<="	return parser::make_LTE(loc);
">"		return parser::make_GT(loc);
"<"		return parser::make_LT(loc);

"!"		return parser::make_NOT(loc);		/* Logical operators. */
"&&"	return parser::make_AND(loc);
"||"	return parser::make_OR(loc);


-?([0-9])+\.([0-9])+	{ return parser::make_FLOAT(strtof(yytext, nullptr), loc); }	/* Float literals are identified by use of a decimal point (e.g. 4.5). Sheep DOES NOT use "f" suffix. */
-?([0-9])+				{ return parser::make_INT(static_cast<int>(strtol(yytext, nullptr, 10)), loc); }	/* Int literals are just whole numbers. */
\"([^\\\"]|\\.)*\"		{ return parser::make_STRING(yytext, loc); }	/* String literals have quotes around them. */
\|\<[^\>\|]*\>\|		{ return parser::make_STRING(yytext, loc); }	/* String literals with from |<This is a string>| - don't know if we need this? */

[a-zA-Z][a-zA-Z0-9_]*[$] { return parser::make_USERID(yytext, loc); } 	/* A user id is any non-keyword or non-literal string with a $ on the end. Used for user-defined functions and variables. */
[a-zA-Z][a-zA-Z0-9_]* { return parser::make_SYSID(yytext, loc); }		/* A system id is any non-keyworld or non-literal with no $ on the end. Used mostly for systen functions. */

"/*"    BEGIN(COMMENT); 	/*  Uses "start conditions" to capture block comments. */
<COMMENT>"*/"	BEGIN(INITIAL);
<COMMENT>[^*\n]+ { } // eat comment in chunks
<COMMENT>"*"     { } // eat the lone star

.	printf("bad input character '%s' at line %d\n", yytext, yylineno); yyterminate(); /* If we get to this point, we have some unexpected input. */
