//
//  SheepDriver.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/24/17.
//
#include <fstream>
#include <sstream>
#include <cassert>

#include "SheepDriver.h"

Sheep::Driver::~Driver()
{
    delete mScanner;
    mScanner = nullptr;
    
    delete mParser;
    mParser = nullptr;
}

void Sheep::Driver::Parse(const char *filename)
{
    assert(filename != nullptr);    
    std::ifstream inFile(filename);
    Parse(inFile);
}

void Sheep::Driver::Parse(std::string sheep)
{
    std::stringstream stringStream(sheep);
    Parse(stringStream);
}

void Sheep::Driver::Parse(std::istream& stream)
{
    if(!stream.good() || stream.eof()) { return; }
    
    delete mScanner;
    try
    {
        mScanner = new Scanner(&stream);
        while(true)
        {
            Sheep::Parser::symbol_type yylookahead(mScanner->yylex(*mScanner, *this));
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
    }
    catch(std::bad_alloc &ba)
    {
        std::cerr << "Failed to allocate scanner: (" << ba.what() << "), exiting!\n";
        exit(EXIT_FAILURE);
    }
    
    delete mParser;
    try
    {
        mParser = new Sheep::Parser(*mScanner, *this);
        mParser->parse();
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
