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
#include "TextLayout.h"

TYPE_DEF_CHILD(UIWidget, UILabel);

UILabel::UILabel(Actor* owner) : UIWidget(owner)
{
	
}

void UILabel::Render()
{
	if(!IsActiveAndEnabled()) { return; }
	
	// Generate the mesh, if needed.
	if(mMesh == nullptr || mNeedMeshRegen)
	{
		GenerateMesh();
		mNeedMeshRegen = false;
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
	SetDirty();
}

void UILabel::SetText(std::string text)
{
	if(text != mText)
	{
		mText = text;
		SetDirty();
	}
}

Vector2 UILabel::GetCharPos(int index) const
{
	const TextLayout::CharInfo* charInfo = mTextLayout.GetChar(index);
	if(charInfo != nullptr)
	{
		return charInfo->pos;
	}
	return Vector2::Zero;
}

void UILabel::PopulateTextLayout(TextLayout& textLayout)
{
	// Add all text to text layout to calculate glyph positions and such.
	textLayout.AddLine(mText);
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
	
	// Create new text layout object with desired settings.
	mTextLayout = TextLayout(mRectTransform->GetRect(), mFont,
						     mHorizontalAlignment, mVerticalAlignment,
						     mHorizontalOverflow, mVerticalOverflow);
	
	// Have this class (or subclass) populate text layout as needed.
	PopulateTextLayout(mTextLayout);
	
	// Do we have any chars to render? If not, early out.
	int charCount = mTextLayout.GetCharCount();
	if(charCount == 0) { return; }
	
	// 4 vertices per character; each vertex has position and UVs.
	int vertexCount = (int)charCount * 4;
	int vertexSize = (9 * sizeof(float));
	
	// Create mesh of desired size and usage.
	mMesh = new Mesh();
	Submesh* submesh = new Submesh(vertexCount, vertexSize, MeshUsage::Static);
	mMesh->AddSubmesh(submesh);
	
	// Create arrays to hold vertex positions and UVs.
	int positionSize = vertexCount * 3;
	int uvSize = vertexCount * 2;
	int colorSize = vertexCount * 4;
	int indexSize = (int)charCount * 6;
	
	float* positions = new float[positionSize];
	float* uvs = new float[uvSize];
	float* colors = new float[colorSize];
	unsigned short* indexes = new unsigned short[indexSize];
	
	// Need color to pass as vertex color for text mesh.
	Color32 fontColor = mFont->GetDefaultColor();
	float colorR = (float)fontColor.GetR() / 255.0f;
	float colorG = (float)fontColor.GetG() / 255.0f;
	float colorB = (float)fontColor.GetB() / 255.0f;
	float colorA = (float)fontColor.GetA() / 255.0f;
	
	int charIndex = 0;
	const std::vector<TextLayout::CharInfo>& charInfos = mTextLayout.GetChars();
	for(auto& charInfo : charInfos)
	{
		Glyph& glyph = charInfo.glyph;
		
		float leftX = charInfo.pos.x;
		float rightX = leftX + glyph.width;
		
		float bottomY = charInfo.pos.y;
		float topY = bottomY + glyph.height;
		
		// Top-Left
		positions[charIndex * 12] = leftX;
		positions[charIndex * 12 + 1] = topY;
		positions[charIndex * 12 + 2] = 0.0f;
		
		uvs[charIndex * 8] = glyph.topLeftUvCoord.x;
		uvs[charIndex * 8 + 1] = glyph.topLeftUvCoord.y;
		
		colors[charIndex * 16] = colorR;
		colors[charIndex * 16 + 1] = colorG;
		colors[charIndex * 16 + 2] = colorB;
		colors[charIndex * 16 + 3] = colorA;
		
		// Top-Right
		positions[charIndex * 12 + 3] = rightX;
		positions[charIndex * 12 + 4] = topY;
		positions[charIndex * 12 + 5] = 0.0f;
		
		uvs[charIndex * 8 + 2] = glyph.topRightUvCoord.x;
		uvs[charIndex * 8 + 3] = glyph.topRightUvCoord.y;
		
		colors[charIndex * 16 + 4] = colorR;
		colors[charIndex * 16 + 5] = colorG;
		colors[charIndex * 16 + 6] = colorB;
		colors[charIndex * 16 + 7] = colorA;
		
		// Bottom-Left
		positions[charIndex * 12 + 6] = leftX;
		positions[charIndex * 12 + 7] = bottomY;
		positions[charIndex * 12 + 8] = 0.0f;
		
		uvs[charIndex * 8 + 4] = glyph.bottomLeftUvCoord.x;
		uvs[charIndex * 8 + 5] = glyph.bottomRightUvCoord.y;
		
		colors[charIndex * 16 + 8] = colorR;
		colors[charIndex * 16 + 9] = colorG;
		colors[charIndex * 16 + 10] = colorB;
		colors[charIndex * 16 + 11] = colorA;
		
		// Bottom-Right
		positions[charIndex * 12 + 9] = rightX;
		positions[charIndex * 12 + 10] = bottomY;
		positions[charIndex * 12 + 11] = 0.0f;
		
		uvs[charIndex * 8 + 6] = glyph.bottomRightUvCoord.x;
		uvs[charIndex * 8 + 7] = glyph.bottomRightUvCoord.y;
		
		colors[charIndex * 16 + 12] = colorR;
		colors[charIndex * 16 + 13] = colorG;
		colors[charIndex * 16 + 14] = colorB;
		colors[charIndex * 16 + 15] = colorA;
		
		// Indexes for this quad will be (0, 1, 2) & (2, 3, 4)
		indexes[charIndex * 6] = charIndex * 4;
		indexes[charIndex * 6 + 1] = charIndex * 4 + 1;
		indexes[charIndex * 6 + 2] = charIndex * 4 + 2;
		indexes[charIndex * 6 + 3] = charIndex * 4 + 1;
		indexes[charIndex * 6 + 4] = charIndex * 4 + 2;
		indexes[charIndex * 6 + 5] = charIndex * 4 + 3;
		
		++charIndex;
	}
	
	// Save positions and UVs to mesh.
	submesh->SetPositions(positions);
	submesh->SetColors(colors);
	submesh->SetUV1(uvs);
	submesh->SetIndexes(indexes, indexSize);
	
	// This mesh should render as a "triangle strip".
	// Vtx 1,2,3 is one triangle. Vtx 2,3,4 is the next triangle.
	submesh->SetRenderMode(RenderMode::Triangles);
}
