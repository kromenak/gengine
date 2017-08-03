//
//  SheepDriver.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/24/17.
//

#include <fstream>
#include <cassert>

#include "SheepDriver.h"

Sheep::Driver::~Driver()
{
    delete scanner;
    scanner = nullptr;
    
    delete parser;
    parser = nullptr;
}

void Sheep::Driver::Parse(const char *filename)
{
    assert(filename != nullptr);
    
    std::ifstream inFile(filename);
    if(!inFile.good())
    {
        exit(EXIT_FAILURE);
    }
    
    ParseHelper(inFile);
}

void Sheep::Driver::Parse(std::istream& stream)
{
    if(!stream.good() && stream.eof())
    {
        return;
    }
    
    ParseHelper(stream);
}

void Sheep::Driver::ParseHelper(std::istream &stream)
{
    delete scanner;
    try
    {
        scanner = new Scanner(&stream);
        /*
        while(true)
        {
            Sheep::Parser::symbol_type yylookahead(scanner->yylex(*scanner, *this));
            if(yylookahead.token() == Sheep::Parser::token::END)
            {
                std::cout << "EOF" << std::endl;
                break;
            }
            else
            {
                std::cout << yylookahead.token() << std::endl;
            }
        }
        */
    }
    catch(std::bad_alloc &ba)
    {
        std::cerr << "Failed to allocate scanner: (" << ba.what() << "), exiting!\n";
        exit(EXIT_FAILURE);
    }
    
    delete parser;
    try
    {
        parser = new Sheep::Parser(*scanner, *this);
        parser->parse();
    }
    catch(std::bad_alloc &ba)
    {
        std::cerr << "Failed to allocate parser: (" << ba.what() << "), exiting!\n";
    }
}

void Sheep::Driver::error(const Sheep::location &l, const std::string &m)
{
    std::cerr << l << ": " << m << std::endl;
}

void Sheep::Driver::error(const std::string &m)
{
    std::cerr << m << std::endl;
}
