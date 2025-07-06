//
// Clark Kromenaker
//
// SysFuncs (or System Functions) provide a way for SheepScript to interact with the game engine.
// Any SysFunc can be called from SheepScript to achieve a variety of effects.
//
// The code here deals with the internal mechanisms of cataloguing and storing SysFuncs.
// The actual various SysFuncs themselves are implemented in the Sheep API.
//
#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <unordered_map>

#include "Value.h"

// Bare minimum data to uniquely identify a SysFunc signature in a SheepScript.
// Called an Import b/c we are sort of "importing" the function for use in a SheepScript.
struct SysFuncImport
{
    // Name of the function.
    std::string name;

    // Return type (0 = void, 1 = int, 2 = float, 3 = string).
    char returnType;

    // Argument count (size of vector) and types (same as return value).
    std::vector<char> argumentTypes;
};

// "Full" info about a SysFunc.
// Contains extra metadata that doesn't need to be stored in a compiled SheepScript, but is useful at runtime.
struct SysFunc : public SysFuncImport
{
    // If true, this function can be "waited" upon.
    // If false, it executes and returns immediately.
    bool waitable = false;

    // If true, this function can only work in dev builds.
    bool devOnly = false;

    // Text that's output to explain this function when using HelpCommand.
    //std::string helpText;

    // For help text output only, the names of the argument variables.
    // The length here should match the length of the argumentTypes array
    //std::vector<std::string> helpArgumentNames;

    // For example, HelpCommand("AddStreamContent") outputs this:
    /*
     ----- 'Dump' * 03/16/2019 * 11:39:21 -----
     ** [DEBUG] AddStreamContent(string streamName, string content) **
     Adds an additional content type to the stream. Possible values for the 'content' parameter are: 'begin' (report headers), 'content' (report content), 'end' (report footers), 'category' (the category of the report), 'date' (the date the report was made), 'time' (the time the report was made), 'debug' (file/line debug info), 'timeblock' (the current timeblock if there is one), and 'location' (the current game scene location if there is one).
    */
};

// Holds all SysFuncs created at runtime. There's only ever a single instance of this.
struct SysFuncs
{
    // Maps of SysFunc names to function pointers. One map per argument count.
    // This is how you actually call the functions.
    std::map<std::string, Value(*)(const Value&)> map0;
    std::map<std::string, Value(*)(const Value&)> map1;
    std::map<std::string, Value(*)(const Value&, const Value&)> map2;
    std::map<std::string, Value(*)(const Value&, const Value&, const Value&)> map3;
    std::map<std::string, Value(*)(const Value&, const Value&, const Value&, const Value&)> map4;
    std::map<std::string, Value(*)(const Value&, const Value&, const Value&, const Value&, const Value&)> map5;
    std::map<std::string, Value(*)(const Value&, const Value&, const Value&, const Value&, const Value&, const Value&)> map6;
    std::map<std::string, Value(*)(const Value&, const Value&, const Value&, const Value&, const Value&, const Value&, const Value&)> map7;
    std::map<std::string, Value(*)(const Value&, const Value&, const Value&, const Value&, const Value&, const Value&, const Value&, const Value&)> map8;

    // A big array of all our defined system functions.
    // This is populated at program start and then never changed.
    std::vector<SysFunc> sysFuncs;

    // Maps from system function name (or hash) to index in the sysFuncs vector.
    std::map<std::string, int> nameToSysFunc;
    std::unordered_map<size_t, int> hashToSysFunc;
};
SysFuncs& GetSysFuncs();

// Add/Retrieve SysFuncs.
void AddSysFunc(const std::string& name, char retType, std::initializer_list<char> argTypes, bool waitable, bool dev);
SysFunc* GetSysFunc(const std::string& name);
SysFunc* GetSysFunc(const SysFuncImport* sysImport);

// Call SysFuncs with various argument counts.
Value CallSysFunc(const std::string& name);
Value CallSysFunc(const std::string& name, const Value& x1);
Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2);
Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3);
Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3, const Value& x4);
Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3, const Value& x4, const Value& x5);
Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3, const Value& x4, const Value& x5, const Value& x6);

// Flags execution error in a SysFunc.
void ExecError();

// Gets the tag associated with the current Sheep thread.
const std::string& GetSheepTag();

