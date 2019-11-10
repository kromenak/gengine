//
// TimeblockTests.cpp
//
// Clark Kromenaker
//
// Tests for Timeblock class.
//
#include "catch.hh"
#include "Timeblock.h"

TEST_CASE("Timeblock 24-hour constructor works")
{
	// Day 1, 1AM
	Timeblock t1(1, 1);
	REQUIRE(t1.GetDay() == 1);
	REQUIRE(t1.GetHour24() == 1);
	REQUIRE(t1.GetHour12() == 1);
	REQUIRE(t1.IsAM());
	REQUIRE(t1.GetAMPMSuffix() == 'A');
	REQUIRE(t1.ToString() == "101A");
	
	// Day 2, 12AM
	Timeblock t2(2, 0);
	REQUIRE(t2.GetDay() == 2);
	REQUIRE(t2.GetHour24() == 0);
	REQUIRE(t2.GetHour12() == 12);
	REQUIRE(t2.IsAM());
	REQUIRE(t2.GetAMPMSuffix() == 'A');
	REQUIRE(t2.ToString() == "212A");
	
	// Day 3, 8PM
	Timeblock t3(3, 20);
	REQUIRE(t3.GetDay() == 3);
	REQUIRE(t3.GetHour24() == 20);
	REQUIRE(t3.GetHour12() == 8);
	REQUIRE(t3.IsPM());
	REQUIRE(t3.GetAMPMSuffix() == 'P');
	REQUIRE(t3.ToString() == "308P");
	
	// And one with bad data.
	// Should fix to Day 1, 11PM
	Timeblock t4(-10, 95);
	REQUIRE(t4.GetDay() == 1);
	REQUIRE(t4.GetHour24() == 23);
	REQUIRE(t4.GetHour12() == 11);
	REQUIRE(t4.IsPM());
	REQUIRE(t4.GetAMPMSuffix() == 'P');
	REQUIRE(t4.ToString() == "111P");
}

TEST_CASE("Timeblock 12-hour constructor works")
{
	// Day 6, 3PM
	Timeblock t1(6, 3, Timeblock::PM);
	REQUIRE(t1.GetDay() == 6);
	REQUIRE(t1.GetHour24() == 15);
	REQUIRE(t1.GetHour12() == 3);
	REQUIRE(t1.IsPM());
	REQUIRE(t1.GetAMPMSuffix() == 'P');
	REQUIRE(t1.ToString() == "603P");
	
	// Day 1, 6AM
	Timeblock t2(1, 6, Timeblock::AM);
	REQUIRE(t2.GetDay() == 1);
	REQUIRE(t2.GetHour24() == 6);
	REQUIRE(t2.GetHour12() == 6);
	REQUIRE(t2.IsAM());
	REQUIRE(t2.GetAMPMSuffix() == 'A');
	REQUIRE(t2.ToString() == "106A");
	
	// Day 9, 12PM
	Timeblock t3(9, 12, Timeblock::PM);
	REQUIRE(t3.GetDay() == 9);
	REQUIRE(t3.GetHour24() == 12);
	REQUIRE(t3.GetHour12() == 12);
	REQUIRE(t3.IsPM());
	REQUIRE(t3.GetAMPMSuffix() == 'P');
	REQUIRE(t3.ToString() == "912P");
	
	// And one with bad data.
	// Should fix to Day 1, 12AM
	Timeblock t4(-10, 95, Timeblock::AM);
	REQUIRE(t4.GetDay() == 1);
	REQUIRE(t4.GetHour24() == 0);
	REQUIRE(t4.GetHour12() == 12);
	REQUIRE(t4.IsAM());
	REQUIRE(t4.GetAMPMSuffix() == 'A');
	REQUIRE(t4.ToString() == "112A");
}

TEST_CASE("Timeblock string constructor works")
{
	// Day 1, 4AM
	Timeblock t1("104A");
	REQUIRE(t1.GetDay() == 1);
	REQUIRE(t1.GetHour24() == 4);
	REQUIRE(t1.GetHour12() == 4);
	REQUIRE(t1.IsAM());
	REQUIRE(t1.GetAMPMSuffix() == 'A');
	REQUIRE(t1.ToString() == "104A");
	
	// Day 4, 12PM
	Timeblock t2("412P");
	REQUIRE(t2.GetDay() == 4);
	REQUIRE(t2.GetHour24() == 12);
	REQUIRE(t2.GetHour12() == 12);
	REQUIRE(t2.IsPM());
	REQUIRE(t2.GetAMPMSuffix() == 'P');
	REQUIRE(t2.ToString() == "412P");
	
	// Day 2, 2PM
	Timeblock t3("202P");
	REQUIRE(t3.GetDay() == 2);
	REQUIRE(t3.GetHour24() == 14);
	REQUIRE(t3.GetHour12() == 2);
	REQUIRE(t3.IsPM());
	REQUIRE(t3.GetAMPMSuffix() == 'P');
	REQUIRE(t3.ToString() == "202P");
}
