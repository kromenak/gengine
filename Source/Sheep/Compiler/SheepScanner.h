//
//  SheepScanner.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/23/17.
//
#pragma once

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "sheep.tab.hh"
#include "location.hh"

class SheepScanner : public yyFlexLexer
{
public:
    SheepScanner(std::istream *in) : yyFlexLexer(in) { }

    char* GetYYText() { return yytext; }

    // YY_DECL is defined in the Flex source file (Sheep.l)
    // The method body is created by Flex in lex.yy.cc.
    Sheep::Parser::symbol_type yylex(SheepScanner& scanner, SheepCompiler& compiler, SheepScriptBuilder& builder);

private:
    Sheep::Parser::semantic_type *yylval = nullptr;
    Sheep::Parser::location_type loc;
};
