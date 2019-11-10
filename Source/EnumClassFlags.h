//
//  EnumClassFlags.h
//  GEngine
//
//  Created by Clark Kromenaker on 11/9/19.
//
#pragma once

#define ENUM_CLASS_FLAGS(x) 												\
inline x operator|(x lhs, x rhs) 											\
{																			\
	using T = std::underlying_type<x>::type;								\
	return static_cast<x>(static_cast<T>(lhs) | static_cast<T>(rhs));		\
}																			\
																			\
inline x& operator|=(x& lhs, x rhs)											\
{																			\
	lhs = lhs | rhs;														\
	return lhs;																\
}																			\
																			\
inline x operator&(x lhs, x rhs)											\
{																			\
	using T = std::underlying_type<x>::type;								\
	return static_cast<x>(static_cast<T>(lhs) & static_cast<T>(rhs));		\
}																			\
																			\
inline x& operator&=(x& lhs, x rhs)											\
{																			\
	lhs = lhs & rhs;														\
	return lhs;																\
}																			\
																			\
inline x operator~(x rhs)													\
{																			\
	using T = std::underlying_type<x>::type;								\
	return static_cast<x>(~static_cast<T>(rhs));							\
}
