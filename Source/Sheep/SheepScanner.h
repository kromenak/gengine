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

namespace Sheep
{
    class Scanner : public yyFlexLexer
    {
    public:
        Scanner(std::istream *in) : yyFlexLexer(in)
        {
            loc = new yy::parser::location_type();
        }
        
        char* GetYYText() { return yytext; }
        
        //using FlexLexer::yylex;
        virtual int yylex(yy::parser::semantic_type* const lval,
                          yy::parser::location_type* location);
        // YY_DECL is defined in the Flex source file (Sheep.l)
        // The method body is created by Flex in lex.yy.cc.
        
    private:
        yy::parser::semantic_type *yylval = nullptr;
        yy::parser::location_type *loc = nullptr;
    };
}
