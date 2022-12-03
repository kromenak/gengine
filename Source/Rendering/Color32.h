//
// Clark Kromenaker
//
// A 32-bit RGBA color (1 byte per component)
//
#pragma once
#include <iostream>

class Color32
{
public:
    static Color32 Clear;   // (0, 0, 0, 0)

	static Color32 Black; 	// (0, 0, 0, 255)
	static Color32 White; 	// (255, 255, 255, 255)
    static Color32 Gray;    // (127, 127, 127, 255)
	
	static Color32 Red;	  	// (255, 0, 0, 255)
	static Color32 Green; 	// (0, 255, 0, 255)
	static Color32 Blue;  	// (0, 0, 255, 255)
	
	static Color32 Cyan;	// (0, 255, 255, 255)
	static Color32 Magenta; // (255, 0, 255, 255)
	static Color32 Yellow;  // (255, 255, 0, 255)
    
    static Color32 Orange;  // (255, 165, 0, 255)
	
    Color32() = default;
	Color32(unsigned char r, unsigned char g, unsigned char b);
	Color32(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	
	Color32(int r, int g, int b);
	Color32(int r, int g, int b, int a);
	
	// Copy
	Color32(const Color32& other);
	
	// Equality
	bool operator==(const Color32& other) const;
	bool operator!=(const Color32& other) const;
	
	// Accessors
    //TODO: Get rid of SetRGBA/GetRGBA - direct access is fine.
	void SetR(unsigned char newR) { r = newR; }
	void SetG(unsigned char newG) { g = newG; }
	void SetB(unsigned char newB) { b = newB; }
	void SetA(unsigned char newA) { a = newA; }
	
	unsigned char GetR() const { return r; }
	unsigned char GetG() const { return g; }
	unsigned char GetB() const { return b; }
	unsigned char GetA() const { return a; }
	unsigned char& operator[](unsigned int i)       { return (&r)[i]; }
	unsigned char  operator[](unsigned int i) const { return (&r)[i]; }
	
	// Addition/Subtraction
	Color32 operator+(const Color32& other) const;
	Color32& operator+=(const Color32& other);
	
	Color32 operator-(const Color32& other) const;
	Color32& operator-=(const Color32& other);
	
	// Multiplication
	Color32 operator*(const Color32& other) const;
	Color32& operator*=(const Color32& other);

    // Lerp
    static Color32 Lerp(const Color32& from, const Color32& to, float t);
	
	// Color components, valued 0-255 each.
	// Order is important for memory layout!
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;
	unsigned char a = 255;
};

std::ostream& operator<<(std::ostream& os, const Color32& v);
