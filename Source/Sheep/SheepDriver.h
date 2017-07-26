//
//  SheepDriver.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/24/17.
//

#pragma once

#include <istream>

#include "SheepScanner.h"
#include "sheep.tab.hh"

namespace Sheep
{
    class Driver
    {
    public:
        Driver() = default;
        virtual ~Driver();
        
        void Parse(const char* filename);
        void Parse(std::istream& iss);
        
    private:
        void ParseHelper(std::istream& stream);
        
        Scanner* scanner = nullptr;
        yy::parser* parser = nullptr;
    };
}
