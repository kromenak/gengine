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
#include "sheep.tab.hh"

namespace Sheep
{
    class Driver
    {
    public:
        Driver() = default;
        virtual ~Driver();
        
        void Parse(const char* filename);
        void Parse(std::string sheep);
        void Parse(std::istream& iss);
        
        // Bison seems to require that these be lowercase.
        void error(const Sheep::location& l, const std::string& m);
        void error(const std::string& m);
        
        std::map<std::string, int> intVariables;
        std::map<std::string, float> floatVariables;
        std::map<std::string, std::string> stringVariables;
        
    private:
        Scanner* mScanner = nullptr;
        Parser* mParser = nullptr;
    };
}
