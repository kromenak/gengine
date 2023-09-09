#include "UILabel.h"

#include "Actor.h"
#include "Debug.h"
#include "Camera.h"
#include "Font.h"
#include "Mesh.h"
#include "StringUtil.h"
#include "TextLayout.h"
#include "Texture.h"

TYPE_DEF_CHILD(UIWidget, UILabel);

UILabel::UILabel(Actor* owner) : UIWidget(owner)
{
	
}

UILabel::~UILabel()
{
    delete mMesh;
}

void UILabel::Render()
{
	if(!IsActiveAndEnabled()) { return; }
	
	// Generate the mesh, if needed.
    GenerateMesh();
	
	// If mesh is still null for some reason, we can't render.
	if(mMesh == nullptr) { return; }
	
	// Activate material.
	mMaterial.Activate(GetRectTransform()->GetLocalToWorldMatrix());
	
	// Render the mesh!
	mMesh->Render();
    //Debug::DrawScreenRect(GetRectTransform()->GetWorldRect(), Color32::Magenta);
}

void UILabel::SetFont(Font* font)
{
	mFont = font;
	if(mFont != nullptr)
	{
        // Assign shader from font.
        mMaterial.SetShader(mFont->GetShader());
        
        // Use font texture.
        mMaterial.SetDiffuseTexture(font->GetTexture());

        // Set color and replace colors.
        mMaterial.SetColor(font->GetColor());
        mMaterial.SetColor("uReplaceColor", font->GetReplaceColor());
	}
    else
    {
        // Null out references if no font is provided.
        mMaterial.SetShader(nullptr);
        mMaterial.SetDiffuseTexture(nullptr);
    }
    
    // Mark label dirty. Changing font may mean our mesh needs to be updated.
	SetDirty();
}

void UILabel::SetColor(const Color32& color)
{
    mColor = color;
    mMaterial.SetColor(color);
}

void UILabel::SetText(const std::string& text)
{
	if(text != mText)
	{
		mText = text;
		SetDirty();
	}
}

int UILabel::GetLineCount()
{
    // Before the line count can be known, an up-to-date mesh is needed.
    // This causes the text layout to be calculated.
    GenerateMesh();

    // Return line count from text layout.
    return mTextLayout.GetLineCount();
}

