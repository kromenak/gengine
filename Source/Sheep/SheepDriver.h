//
//  SheepDriver.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/24/17.
//

#pragma once

#include <istream>
#include <string>
#include <map>

#include "SheepScanner.h"
#include "Sheep.tab.hh"

namespace Sheep
{
    class Driver
    {
    public:
        Driver() = default;
        virtual ~Driver();
        
        void Parse(const char* filename);
        void Parse(std::istream& iss);
        
        // Bison seems to require that these be lowercase.
        void error(const Sheep::location& l, const std::string& m);
        void error(const std::string& m);
        
    private:
        void ParseHelper(std::istream& stream);
        
        Scanner* scanner = nullptr;
        Parser* parser = nullptr;
    };
}