// Adds a "wait" to the current Sheep thread.
// The current Sheep thread will suspend execution until the returned callback gets called.
std::function<void()> AddWait();

// These are used in the below macros to convert keywords into integers using ## macro operator.
#define void_TYPE 0
#define int_TYPE 1
#define float_TYPE 2
#define string_TYPE 3

// When registering functions, use these for Waitable and Dev/Release options to improve readability.
#define WAITABLE true
#define IMMEDIATE false

#define DEV_FUNC true
#define REL_FUNC false

// All Sheep functions must return a value (this is just b/c of C++ Value class usage).
// So, just do a special/dummy define for any "void" Sheep function to return.
#define shpvoid int

// Macros that register functions of various argument lengths with the system.
// Creates a function with same name as the actual function, but which uses generic "Value" args and return type.
// The generic function just calls the real function with correct argument types.

// Also registers an entry in the function map with a pointer to the "generic function".
// Flow is: Look Up in Map -> Calls Generic Function -> Calls Actual Function
#define RegFunc0(name, ret, waitable, dev)          					\
    Value name(const Value& x1) {                   					\
        return name();                              					\
    }                                               					\
    struct name##_ {                                					\
        name##_() {                                 					\
            GetSysFuncs().map0[#name]=&name;                            \
            AddSysFunc(#name, ret##_TYPE, { }, waitable, dev); 		    \
        }                                           					\
    } name##_instance

#define RegFunc1(name, ret, t1, waitable, dev)                     		\
    Value name(const Value& x1) {                   					\
        return name(x1.To<t1>());                   					\
    }                                               					\
    struct name##_ {                                					\
        name##_() {                                 					\
            GetSysFuncs().map1[#name]=&name;                      		\
            AddSysFunc(#name, ret##_TYPE, { t1##_TYPE }, waitable, dev);			\
        }                                           					\
    } name##_instance

#define RegFunc2(name, ret, t1, t2, waitable, dev)                      \
    Value name(const Value& x1, const Value& x2) {          			\
        return name(x1.To<t1>(), x2.To<t2>());              			\
    }                                                       			\
    struct name##_ {                                        			\
        name##_() {                                         			\
            GetSysFuncs().map2[#name]=&name;                            \
            AddSysFunc(#name, ret##_TYPE, { t1##_TYPE, t2##_TYPE }, waitable, dev); \
        }                                                   			\
    } name##_instance

#define RegFunc3(name, ret, t1, t2, t3, waitable, dev)                  \
    Value name(const Value& x1, const Value& x2, const Value& x3) {     \
        return name(x1.To<t1>(), x2.To<t2>(), x3.To<t3>());             \
    }                                                       			\
    struct name##_ {                                        			\
        name##_() {                                         			\
            GetSysFuncs().map3[#name]=&name;                            \
            AddSysFunc(#name, ret##_TYPE, { t1##_TYPE, t2##_TYPE, t3##_TYPE }, waitable, dev); \
        }                                                   			\
    } name##_instance

#define RegFunc4(name, ret, t1, t2, t3, t4, waitable, dev)              \
    Value name(const Value& x1, const Value& x2, const Value& x3, const Value& x4) { \
        return name(x1.To<t1>(), x2.To<t2>(), x3.To<t3>(), x4.To<t4>()); \
    }                                                       			\
    struct name##_ {                                        			\
        name##_() {                                         			\
            GetSysFuncs().map4[#name]=&name;                            \
            AddSysFunc(#name, ret##_TYPE, { t1##_TYPE, t2##_TYPE, t3##_TYPE, t4##_TYPE }, waitable, dev); \
        }                                                   			\
    } name##_instance

#define RegFunc5(name, ret, t1, t2, t3, t4, t5, waitable, dev)          \
    Value name(const Value& x1, const Value& x2, const Value& x3, const Value& x4, const Value& x5) { \
        return name(x1.To<t1>(), x2.To<t2>(), x3.To<t3>(), x4.To<t4>(), x5.To<t5>()); \
    }                                                       			\
    struct name##_ {                                        			\
        name##_() {                                         			\
            GetSysFuncs().map5[#name]=&name;                            \
            AddSysFunc(#name, ret##_TYPE, { t1##_TYPE, t2##_TYPE, t3##_TYPE, t4##_TYPE, t5##_TYPE }, waitable, dev); \
        }                                                   			\
    } name##_instance
