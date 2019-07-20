//
// Debug.h
//
// Clark Kromenaker
//
// Provides some functions for debugging and visualizing constructs in 3D space.
//
#pragma once

#include <list>

#include "Color32.h"
#include "Matrix4.h"

class Mesh;
class Rect;
class Vector3;

struct DrawCommand
{
	// The mesh to draw.
	Mesh* mesh = nullptr;
	
	// Parameters for drawing.
	Matrix4 worldTransformMatrix;
	Color32 color = Color32::White;
	
	// Tracks how long the line remains visible.
	float timer = 0.0f;
};

class Debug
{
public:
	static void DrawLine(const Vector3& from, const Vector3& to, const Color32& color);
	static void DrawLine(const Vector3& from, const Vector3& to, const Color32& color, float duration);
	
	static void DrawAxes(const Vector3& position);
	static void DrawAxes(const Vector3& position, float duration);
	
	static void DrawAxes(const Matrix4& worldTransform);
	static void DrawAxes(const Matrix4& worldTransform, float duration);
	
	static void DrawRect(const Rect& rect, const Color32& color);
	
	static void Update(float deltaTime);
	
	static void Render();
	
	static bool RenderSubmeshLocalAxes() { return sRenderSubmeshLocalAxes; }
	
private:
	static std::list<DrawCommand> sDrawCommands;
	
	// Debug settings, possible to toggle in-game.
	static bool sRenderSubmeshLocalAxes;
};
