//
// UILabel.cpp
//
// Clark Kromenaker
//
#include "UILabel.h"

#include "Actor.h"
#include "Debug.h"
#include "Camera.h"
#include "Font.h"
#include "Mesh.h"
#include "StringUtil.h"

TYPE_DEF_CHILD(UIWidget, UILabel);

UILabel::UILabel(Actor* owner) : UIWidget(owner)
{
	
}

void UILabel::Render()
{
	// Generate the mesh, if needed.
	if(mMesh == nullptr || mNeedMeshRegen)
	{
		GenerateMesh();
	}
	
	// If mesh is still null for some reason, we can't render.
	if(mMesh == nullptr) { return; }
	
	// Activate material.
	mMaterial.SetShader(mFont->GetShader());
	mMaterial.Activate();
	mMaterial.SetWorldTransformMatrix(mRectTransform->GetLocalToWorldMatrix());
	
	// Render the mesh!
	mMesh->Render();
}



void UILabel::SetFont(Font* font)
{
	mFont = font;
	if(mFont == nullptr)
	{
		mMaterial.SetDiffuseTexture(nullptr);
	}
	else
	{
		mMaterial.SetDiffuseTexture(font->GetTexture());
	}
	mNeedMeshRegen = true;
}

void UILabel::SetText(std::string text)
{
	if(text != mText)
	{
		mText = text;
		mNeedMeshRegen = true;
	}
}

void UILabel::SetLineSpacing(int spacing)
{
	mLineSpacing = spacing;
	mNeedMeshRegen = true;
}

Vector2 UILabel::GetGlyphPos(int index) const
{
	if(index >= 0 && index < mGlyphPositions.size())
	{
		return mGlyphPositions[index];
	}
	return Vector2::Zero;
}

