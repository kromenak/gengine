//
// Clark Kromenaker
//
// Provides some functions for debugging and visualizing constructs in 3D space.
//
#pragma once
#include <list>

#include "Color32.h"
#include "FlagSet.h"
#include "Matrix4.h"
#include "Sphere.h"

class AABB;
class Mesh;
class Plane;
class Rect;
class Shader;
class Sphere;
class Triangle;
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
    static void Update(float deltaTime);
    static void Render();
    
    static void DrawLine(const Vector3& from, const Vector3& to, const Color32& color, float duration = 0.0f);
	
    static void DrawAxes(const Vector3& position, float duration = 0.0f);
    static void DrawAxes(const Matrix4& worldTransform, float duration = 0.0f);
	
    static void DrawRect(const Rect& rect, const Color32& color, float duration = 0.0f, const Matrix4* transformMatrix = nullptr);
    static void DrawRectXZ(const Rect& rect, float height, const Color32& color, float duration = 0.0f, const Matrix4* transformMatrix = nullptr);
    static void DrawScreenRect(const Rect& rect, const Color32& color);

    static void DrawAABB(const AABB& aabb, const Color32& color, float duration = 0.0f, const Matrix4* transformMatrix = nullptr);
    
    static void DrawPlane(const Plane& plane, const Vector3& point, const Color32& color, float duration = 0.0f, const Matrix4* transformMatrix = nullptr);
    
    static void DrawTriangle(const Triangle& triangle, const Color32& color, float duration = 0.0f, const Matrix4* transformMatrix = nullptr);

    static void DrawSphere(const Sphere& sphere, const Color32& color, float duration = 0.0f, const Matrix4* transformMatrix = nullptr);
    static void DrawSphere(const Vector3& position, float radius, const Color32& color, float duration = 0.0f, const Matrix4* transformMatrix = nullptr);

    // Flags
    static bool GetFlag(const std::string& flagName) { return sDebugFlags.Get(flagName); }
    static void SetFlag(const std::string& flagName) { sDebugFlags.Set(flagName); }
    static void ClearFlag(const std::string& flagName) { sDebugFlags.Clear(flagName); }
    static void ToggleFlag(const std::string& flagName) { sDebugFlags.Toggle(flagName); }
    static void DumpFlags() { sDebugFlags.Dump("debug"); }

	static bool RenderActorTransformAxes() { return sRenderActorTransformAxes; }
	static bool RenderSubmeshLocalAxes() { return sRenderSubmeshLocalAxes; }
	static bool RenderRectTransformRects() { return sRenderRectTransformRects; }
    static bool RenderAABBs() { return sDebugFlags.Get("ShowBoundingBoxes"); }
	
private:
    // Draw commands & shader.
	static std::list<DrawCommand> sDrawCommands;
    static Shader* sDrawShader;

    // Debug flags.
    static FlagSet sDebugFlags;
	
	// Debug settings, possible to toggle in-game.
	static bool sRenderActorTransformAxes;
	static bool sRenderSubmeshLocalAxes;
	static bool sRenderRectTransformRects;
};
