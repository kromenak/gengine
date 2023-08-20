//
// Clark Kromenaker
//
// A value in the Sheep language. Sheep only supports int, float, and string types.
//
#pragma once
#include <string>

enum class SheepValueType
{
    Void,
    Int,
    Float,
    String
};

struct SheepValue
{
    SheepValueType type = SheepValueType::Int;
    union
    {
        int intValue;
        float floatValue;
        const char* stringValue;
    };
    
    SheepValue() { intValue = 0; }
    explicit SheepValue(SheepValueType t) { type = t; intValue = 0; }
    explicit SheepValue(int i) { type = SheepValueType::Int; intValue = i; }
    explicit SheepValue(float f) { type = SheepValueType::Float; floatValue = f; }
    explicit SheepValue(const char* s) { type = SheepValueType::String; stringValue = s; }
	~SheepValue() { }
	
	// Helpers for implicit conversions between Int/Float when needed.
	int GetInt() const
	{
		switch(type)
		{
		default:
        case SheepValueType::Void:
            printf("Warning: converting unexpected SheepValue type to int.\n");
            return intValue;
		case SheepValueType::Int:
			return intValue;
		case SheepValueType::Float:
			return (int)floatValue;
		case SheepValueType::String:
			//TODO: This will throw an exception if the string doesn't represent a number!
			return std::stoi(stringValue);
		}
	}
	
	float GetFloat() const
	{
		switch(type)
		{
		default:
        case SheepValueType::Void:
            printf("Warning: converting unexpected SheepValue type to float.\n");
            return floatValue;
		case SheepValueType::Float:
			return floatValue;
		case SheepValueType::Int:
			return (float)intValue;
		case SheepValueType::String:
			//TODO: This will throw an exception if the string doesn't represent a number!
			return std::stof(stringValue);
		}
	}
	
	std::string GetString() const
	{
		switch(type)
		{
		default:
        case SheepValueType::Void:
            printf("Warning: converting unexpected SheepValue type to string.\n");
            return stringValue != nullptr ? std::string(stringValue) : std::string();
		case SheepValueType::String:
            return stringValue != nullptr ? std::string(stringValue) : std::string();
		case SheepValueType::Float:
			return std::to_string(floatValue);
		case SheepValueType::Int:
			return std::to_string(intValue);
		}
	}
	
	std::string GetTypeString() const
	{
		switch(type)
		{
        default:
		case SheepValueType::Void:
			return "void";
		case SheepValueType::String:
			return "string";
		case SheepValueType::Float:
			return "float";
		case SheepValueType::Int:
			return "int";
		}
	}
};
