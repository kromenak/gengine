//
// Debug.cpp
//
// Clark Kromenaker
//
#include "Debug.h"

#include "Material.h"
#include "Matrix4.h"
#include "Mesh.h"
#include "Rect.h"
#include "Services.h"
#include "Vector3.h"

extern Mesh* line;
extern Mesh* axes;

std::list<DrawCommand> Debug::sDrawCommands;

// Default debug settings.
bool Debug::sRenderActorTransformAxes = false;
bool Debug::sRenderSubmeshLocalAxes = false;
bool Debug::sRenderRectTransformRects = false;

void Debug::DrawLine(const Vector3& from, const Vector3& to, const Color32& color)
{
	DrawLine(from, to, color, 0.0f);
}

void Debug::DrawLine(const Vector3& from, const Vector3& to, const Color32& color, float duration)
{
	// The line "starts at" the from position.
	// Scale the line to the end point with appropriate scale matrix.
	Matrix4 translateMatrix = Matrix4::MakeTranslate(from);
	Matrix4 scaleMatrix = Matrix4::MakeScale(to - from);
	
	// Combine in order (Scale, Rotate, Translate) to generate world transform matrix.
	DrawCommand command;
	command.mesh = line;
	command.worldTransformMatrix = translateMatrix * scaleMatrix;
	command.color = color;
	command.timer = duration;
	sDrawCommands.push_back(command);
}

void Debug::DrawAxes(const Vector3& position)
{
	DrawAxes(position, 0.0f);
}

void Debug::DrawAxes(const Vector3& position, float duration)
{
	DrawAxes(Matrix4::MakeTranslate(position), duration);
}

void Debug::DrawAxes(const Matrix4& worldTransform)
{
	DrawAxes(worldTransform, 0.0f);
}

void Debug::DrawAxes(const Matrix4& worldTransform, float duration)
{
	DrawCommand command;
	command.mesh = axes;
	command.worldTransformMatrix = worldTransform;
	command.timer = duration;
	sDrawCommands.push_back(command);
}

void Debug::DrawRect(const Rect& rect, const Color32& color)
{
	Vector2 bottomLeft = rect.GetMin();
	Vector2 topRight = rect.GetMax();
	Vector2 topLeft = Vector2(bottomLeft.x, topRight.y);
	Vector2 bottomRight = Vector2(topRight.x, bottomLeft.y);
	
	DrawLine(bottomLeft, topLeft, color);
	DrawLine(topLeft, topRight, color);
	DrawLine(topRight, bottomRight, color);
	DrawLine(bottomRight, bottomLeft, color);
}

void Debug::Update(float deltaTime)
{
	// Decrement timers in all draw commands.
	for(auto& command : sDrawCommands)
	{
		command.timer -= deltaTime;
	}
	
	// Check for debug setting inputs.
	if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_F1))
	{
		sRenderActorTransformAxes = !sRenderActorTransformAxes;
	}
	if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_F2))
	{
		sRenderSubmeshLocalAxes = !sRenderSubmeshLocalAxes;
	}
	if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_F3))
	{
		sRenderRectTransformRects = !sRenderRectTransformRects;
	}
}

void Debug::Render()
{
	// We can just use any old material for now (uses default shader under the hood).
	Material material;
	
	// Iterate over all draw commands and render them.
	auto it = sDrawCommands.begin();
	while(it != sDrawCommands.end())
	{
		DrawCommand& command = *it;
		
		// Set color and world transform.
		material.SetColor(command.color);
        material.Activate(command.worldTransformMatrix);
		
		// Draw the mesh.
		if(command.mesh != nullptr)
		{
			command.mesh->Render();
		}
		
		// Erase from list if time is up.
		if(command.timer <= 0.0f)
		{
			it = sDrawCommands.erase(it);
		}
		else
		{
			it++;
		}
	}
}
