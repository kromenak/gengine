%{
#include "SheepScanner.h"
#undef YY_DECL
#define YY_DECL Sheep::Parser::symbol_type Sheep::Scanner::yylex(Sheep::Scanner& scanner, Sheep::Driver& driver)

using token = Sheep::Parser::token;

#define YY_USER_ACTION loc->step(); loc->columns(yyleng);
%}

%option yyclass="Sheep::Scanner"
%option c++
%option noyywrap
%option yylineno
%option stack
%option never-interactive

%s COMMENT

%%

[ \t\n\r]+	{ }
"//".* { }

<INITIAL>{
"code"		return token::CODE;
"symbols"	return token::SYMBOLS;

"int"		return token::INTVAR;
"float"		return token::FLOATVAR;
"string"	return token::STRINGVAR;

"return" 	return token::RETURN;
"wait"		return token::WAIT;
"if"		return token::IF;
"else"		return token::ELSE;
"goto"		return token::GOTO;

":"		return token::COLON;
";"		return token::SEMICOLON;
","		return token::COMMA;
"$"		return token::DOLLAR;
"("		return token::OPENPAREN;
")"		return token::CLOSEPAREN;
"{"		return token::OPENBRACKET;
"}"		return token::CLOSEBRACKET;

"\""	return token::QUOTE;

"=="	return token::EQUAL;
"!="	return token::NOTEQUAL;
"="		return token::ASSIGN;

"+"		return token::PLUS;
"-"		return token::MINUS;
"*"		return token::MULTIPLY;
"/"		return token::DIVIDE;

">="	return token::GTE;
"<="	return token::LTE;
">"		return token::GT;
"<"		return token::LT;

"!"		return token::NOT;
"&&"	return token::AND;
"||"	return token::OR;


-?([0-9])+\.([0-9])+	{ return token::FLOAT; }
-?([0-9])+				{ return token::INT; }
\"([^\\\"]|\\.)*\"		{ return token::STRING; }
\|\<[^\>\|]*\>\|		{ return token::STRING; }
}

[a-zA-Z][a-zA-Z0-9_]*[$] { return token::USERNAME; }
[a-zA-Z][a-zA-Z0-9_]* { return token::SYSNAME; }

<INITIAL>{
"/*"    BEGIN(COMMENT);
}

<COMMENT>{
"*/"	BEGIN(INITIAL);
[^*\n]+ // eat comment in chucks
"*"     // eat the lone star
}

.	printf("bad input character '%s' at line %d\n", yytext, yylineno);
