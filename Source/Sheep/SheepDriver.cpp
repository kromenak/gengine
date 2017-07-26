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
        
        yy::parser::semantic_type type;
        yy::parser::location_type location;
    
        int val;
        do
        {
            val = scanner->yylex(&type, &location);
            std::cout << val << std::endl;
        }
        while(val != 0);
    }
    catch(std::bad_alloc &ba)
    {
        std::cerr << "Failed to allocate scanner: (" << ba.what() << "), exiting!\n";
        exit(EXIT_FAILURE);
    }
    
    delete parser;
    try
    {
        parser = new yy::parser(*scanner, *this);
    }
    catch(std::bad_alloc &ba)
    {
        std::cerr << "Failed to allocate parser: (" << ba.what() << "), exiting!\n";
    }
}
