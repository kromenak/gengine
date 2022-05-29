//
// Clark Kromenaker
//
// Sheep SysFuncs related to manipulating location and time.
//
#pragma once
#include "SheepSysFunc.h"

// LOCATION
int GetEgoCurrentLocationCount();
int GetEgoLocationCount(const std::string& locationName);
shpvoid SetEgoLocationCount(const std::string& locationName, int count); // DEV
int WasEgoEverInLocation(const std::string& locationName);

int IsActorAtLocation(const std::string& actorName, const std::string& locationName);
int IsActorOffstage(const std::string& actorName);

shpvoid SetActorLocation(const std::string& actorName, const std::string& locationName);
shpvoid SetActorOffstage(const std::string& actorName);

int IsCurrentLocation(const std::string& location);
int WasLastLocation(const std::string& location);
shpvoid SetLocation(const std::string& location); // WAIT

shpvoid DumpLocations(); // DEV

// TIME
int IsCurrentTime(const std::string& timeblock);
int WasLastTime(const std::string& timeblock);
shpvoid SetTime(const std::string& timeblock); // WAIT

shpvoid DumpTimes(); // DEV

// LOCATION & TIME
shpvoid SetLocationTime(const std::string& location, const std::string& timeblock); // WAIT
shpvoid Warp(const std::string& locationAndTime); // WAIT
