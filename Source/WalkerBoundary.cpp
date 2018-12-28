//
// WalkerBoundary.cpp
//
// Clark Kromenaker
//
#include "WalkerBoundary.h"

#include <queue>

#include "Texture.h"

bool WalkerBoundary::CanWalkTo(Vector3 position) const
{
	// If no texture...can walk anywhere?
	if(mTexture == nullptr) { return true; }
	
	// Convert the world position to a pixel position in the texture.
	// Add offset to world position to offset world position to walker texture position.
	Vector2 texturePos = WorldPosToTexturePos(position);
	
	// The color of the pixel at pos seems to indicate whether that spot is walkable.
	// White = totally OK to walk 				(255, 255, 255)
	// Blue = OK to walk						(0, 0, 255)
	// Green = sort of OK to walk 				(0, 255, 0)
	// Red = getting less OK to walk 			(255, 0, 0)
	// Yellow = sort of not OK to walk 			(255, 255, 0)
	// Magenta = really pushing it here 		(255, 0, 255)
	// Grey = pretty not OK to walk here 		(128, 128, 128)
	// Cyan = this is your last warning, buddy 	(0, 255, 255)
	// Black = totally not OK to walk 			(0, 0, 0)
	Color32 color = mTexture->GetPixelColor32(texturePos.GetX(), texturePos.GetY());
	
	// Basically, if the texture color is not black, you can walk there.
	return color != Color32::Black;
}

std::vector<Vector3> WalkerBoundary::FindPath(Vector3 from, Vector3 to) const
{
	// Convert world positions to texture positions.
	Vector2 start = WorldPosToTexturePos(from);
	Vector2 goal = WorldPosToTexturePos(to);
	
	//TODO: What if the positions are not walkable? Should we just fail? Should we find the nearest valid spot?
	
	// Let's try some A* to figure this out...
	std::vector<Vector3> path;
	
	return std::vector<Vector3>();
}

Vector2 WalkerBoundary::WorldPosToTexturePos(Vector3 worldPos) const
{
	// If no texture, the end result is going to be zero.
	if(mTexture == nullptr) { return Vector2::Zero; }
	
	// Convert the world position to a pixel position in the texture.
	// Add offset to world position to offset world position to walker texture position.
	Vector2 texturePos;
	texturePos.SetX(worldPos.GetX() + mOffset.GetX());
	texturePos.SetY(worldPos.GetZ() + mOffset.GetY());
	//std::cout << "Offset Pos: " << position << std::endl;
	
	// Divide position by walkable area size to get a normalized position within that area.
	// Hopefully 0-1, but could be outside those bounds. If so, not walkable.
	texturePos.SetX(texturePos.GetX() / mSize.GetX());
	texturePos.SetY(texturePos.GetY() / mSize.GetY());
	//std::cout << "Normalized Pos: " << position << std::endl;
	
	// Multiply by texture width/height to determine the pixel within the texture.
	texturePos.SetX(texturePos.GetX() * mTexture->GetWidth());
	texturePos.SetY(texturePos.GetY() * mTexture->GetHeight());
	//std::cout << "Pixel Pos: " << position << std::endl;
	
	// Need to flip Y because the calculated value is from lower-left of the walkable area.
	// But texture sample X/Y are from upper-left.
	texturePos.SetY(mTexture->GetHeight() - texturePos.GetY());
	return texturePos;
}
