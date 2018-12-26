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

extern Mesh* quad;

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
	mMaterial.SetWorldTransformMatrix(mRectTransform->GetLocalToWorldMatrix());
	mMaterial.Activate();
	
	// Render the mesh!
	mMesh->Render();
	
	Rect screenRect = mRectTransform->GetScreenRect();
	Vector3 from = Services::GetRenderer()->GetCamera()->ScreenToWorldPoint(screenRect.GetMin(), 0.0f);
	Vector3 to = Services::GetRenderer()->GetCamera()->ScreenToWorldPoint(screenRect.GetMax(), 0.0f);
	Debug::DrawLine(from, to, Color32::Blue);
}

void UILabel::SetText(std::string text)
{
	mText = text;
	mNeedMeshRegen = true;
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
	
	// If the text size is zero, we don't need to continue.
	if(mText.size() == 0) { return; }
	
	// 4 vertices per character; each vertex has position and UVs.
	int vertexCount = (int)mText.size() * 4;
	int vertexSize = (9 * sizeof(float));
	
	// Create mesh of desired size and usage.
	mMesh = new Mesh();
	Submesh* submesh = new Submesh(vertexCount, vertexSize, MeshUsage::Static);
	mMesh->AddSubmesh(submesh);
	
	// Create arrays to hold vertex positions and UVs.
	int positionSize = vertexCount * 3;
	int uvSize = vertexCount * 2;
	int colorSize = vertexCount * 4;
	int indexSize = (int)mText.size() * 6;
	
	float* positions = new float[positionSize];
	float* uvs = new float[uvSize];
	float* colors = new float[colorSize];
	unsigned short* indexes = new unsigned short[indexSize];
	
	// Calculate desired pixel height from points.
	//float desiredHeight = (mFontSizePt / 72.0f) * 96.0f;
	//float desiredWidth = (desiredPixelHeight / glyph.height) * glyph.width;
	
	// Iterate over each char of the text and generate the needed quad per letter.
	// Each char maps to a glyph from the font, which contains size and UV data.
	Color32 defaultFontColor = mFont->GetDefaultColor();
	float xPos = 0.0f;
	for(int i = 0; i < mText.size(); i++)
	{
		Glyph& glyph = mFont->GetGlyph(mText[i]);
		
		float leftX = xPos;
		float rightX = xPos + glyph.width;
		
		float topY = glyph.height;
		float bottomY = 0.0f;
		
		// Top-Left
		positions[i * 12] = leftX;
		positions[i * 12 + 1] = topY;
		positions[i * 12 + 2] = 0.0f;
		
		uvs[i * 8] = glyph.topLeftUvCoord.GetX();
		uvs[i * 8 + 1] = glyph.topLeftUvCoord.GetY();
		
		colors[i * 16] = (float)defaultFontColor.GetR() / 255.0f;
		colors[i * 16 + 1] = (float)defaultFontColor.GetG() / 255.0f;
		colors[i * 16 + 2] = (float)defaultFontColor.GetB() / 255.0f;
		colors[i * 16 + 3] = (float)defaultFontColor.GetA() / 255.0f;
		
		// Top-Right
		positions[i * 12 + 3] = rightX;
		positions[i * 12 + 4] = topY;
		positions[i * 12 + 5] = 0.0f;
		
		uvs[i * 8 + 2] = glyph.topRightUvCoord.GetX();
		uvs[i * 8 + 3] = glyph.topRightUvCoord.GetY();
		
		colors[i * 16 + 4] = (float)defaultFontColor.GetR() / 255.0f;
		colors[i * 16 + 5] = (float)defaultFontColor.GetG() / 255.0f;
		colors[i * 16 + 6] = (float)defaultFontColor.GetB() / 255.0f;
		colors[i * 16 + 7] = (float)defaultFontColor.GetA() / 255.0f;
		
		// Bottom-Left
		positions[i * 12 + 6] = leftX;
		positions[i * 12 + 7] = bottomY;
		positions[i * 12 + 8] = 0.0f;
		
		uvs[i * 8 + 4] = glyph.bottomLeftUvCoord.GetX();
		uvs[i * 8 + 5] = glyph.bottomRightUvCoord.GetY();
		
		colors[i * 16 + 8] = (float)defaultFontColor.GetR() / 255.0f;
		colors[i * 16 + 9] = (float)defaultFontColor.GetG() / 255.0f;
		colors[i * 16 + 10] = (float)defaultFontColor.GetB() / 255.0f;
		colors[i * 16 + 11] = (float)defaultFontColor.GetA() / 255.0f;
		
		// Bottom-Right
		positions[i * 12 + 9] = rightX;
		positions[i * 12 + 10] = bottomY;
		positions[i * 12 + 11] = 0.0f;
		
		uvs[i * 8 + 6] = glyph.bottomRightUvCoord.GetX();
		uvs[i * 8 + 7] = glyph.bottomRightUvCoord.GetY();
		
		colors[i * 16 + 12] = (float)defaultFontColor.GetR() / 255.0f;
		colors[i * 16 + 13] = (float)defaultFontColor.GetG() / 255.0f;
		colors[i * 16 + 14] = (float)defaultFontColor.GetB() / 255.0f;
		colors[i * 16 + 15] = (float)defaultFontColor.GetA() / 255.0f;
		
		// Indexes for this quad will be (0, 1, 2) & (2, 3, 4)
		indexes[i * 6] = i * 4;
		indexes[i * 6 + 1] = i * 4 + 1;
		indexes[i * 6 + 2] = i * 4 + 2;
		indexes[i * 6 + 3] = i * 4 + 1;
		indexes[i * 6 + 4] = i * 4 + 2;
		indexes[i * 6 + 5] = i * 4 + 3;
		
		xPos += glyph.width;
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
