%{
#include <string>
#include <cstdlib>
#include "SheepScanner.h"
#undef YY_DECL
#define YY_DECL Sheep::Parser::symbol_type SheepScanner::yylex(SheepScanner& scanner, SheepCompiler& compiler, SheepScriptBuilder& builder)

using token = Sheep::Parser::token;
using parser = Sheep::Parser;

#define YY_USER_ACTION loc.step(); loc.columns(yyleng);

#define yyterminate() return parser::make_END(loc)
%}

%option yyclass="SheepScanner"
%option c++
%option noyywrap
%option yylineno
%option stack
%option never-interactive

%x COMMENT

%%

[ \t\n\r]+	{ }
"//".* { }

"code"		return parser::make_CODE(loc);
"symbols"	return parser::make_SYMBOLS(loc);

"int"		return parser::make_INTVAR(loc);
"float"		return parser::make_FLOATVAR(loc);
"string"	return parser::make_STRINGVAR(loc);

"return" 	return parser::make_RETURN(loc);
"wait"		return parser::make_WAIT(loc);
"if"		return parser::make_IF(loc);
"else"		return parser::make_ELSE(loc);
"goto"		return parser::make_GOTO(loc);

":"		return parser::make_COLON(loc);
";"		return parser::make_SEMICOLON(loc);
","		return parser::make_COMMA(loc);
"$"		return parser::make_DOLLAR(loc);
"("		return parser::make_OPENPAREN(loc);
")"		return parser::make_CLOSEPAREN(loc);
"{"		return parser::make_OPENBRACKET(loc);
"}"		return parser::make_CLOSEBRACKET(loc);

"\""	return parser::make_QUOTE(loc);

"=="	return parser::make_EQUAL(loc);
"!="	return parser::make_NOTEQUAL(loc);
"="		return parser::make_ASSIGN(loc);

"+"		return parser::make_PLUS(loc);
"-"		return parser::make_MINUS(loc);
"*"		return parser::make_MULTIPLY(loc);
"/"		return parser::make_DIVIDE(loc);

">="	return parser::make_GTE(loc);
"<="	return parser::make_LTE(loc);
">"		return parser::make_GT(loc);
"<"		return parser::make_LT(loc);

"!"		return parser::make_NOT(loc);
"&&"	return parser::make_AND(loc);
"||"	return parser::make_OR(loc);


-?([0-9])+\.([0-9])+	{ return parser::make_FLOAT(strtof(yytext, nullptr), loc); }
-?([0-9])+				{ return parser::make_INT(strtol(yytext, nullptr, 10), loc); }
\"([^\\\"]|\\.)*\"		{ return parser::make_STRING(yytext, loc); }
\|\<[^\>\|]*\>\|		{ return parser::make_STRING(yytext, loc); }

[a-zA-Z][a-zA-Z0-9_]*[$] { return parser::make_USERID(yytext, loc); }
[a-zA-Z][a-zA-Z0-9_]* { return parser::make_SYSID(yytext, loc); }

"/*"    BEGIN(COMMENT);

<COMMENT>"*/"	BEGIN(INITIAL);
<COMMENT>[^*\n]+ { } // eat comment in chunks
<COMMENT>"*"     { } // eat the lone star

.	printf("bad input character '%s' at line %d\n", yytext, yylineno); yyterminate();