float UILabel::GetTextWidth()
{
    // Before the text width can be known, an up-to-date mesh is needed.
    GenerateMesh();

    // Used generated mesh to find min/max x-positions of the glyphs.
    float smallestX = FLT_MAX;
    float largestX = FLT_MIN;
    for(auto& charInfo : mTextLayout.GetChars())
    {
        float minX = charInfo.pos.x;
        if(minX < smallestX)
        {
            smallestX = minX;
        }

        float maxX = charInfo.pos.x + charInfo.glyph.width;
        if(maxX > largestX)
        {
            largestX = maxX;
        }
    }

    //HACK: Generating the mesh above makes the label no longer "dirty".
    //HACK: However, it may be important for UI creation code to keep the label dirty after calling this (if changing transform properties).
    //HACK: The REAL problem is that changing a label's transform doesn't flag it as dirty, which might be good to fix!
    SetDirty();

    // The width is then the difference between the min/max x-positions.
    return Math::Abs(largestX - smallestX);
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
    // Don't need to generate mesh if we have one and not dirty.
    if(mMesh != nullptr && !mNeedMeshRegen)
    {
        return;
    }

	// Need font to generate mesh.
	if(mFont == nullptr) { return; }

	// If a previous mesh exists, get rid of it.
	if(mMesh != nullptr)
	{
		delete mMesh;
		mMesh = nullptr;
	}
	
	// Create new text layout object with desired settings.
    Rect rect = GetRectTransform()->GetRect();
	mTextLayout = TextLayout(rect, mFont,
						     mHorizontalAlignment, mVerticalAlignment,
						     mHorizontalOverflow, mVerticalOverflow);
	
	// Have this class (or subclass) populate text layout as needed.
	PopulateTextLayout(mTextLayout);
	
	// Do we have any chars to render? If not, early out.
	int charCount = mTextLayout.GetCharCount();
	if(charCount == 0) { return; }
	
	// Create mesh of desired size and usage.
	mMesh = new Mesh();
    
    // 4 vertices per character; each vertex has position and UVs.
    int vertexCount = (int)charCount * 4;
	
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
    Color32 fontColor = Color32::White;
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

        float topUVy = glyph.topLeftUvCoord.y;
        float botUVy = glyph.bottomLeftUvCoord.y;

        // If desired, we can "mask" the text mesh based on the size of the label's rect.
        // This is implemented by clamping the vertex positions and adjusting the UVs accordingly.
        //TODO: Masking may also be possible more generally using a Mask component and shader stencils...
        if(mMask)
        {
            if(topY > rect.GetMax().y)
            {
                float diff = rect.GetMax().y - topY;
                topY = rect.GetMax().y;

                diff /= mFont->GetTexture()->GetHeight();
                topUVy += Math::Abs(diff);
            }
            if(bottomY < rect.GetMin().y)
            {
                float diff = rect.GetMin().y - bottomY;
                bottomY = rect.GetMin().y;

                diff /= mFont->GetTexture()->GetHeight();
                botUVy -= Math::Abs(diff);
            }
            //TODO: left/right sides
        }
        
		// Top-Left
		positions[charIndex * 12] = leftX;
		positions[charIndex * 12 + 1] = topY;
		positions[charIndex * 12 + 2] = 0.0f;
		
		uvs[charIndex * 8] = glyph.topLeftUvCoord.x;
		uvs[charIndex * 8 + 1] = topUVy;
		
		colors[charIndex * 16] = colorR;
		colors[charIndex * 16 + 1] = colorG;
		colors[charIndex * 16 + 2] = colorB;
		colors[charIndex * 16 + 3] = colorA;
		
		// Top-Right
		positions[charIndex * 12 + 3] = rightX;
		positions[charIndex * 12 + 4] = topY;
		positions[charIndex * 12 + 5] = 0.0f;
		
		uvs[charIndex * 8 + 2] = glyph.topRightUvCoord.x;
		uvs[charIndex * 8 + 3] = topUVy;
		
		colors[charIndex * 16 + 4] = colorR;
		colors[charIndex * 16 + 5] = colorG;
		colors[charIndex * 16 + 6] = colorB;
		colors[charIndex * 16 + 7] = colorA;
		
		// Bottom-Left
		positions[charIndex * 12 + 6] = leftX;
		positions[charIndex * 12 + 7] = bottomY;
		positions[charIndex * 12 + 8] = 0.0f;
		
		uvs[charIndex * 8 + 4] = glyph.bottomLeftUvCoord.x;
		uvs[charIndex * 8 + 5] = botUVy;
		
		colors[charIndex * 16 + 8] = colorR;
		colors[charIndex * 16 + 9] = colorG;
		colors[charIndex * 16 + 10] = colorB;
		colors[charIndex * 16 + 11] = colorA;
		
		// Bottom-Right
		positions[charIndex * 12 + 9] = rightX;
		positions[charIndex * 12 + 10] = bottomY;
		positions[charIndex * 12 + 11] = 0.0f;
		
		uvs[charIndex * 8 + 6] = glyph.bottomRightUvCoord.x;
		uvs[charIndex * 8 + 7] = botUVy;
		
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
	
    MeshDefinition meshDefinition(MeshUsage::Static, vertexCount);
    meshDefinition.SetVertexLayout(VertexLayout::Packed);

    meshDefinition.AddVertexData(VertexAttribute::Position, positions);
    meshDefinition.AddVertexData(VertexAttribute::Color, colors);
    meshDefinition.AddVertexData(VertexAttribute::UV1, uvs);

    meshDefinition.SetIndexData(indexSize, indexes);
    
	// This mesh should render as a "triangle strip".
	// Vtx 1,2,3 is one triangle. Vtx 2,3,4 is the next triangle.
    Submesh* submesh = mMesh->AddSubmesh(meshDefinition);
	submesh->SetRenderMode(RenderMode::Triangles);

    // Mesh has been generated.
    mNeedMeshRegen = false;
}
