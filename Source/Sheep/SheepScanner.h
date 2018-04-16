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

#include "Sheep.tab.hh"
#include "location.hh"

namespace Sheep
{
    class Scanner : public yyFlexLexer
    {
    public:
        Scanner(std::istream *in) : yyFlexLexer(in)
        {
            //loc = new Sheep::Parser::location_type();
        }
        
        char* GetYYText() { return yytext; }
        
        //using FlexLexer::yylex;
        //virtual int yylex(Sheep::Parser::semantic_type* const lval,
        //                  Sheep::Parser::location_type* location);
        //virtual int yylex(Sheep::Scanner& scanner, Sheep::Driver& driver);
        Sheep::Parser::symbol_type yylex(Sheep::Scanner& scanner, Sheep::Driver& driver);
        // YY_DECL is defined in the Flex source file (Sheep.l)
        // The method body is created by Flex in lex.yy.cc.
        
    private:
        Sheep::Parser::semantic_type *yylval = nullptr;
        Sheep::Parser::location_type loc;
    };
}
