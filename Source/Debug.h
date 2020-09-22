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

class AABB;
class Mesh;
class Triangle;
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
    static void DrawLine(const Vector3& from, const Vector3& to, const Color32& color, float duration = 0.0f);
	
    static void DrawAxes(const Vector3& position, float duration = 0.0f);
    static void DrawAxes(const Matrix4& worldTransform, float duration = 0.0f);
	
    static void DrawRect(const Rect& rect, const Color32& color, float duration = 0.0f, const Matrix4* transformMatrix = nullptr);
    static void DrawAABB(const AABB& aabb, const Color32& color, float duration = 0.0f, const Matrix4* transformMatrix = nullptr);
    
    static void DrawTriangle(const Triangle& triangle, const Color32& color, float duration = 0.0f, const Matrix4* transformMatrix = nullptr);
	
	static void Update(float deltaTime);
	
	static void Render();
	
	static bool RenderActorTransformAxes() { return sRenderActorTransformAxes; }
	static bool RenderSubmeshLocalAxes() { return sRenderSubmeshLocalAxes; }
	static bool RenderRectTransformRects() { return sRenderRectTransformRects; }
	
private:
	static std::list<DrawCommand> sDrawCommands;
	
	// Debug settings, possible to toggle in-game.
	static bool sRenderActorTransformAxes;
	static bool sRenderSubmeshLocalAxes;
	static bool sRenderRectTransformRects;
};