void UILabel::GenerateMesh()
{
	// Need font to generate mesh.
	if(mFont == nullptr) { return; }
	
	// If a previous mesh exists, get rid of it.
	if(mMesh != nullptr)
	{
		delete mMesh;
		mMesh = nullptr;
	}
	
	// Reset next glyph pos (in case we have zero glyphs).
	mGlyphPositions.clear();
	mNextGlyphPos = Vector2::Zero;
	
	// Split text into lines based on newline char.
	//TODO: May also need to split lines based on overflow settings! How would that work?
	std::vector<std::string> lines = StringUtil::Split(mText, '\n');
	
	// For each line, determine it's width ahead of time for proper alignment calculations.
	std::vector<int> lineWidths;
	int glyphCount = 0;
	for(std::string& line : lines)
	{
		int lineWidth = 0;
		for(int i = 0; i < line.size(); ++i)
		{
			Glyph& glyph = mFont->GetGlyph(line[i]);
			lineWidth += glyph.width;
			++glyphCount;
		}
		lineWidths.push_back(lineWidth);
	}
	int lineHeight = mFont->GetGlyphHeight();
	
	// If the text size is zero, we don't need to continue.
	if(glyphCount == 0) { return; }
	
	// 4 vertices per character; each vertex has position and UVs.
	int vertexCount = (int)glyphCount * 4;
	int vertexSize = (9 * sizeof(float));
	
	// Create mesh of desired size and usage.
	mMesh = new Mesh();
	Submesh* submesh = new Submesh(vertexCount, vertexSize, MeshUsage::Static);
	mMesh->AddSubmesh(submesh);
	
	// Create arrays to hold vertex positions and UVs.
	int positionSize = vertexCount * 3;
	int uvSize = vertexCount * 2;
	int colorSize = vertexCount * 4;
	int indexSize = (int)glyphCount * 6;
	
	float* positions = new float[positionSize];
	float* uvs = new float[uvSize];
	float* colors = new float[colorSize];
	unsigned short* indexes = new unsigned short[indexSize];
	
	// Calculate desired pixel height from points.
	//float desiredHeight = (mFontSizePt / 72.0f) * 96.0f;
	//float desiredWidth = (desiredPixelHeight / glyph.height) * glyph.width;
	
	// Need rect to determine x/y starting pos for each line.
	Rect rect = mRectTransform->GetRect();
	
	// Need color to pass as vertex color for text mesh.
	Color32 fontColor = mFont->GetDefaultColor();
	
	// Iterate each line and draw it.
	unsigned int charIndex = 0;
	for(int lIndex = 0; lIndex < lines.size(); ++lIndex)
	{
		std::string& line = lines[lIndex];
		
		float xPos = 0.0f;
		switch(mHorizontalAlignment)
		{
			case HorizontalAlignment::Left:
				xPos = rect.GetMin().GetX();
				break;
			case HorizontalAlignment::Right:
				xPos = rect.GetMax().GetX() - lineWidths[lIndex];
				break;
			//case HorizontalAlignment::Center:
			//	break;
		}
		
		float yPos = 0.0f;
		switch(mVerticalAlignment)
		{
			case VerticalAlignment::Bottom:
				yPos = rect.GetMin().GetY() + ((lineHeight + mLineSpacing) * (lines.size() - 1 - lIndex));
				break;
			case VerticalAlignment::Top:
				yPos = rect.GetMax().GetY() - ((lineHeight + mLineSpacing) * (lIndex + 1));
				break;
			//case VerticalAlignment::Center:
			//	break;
		}
		
		for(int i = 0; i < line.size(); ++i)
		{
			Glyph& glyph = mFont->GetGlyph(line[i]);
			
			float leftX = xPos;
			float rightX = xPos + glyph.width;
			
			float bottomY = yPos;
			float topY = yPos + glyph.height;
			
			//std::cout << line[i] << ", " << leftX << ", " << bottomY << std::endl;
			//std::cout << line[i] << ", " << rightX << ", " << topY << std::endl;
			
			// Top-Left
			positions[charIndex * 12] = leftX;
			positions[charIndex * 12 + 1] = topY;
			positions[charIndex * 12 + 2] = 0.0f;
			
			uvs[charIndex * 8] = glyph.topLeftUvCoord.GetX();
			uvs[charIndex * 8 + 1] = glyph.topLeftUvCoord.GetY();
			
			colors[charIndex * 16] = (float)fontColor.GetR() / 255.0f;
			colors[charIndex * 16 + 1] = (float)fontColor.GetG() / 255.0f;
			colors[charIndex * 16 + 2] = (float)fontColor.GetB() / 255.0f;
			colors[charIndex * 16 + 3] = (float)fontColor.GetA() / 255.0f;
			
			// Top-Right
			positions[charIndex * 12 + 3] = rightX;
			positions[charIndex * 12 + 4] = topY;
			positions[charIndex * 12 + 5] = 0.0f;
			
			uvs[charIndex * 8 + 2] = glyph.topRightUvCoord.GetX();
			uvs[charIndex * 8 + 3] = glyph.topRightUvCoord.GetY();
			
			colors[charIndex * 16 + 4] = (float)fontColor.GetR() / 255.0f;
			colors[charIndex * 16 + 5] = (float)fontColor.GetG() / 255.0f;
			colors[charIndex * 16 + 6] = (float)fontColor.GetB() / 255.0f;
			colors[charIndex * 16 + 7] = (float)fontColor.GetA() / 255.0f;
			
			// Bottom-Left
			positions[charIndex * 12 + 6] = leftX;
			positions[charIndex * 12 + 7] = bottomY;
			positions[charIndex * 12 + 8] = 0.0f;
			
			uvs[charIndex * 8 + 4] = glyph.bottomLeftUvCoord.GetX();
			uvs[charIndex * 8 + 5] = glyph.bottomRightUvCoord.GetY();
			
			colors[charIndex * 16 + 8] = (float)fontColor.GetR() / 255.0f;
			colors[charIndex * 16 + 9] = (float)fontColor.GetG() / 255.0f;
			colors[charIndex * 16 + 10] = (float)fontColor.GetB() / 255.0f;
			colors[charIndex * 16 + 11] = (float)fontColor.GetA() / 255.0f;
			
			// Bottom-Right
			positions[charIndex * 12 + 9] = rightX;
			positions[charIndex * 12 + 10] = bottomY;
			positions[charIndex * 12 + 11] = 0.0f;
			
			uvs[charIndex * 8 + 6] = glyph.bottomRightUvCoord.GetX();
			uvs[charIndex * 8 + 7] = glyph.bottomRightUvCoord.GetY();
			
			colors[charIndex * 16 + 12] = (float)fontColor.GetR() / 255.0f;
			colors[charIndex * 16 + 13] = (float)fontColor.GetG() / 255.0f;
			colors[charIndex * 16 + 14] = (float)fontColor.GetB() / 255.0f;
			colors[charIndex * 16 + 15] = (float)fontColor.GetA() / 255.0f;
			
			// Indexes for this quad will be (0, 1, 2) & (2, 3, 4)
			indexes[charIndex * 6] = charIndex * 4;
			indexes[charIndex * 6 + 1] = charIndex * 4 + 1;
			indexes[charIndex * 6 + 2] = charIndex * 4 + 2;
			indexes[charIndex * 6 + 3] = charIndex * 4 + 1;
			indexes[charIndex * 6 + 4] = charIndex * 4 + 2;
			indexes[charIndex * 6 + 5] = charIndex * 4 + 3;
			
			mGlyphPositions.emplace_back(xPos, yPos);
			xPos += glyph.width;
			++charIndex;
		}
		
		// Save whatever the next glyph pos would be.
		mGlyphPositions.emplace_back(xPos, yPos);
		mNextGlyphPos = Vector2(xPos, yPos);
	}
	
	// Save positions and UVs to mesh.
	submesh->SetPositions(positions);
	submesh->SetColors(colors);
	submesh->SetUV1(uvs);
	submesh->SetIndexes(indexes, indexSize);
	
	// This mesh should render as a "triangle strip".
	// Vtx 1,2,3 is one triangle. Vtx 2,3,4 is the next triangle.
	submesh->SetRenderMode(RenderMode::Triangles);
	
	// No longer need mesh regen.
	mNeedMeshRegen = false;
}
