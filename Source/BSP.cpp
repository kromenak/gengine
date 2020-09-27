//
// BSP.cpp
//
// Clark Kromenaker
//
#include "BSP.h"

#include <iostream>

#include "BinaryReader.h"
#include "BSPActor.h"
#include "Debug.h"
#include "Services.h"
#include "StringUtil.h"
#include "Vector2.h"
#include "Vector3.h"

BSP::BSP(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
    
    // Load shader and map lightmap texture unit (remember, must activate before setting texture unit).
    Shader* lightmapShader = Services::GetAssets()->LoadShader("3D-Lightmap");
    lightmapShader->Activate();
    lightmapShader->SetUniformInt("uLightmap", 1);
    
    // Use lightmap shader for material.
    mMaterial.SetShader(lightmapShader);
}

bool BSP::RaycastNearest(const Ray& ray, RaycastHit& outHitInfo)
{
	// Values for tracking closest found hit.
    outHitInfo.t = FLT_MAX;
    std::string* closest = nullptr;
	
	// Iterate through all polygons in the BSP and see if the ray intersects
	// with any triangles inside the polygon. Triangles within the BSP are made
	// up of "triangle fans", so the first vertex in a polygon is shared by all triangles.
    for(auto& polygon : mPolygons)
    {
        BSPSurface& surface = mSurfaces[polygon.surfaceIndex];
        if(!surface.interactive) { continue; }
		
        Vector3 p0 = mVertices[mVertexIndices[polygon.vertexIndexOffset]];
        for(int i = 1; i < polygon.vertexIndexCount - 1; i++)
        {
            Vector3 p1 = mVertices[mVertexIndices[polygon.vertexIndexOffset + i]];
            Vector3 p2 = mVertices[mVertexIndices[polygon.vertexIndexOffset + i + 1]];
			RaycastHit hitInfo;
			if(Collisions::TestRayTriangle(ray, p0, p1, p2, hitInfo))
            {
                if(hitInfo.t < outHitInfo.t)
                {
					// Save closest distance.
                    outHitInfo.t = hitInfo.t;
                    
                    // Find surface for this polygon, and then name for the surface.
                    closest = &mObjectNames[surface.objectIndex];
                }
            }
        }
    }
	
	// If no closest object was found, no hits occurred. Early out.
	if(closest == nullptr) { return false; }
	
	// Otherwise, fill in out hit info and return.
	outHitInfo.name = *closest;
	return true;
}

bool BSP::RaycastSingle(const Ray& ray, std::string name, RaycastHit& outHitInfo)
{
	for(auto& polygon : mPolygons)
	{
		// We're only interested in intersections with a certain object.
		// So, if this isn't the object, we can continue!
        BSPSurface& surface = mSurfaces[polygon.surfaceIndex];
        if(mObjectNames[surface.objectIndex] != name) { continue; }
        if(!surface.interactive) { continue; }
		
        Vector3 p0 = mVertices[mVertexIndices[polygon.vertexIndexOffset]];
        for(int i = 1; i < polygon.vertexIndexCount - 1; i++)
		{
            Vector3 p1 = mVertices[mVertexIndices[polygon.vertexIndexOffset + i]];
            Vector3 p2 = mVertices[mVertexIndices[polygon.vertexIndexOffset + i + 1]];
			if(Collisions::TestRayTriangle(ray, p0, p1, p2, outHitInfo))
			{
				// Save name of hit object.
				outHitInfo.name = name;
				return true;
			}
		}
	}
	
	// Couldn't find the given name, or ray didn't intersect object with given name.
	return false;
}

std::vector<RaycastHit> BSP::RaycastAll(const Ray& ray)
{
	std::vector<RaycastHit> hits;
	
	// Iterate through all polygons in the BSP and see if the ray intersects
	// with any triangles inside the polygon. Triangles within the BSP are made
	// up of "triangle fans", so the first vertex in a polygon is shared by all triangles.
	for(auto& polygon : mPolygons)
	{
        BSPSurface& surface = mSurfaces[polygon.surfaceIndex];
        if(!surface.interactive) { continue; }
		
        Vector3 p0 = mVertices[mVertexIndices[polygon.vertexIndexOffset]];
        for(int i = 1; i < polygon.vertexIndexCount - 1; i++)
		{
            Vector3 p1 = mVertices[mVertexIndices[polygon.vertexIndexOffset + i]];
            Vector3 p2 = mVertices[mVertexIndices[polygon.vertexIndexOffset + i + 1]];
			RaycastHit hitInfo;
			if(Collisions::TestRayTriangle(ray, p0, p1, p2, hitInfo))
			{
				// Save hit object name.
                hitInfo.name = mObjectNames[surface.objectIndex];
				
				// Add to hit info vector.
				hits.push_back(hitInfo);
			}
		}
	}

	// Return vector of hits.
	return hits;
}

bool BSP::RaycastPolygon(const Ray& ray, const BSPPolygon* polygon, RaycastHit& outHitInfo)
{
	Vector3 p0 = mVertices[mVertexIndices[polygon->vertexIndexOffset]];
	for(int i = 1; i < polygon->vertexIndexCount - 1; i++)
	{
		Vector3 p1 = mVertices[mVertexIndices[polygon->vertexIndexOffset + i]];
		Vector3 p2 = mVertices[mVertexIndices[polygon->vertexIndexOffset + i + 1]];
		if(Collisions::TestRayTriangle(ray, p0, p1, p2, outHitInfo))
		{
			return true;
		}
	}
	return false;
}

BSPActor* BSP::CreateBSPActor(const std::string& objectName)
{
	// Find index for object name or fail.
	int objectIndex = -1;
	for(int i = 0; i < mObjectNames.size(); i++)
	{
		if(StringUtil::EqualsIgnoreCase(mObjectNames[i], objectName))
		{
			objectIndex = i;
			break;
		}
	}
	if(objectIndex == -1) { return nullptr; }
	
	// OK, we found it! Create the actor.
	BSPActor* actor = new BSPActor(this, objectName);
	
	// Generate AABB from this BSP object's position data.
	bool firstPoint = true;
	AABB aabb;
	for(int surfaceIndex = 0; surfaceIndex < mSurfaces.size(); surfaceIndex++)
	{
        if(mSurfaces[surfaceIndex].objectIndex == objectIndex)
		{
			actor->AddSurface(&mSurfaces[surfaceIndex]);
			
			for(int polygonIndex = 0; polygonIndex < mPolygons.size(); polygonIndex++)
			{
                if(mPolygons[polygonIndex].surfaceIndex == surfaceIndex)
				{
					actor->AddPolygon(&mPolygons[polygonIndex]);
					
                    int start = mPolygons[polygonIndex].vertexIndexOffset;
                    int end = start + mPolygons[polygonIndex].vertexIndexCount;
					for(int k = start; k < end; k++)
					{
						if(firstPoint)
						{
							aabb = AABB(mVertices[mVertexIndices[k]], mVertices[mVertexIndices[k]]);
							firstPoint = false;
						}
						else
						{
							aabb.GrowToContain(mVertices[mVertexIndices[k]]);
						}
					}
				}
			}
		}
	}
	actor->SetAABB(aabb);
	
	// Position actor at center of BSP object position.
	actor->SetPosition(GetPosition(objectName));
	return actor;
}

void BSP::SetVisible(std::string objectName, bool visible)
{
	// Find index of the object name.
	int index = -1;
	for(int i = 0; i < mObjectNames.size(); i++)
	{
		if(StringUtil::EqualsIgnoreCase(mObjectNames[i], objectName))
		{
			index = i;
			break;
		}
	}
	
	// Can't hide an object if the passed name isn't present.
	if(index == -1) { return; }
	
	// All surfaces belonging to this object will be hidden.
	for(auto& surface : mSurfaces)
	{
        if(surface.objectIndex == index)
		{
            surface.visible = visible;
		}
	}
}

void BSP::SetTexture(std::string objectName, Texture* texture)
{
	// Find index of the object name.
	int index = -1;
	for(int i = 0; i < mObjectNames.size(); i++)
	{
		if(StringUtil::EqualsIgnoreCase(mObjectNames[i], objectName))
		{
			index = i;
			break;
		}
	}
	
	// Can't hide an object if the passed name isn't present.
	if(index == -1) { return; }
	
	// All surfaces belonging to this object will be hidden.
	for(auto& surface : mSurfaces)
	{
        if(surface.objectIndex == index)
		{
            surface.texture = texture;
		}
	}
}

bool BSP::Exists(std::string objectName) const
{
	for(int i = 0; i < mObjectNames.size(); i++)
	{
		if(StringUtil::EqualsIgnoreCase(mObjectNames[i], objectName))
		{
			return true;
		}
	}
	return false;
}

bool BSP::IsVisible(std::string objectName) const
{
	// Find index of the object name.
	int index = -1;
	for(int i = 0; i < mObjectNames.size(); i++)
	{
		if(StringUtil::EqualsIgnoreCase(mObjectNames[i], objectName))
		{
			index = i;
			break;
		}
	}
	
	// If can't find object name, it's certainly not visible...
	if(index == -1) { return false; }
	
	// Find any surface belonging to this object and see if it is visible.
	for(auto& surface : mSurfaces)
	{
        if(surface.objectIndex == index)
		{
            return surface.visible;
		}
	}
	
	// Worst case, no surfaces belong to this object. Must not be visible then!
	return false;
}

Vector3 BSP::GetPosition(const std::string& objectName) const
{
	// Find index of the object name.
	int objectIndex = -1;
	for(int i = 0; i < mObjectNames.size(); i++)
	{
		if(StringUtil::EqualsIgnoreCase(mObjectNames[i], objectName))
		{
			objectIndex = i;
			break;
		}
	}
	
	// Couldn't find object!
	//TODO: Maybe we should return true/false with an out parameter?
	if(objectIndex == -1) { return Vector3::Zero; }
	
	// Find index of a surface.
	Vector3 pos = Vector3::Zero;
	int vertexCount = 0;
	for(int i = 0; i < mSurfaces.size(); i++)
	{
        if(mSurfaces[i].objectIndex == objectIndex)
		{
			for(int j = 0; j < mPolygons.size(); j++)
			{
                if(mPolygons[j].surfaceIndex == i)
				{
                    int start = mPolygons[j].vertexIndexOffset;
                    int end = start + mPolygons[j].vertexIndexCount;
					
					for(int k = start; k < end; k++)
					{
						pos += mVertices[mVertexIndices[k]];
						vertexCount++;
					}
				}
			}
		}
	}
	
	// Get average position.
	return pos / vertexCount;
}

void BSP::ApplyLightmap(const BSPLightmap& lightmap)
{
    const std::vector<Texture*>& lightmapTextures = lightmap.GetLightmapTextures();
    for(int i = 0; i < mSurfaces.size(); ++i)
    {
        mSurfaces[i].lightmapTexture = lightmapTextures[i];
    }
}

// For debugging BSP issues, helpful to track polygons rendered and tree depth.
int renderedPolygonCount = 0;
int treeDepth = 0;

void BSP::RenderOpaque(const Vector3& cameraPosition, const Vector3& cameraDirection)
{
    // Activate material for rendering.
    mMaterial.Activate(Matrix4::Identity);
    
    // Reset render stat values.
    renderedPolygonCount = 0;
    treeDepth = 0;
    
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    RenderTree(mNodes[mRootNodeIndex], cameraPosition, cameraDirection);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    //std::cout << "Rendered " << renderedPolygonCount << " polygons." << std::endl;
}

void BSP::RenderTranslucent()
{
    BSPPolygon* polygon = mAlphaPolygons;
    while(polygon != nullptr)
    {
        RenderPolygon(*polygon, true);
        polygon = polygon->next;
    }
    mAlphaPolygons = nullptr;
}

void BSP::RenderTree(const BSPNode& node, const Vector3& cameraPosition, const Vector3& cameraDirection)
{
    // Check signed distance of point to plane to determine if point is in front of, behind, or on the plane.
    float signedDistance = mPlanes[node.planeIndex].GetSignedDistance(cameraPosition);
    
    // Based on camera position/facing direction, we can sometimes avoid rendering entire parts of the BSP tree.
    //TODO: Maybe base this on the camera's FOV rather than hardcoding?
    const float kCameraFacingDot = 0.9f;
    
    // Determine render order for this node.
    // This makes a "front-to-back" renderer, resulting in no overdraw for opaque rendering.
    bool renderCurrent = true;
    int firstNodeIndex = -1;
    int secondNodeIndex = -1;
    if(Math::IsZero(signedDistance))
    {
        // Point is on plane - render front and back trees. Don't render current node.
        renderCurrent = false;
        firstNodeIndex = node.frontChildIndex;
        secondNodeIndex = node.backChildIndex;
    }
    else if(signedDistance > 0.0f)
    {
        // Point is in front of plane - render front, then back trees.
        firstNodeIndex = node.frontChildIndex;
        secondNodeIndex = node.backChildIndex;
        
        // If in front of plane AND facing away from plane, there's no need to render the back at all.
        float dot = Vector3::Dot(mPlanes[node.planeIndex].normal, cameraDirection);
        if(dot > kCameraFacingDot)
        {
            secondNodeIndex = -1;
        }
    }
    else
    {
        // Point is behind plane - render back, then front trees.
        firstNodeIndex = node.backChildIndex;
        secondNodeIndex = node.frontChildIndex;
        
        // If behind plane AND facing away from plane, there's no need to render the front at all.
        float dot = Vector3::Dot(mPlanes[node.planeIndex].normal, cameraDirection);
        if(dot < -kCameraFacingDot)
        {
            secondNodeIndex = -1;
        }
    }
    
    // Render first tree.
    if(firstNodeIndex >= 0 && firstNodeIndex < mNodes.size())
    {
        ++treeDepth;
        RenderTree(mNodes[firstNodeIndex], cameraPosition, cameraDirection);
        --treeDepth;
    }
    
    // Render current, maybe (probably).
    if(renderCurrent)
    {
        // Determine whether polygon sets 1 & 2 are present.
        bool hasPolygon1 = node.polygonIndex != 65535 && node.polygonCount > 0;
        bool hasPolygon2 = node.polygonIndex2 != 65535 && node.polygonCount2 > 0;
        
        // Some debug keys to visualize what polygons are in each set.
        if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_Y))
        {
            hasPolygon1 = false;
        }
        if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_U))
        {
            hasPolygon2 = false;
        }
        
        // Render first set of polygons.
        if(hasPolygon1)
        {
            for(int i = node.polygonIndex; i < node.polygonIndex + node.polygonCount; i++)
            {
                RenderPolygon(mPolygons[i], false);
                ++renderedPolygonCount;
            }
        }
        
        // Render second set of polygons.
        if(hasPolygon2)
        {
            for(int i = node.polygonIndex2; i < node.polygonIndex2 + node.polygonCount2; i++)
            {
                RenderPolygon(mPolygons[i], false);
                ++renderedPolygonCount;
            }
        }
    }
    
    // Render second tree.
    if(secondNodeIndex >= 0 && secondNodeIndex < mNodes.size())
    {
        ++treeDepth;
        RenderTree(mNodes[secondNodeIndex], cameraPosition, cameraDirection);
        --treeDepth;
    }
}

void BSP::RenderPolygon(BSPPolygon& polygon, bool translucent)
{
    // If we have a valid surface reference, use it to get rendering configured.
    BSPSurface& surface = mSurfaces[polygon.surfaceIndex];
    
    // Not going to render non-visible surfaces.
    if(!surface.visible) { return; }
		
    // Retrieve texture and activate it, if possible.
    Texture* tex = surface.texture;
    if(tex != nullptr)
    {
        // If has alpha, don't render it now, but add it to the alpha chain.
        if(tex->GetRenderType() == Texture::RenderType::Translucent && translucent)
        {
            polygon.next = mAlphaPolygons;
            mAlphaPolygons = &polygon;
            return;
        }
        tex->Activate(0);
    }
    else
    {
        Texture::Deactivate();
    }
     
    Texture* lightmapTex = surface.lightmapTexture;
    if(lightmapTex != nullptr)
    {
        lightmapTex->Activate(1);
    }
	
    // Draw the polygon.
    mVertexArray.DrawTriangleFans(polygon.vertexIndexOffset, polygon.vertexIndexCount);
}

void BSP::ParseFromData(char *data, int dataLength)
{
    BinaryReader reader(data, dataLength);
    
    // First 4 bytes: file identifier "NECS" (SCEN backwards).
    std::string identifier = reader.ReadString(4);
    if(identifier != "NECS")
    {
        std::cout << "BSP file does not have SCEN identifier! Instead has " << identifier << std::endl;
        return;
    }
    
    // 4 bytes: version number
    // 4 bytes: content size in bytes
    reader.Skip(8);
    
    // 4 bytes: index of root node for BSP tree.
    mRootNodeIndex = reader.ReadUInt();
    
    // Read in all the header count values.
    int nameCount = reader.ReadUInt();
    int vertexCount = reader.ReadUInt();
    int uvCount = reader.ReadUInt();
    int vertexIndexCount = reader.ReadUInt();
    int uvIndexCount = reader.ReadUInt();
    int surfaceCount = reader.ReadUInt();
    int planeCount = reader.ReadUInt();
    int nodeCount = reader.ReadUInt();
    int polygonCount = reader.ReadUInt();
    
    // Iterate and read all names.
    for(int i = 0; i < nameCount; i++)
    {
        mObjectNames.push_back(reader.ReadString(32));
    }
    
    // Iterate and read surfaces.
    for(int i = 0; i < surfaceCount; i++)
    {
        BSPSurface surface;
        surface.objectIndex = reader.ReadUInt();
        
        surface.texture = Services::GetAssets()->LoadTexture(reader.ReadString(32));
        
        surface.uvOffset = reader.ReadVector2();
        surface.uvScale = reader.ReadVector2();
        
        surface.scale = reader.ReadFloat();
        //std::cout << i << ", " << mObjectNames[surface->objectIndex] << ": uv-off " << surface->uvOffset << ", uv-scale " << surface->uvScale << ", scale " << surface->scale << std::endl;
        
        /*
		 FLAGS - bitmask flags that indicate some interesting aspect about this surface.
         1 => ???
         2 => ???
         4 => interactable?
         8 => invisible?
         16 => light source?
         32 => ???
         64 => ???
         128 => ???
		 
		 // NO flag seems to be dedicated to opaque vs. translucent.
		 
         (RC1, rc4_pjcar uses 76 (64+8+4))
         (RC1, windows often use 12 (8+4))
         (RC1, street lamps have 16 sometimes)
         (RC1, luggage has 68 (64+4))
         (B25, toilet paper has 2)
         (B25/RC1 - many instances of 0/1 too)
        */
		reader.ReadUInt();
        //unsigned int flags = reader.ReadUInt();
		/*
		if(!surface->texture->HasAlpha())
		{
			std::cout << mObjectNames[surface->objectIndex] << ", " << flags << std::endl;
		}
		*/
		//std::cout << mObjectNames[surface->objectIndex] << ", " << flags << std::endl;
		
        mSurfaces.push_back(surface);
    }
    
    // Iterate and read nodes.
    for(int i = 0; i < nodeCount; i++)
    {
        BSPNode node;
        node.frontChildIndex = reader.ReadUShort(); 
        node.backChildIndex = reader.ReadUShort();
        
        node.planeIndex = reader.ReadUShort();
        
        node.polygonIndex = reader.ReadUShort();
        node.polygonIndex2 = reader.ReadUShort();
        node.polygonCount = reader.ReadUShort();
        node.polygonCount2 = reader.ReadUShort();
        
        reader.ReadUShort(); // Unknown value
        
        //std::cout << "Node " << mNodes.size() << ":" << std::endl;
        //std::cout << "  Polygon Idx: " << node->polygonIndex << ", Polygon Count: " << node->polygonCount << std::endl;
        //std::cout << "  Unknown Val 1: " << val1 << std::endl;
        mNodes.push_back(node);
    }
    
    // Iterate and read polygons.
    for(int i = 0; i < polygonCount; i++)
    {
        BSPPolygon polygon;
        polygon.vertexIndexOffset = reader.ReadUShort();
		reader.ReadUByte(); // unknown
		reader.ReadUByte(); // unknown
        polygon.vertexIndexCount = reader.ReadUShort();
        polygon.surfaceIndex = reader.ReadUShort();
        
        mPolygons.push_back(polygon);
    }
    
    // Iterate and read planes.
    for(int i = 0; i < planeCount; i++)
    {
        float normalX = reader.ReadFloat();
        float normalY = reader.ReadFloat();
        float normalZ = reader.ReadFloat();
        float distance = reader.ReadFloat();
        mPlanes.emplace_back(normalX, normalY, normalZ, distance);
    }
    
    // Iterate and read vertices
    for(int i = 0; i < vertexCount; i++)
    {
        mVertices.push_back(reader.ReadVector3());
    }
    
    // Iterate and read UVs
    for(int i = 0; i < uvCount; i++)
    {
        Vector2 uv = reader.ReadVector2();
        mUVs.push_back(uv);
    }
    
    // Iterate and read vertex indexes.
    for(int i = 0; i < vertexIndexCount; i++)
    {
        mVertexIndices.push_back(reader.ReadUShort());
    }
    
    // Iterate and read UV indexes.
    // Skipped for now - not sure if we'll ever need these.
    reader.Skip(uvIndexCount * 2); // 2 bytes per UV index.
    
    // Next up are spheres centers with radiis for each node. Not sure what these are for.
    reader.Skip(nodeCount * 16); // 4 floats per node, each float is 4 bytes
    
    std::cout << "BSP has " << mVertices.size() << " vertices. " << std::endl;
    std::cout << "BSP has " << mUVs.size() << " UVs. " << std::endl;
    std::cout << "BSP has " << mVertexIndices.size() << " indexes. " << std::endl;
    
    mColors.resize(mVertices.size(), Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    
    // Calculate lightmap UVs.
    mLightmapUVs.resize(mUVs.size());
    int totalIndexes = 0;
    for(int i = 0; i < surfaceCount; i++)
    {
        // Some stuff that isn't super clear/we don't currently need.
        reader.Skip(28);
        
        // Number of indices and triangles for this surface.
        int indexCount = reader.ReadUInt();
        int triangleCount = reader.ReadUInt();
        totalIndexes += indexCount;
        
        std::cout << "Surface " <<  i << " (" << mObjectNames[mSurfaces[i].objectIndex] << ")" << std::endl;
        std::cout << "  Index Count: " << indexCount << std::endl;
        
        // Next, a certain number of index values.
        std::vector<unsigned short> indexes;
        for(int j = 0; j < indexCount; j++)
        {
            unsigned short vertexIndex = reader.ReadUShort();
            indexes.push_back(vertexIndex);
            std::cout << "    Index " << j << ": " << vertexIndex << std::endl;
            
            Vector2 uv = (mUVs[vertexIndex] + mSurfaces[i].uvOffset) * mSurfaces[i].uvScale;
            mLightmapUVs[vertexIndex] = uv;
            
            if(i >= 5 && i <= 24)
            {
                //Debug::DrawLine(Vector3::Zero, mVertices[vertexIndex], Color32::Magenta, 120.0f);
                mColors[vertexIndex] = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
            }
            else
            {
                mColors[vertexIndex] = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
            }
        }
        
        std::cout << "  Triangle Count: " << triangleCount << std::endl;
        for(int j = 0; j < triangleCount; j++)
        {
            unsigned short p0Index = reader.ReadUShort();
            unsigned short p1Index = reader.ReadUShort();
            unsigned short p2Index = reader.ReadUShort();
            std::cout << "    Triangle " << j << ": " << p0Index << ", " << p1Index << ", " << p2Index << std::endl;
        }
    }
    std::cout << "We have " << surfaceCount << " surfaces with a total of " << totalIndexes << " vertices" << std::endl;
    
    int totalCountFromPolygons = 0;
    for(auto& polygon : mPolygons)
    {
        totalCountFromPolygons += polygon.vertexIndexCount;
        
        int start = polygon.vertexIndexOffset;
        int end = start + polygon.vertexIndexCount;
        for(int index = start; index < end; ++index)
        {
            unsigned short vertexIndex = mVertexIndices[index];
            Vector2 uv = (mUVs[vertexIndex] + mSurfaces[polygon.surfaceIndex].uvOffset) * mSurfaces[polygon.surfaceIndex].uvScale;
            mLightmapUVs[vertexIndex] = uv;
            
            mColors[vertexIndex] = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
        }
    }
    std::cout << "But if we count all polygon surfaces, we have " << totalCountFromPolygons << " vertices..." << std::endl;
    
    for(auto& polygon : mPolygons)
    {
        int start = polygon.vertexIndexOffset;
        int end = start + polygon.vertexIndexCount;
        for(int index = start; index < end; ++index)
        {
            unsigned short vertexIndex = mVertexIndices[index];
            if(polygon.surfaceIndex == 127)
            {
                Vector2 uv = (mUVs[vertexIndex] + mSurfaces[polygon.surfaceIndex].uvOffset) * mSurfaces[polygon.surfaceIndex].uvScale;
                mLightmapUVs[vertexIndex] = uv;
                
                Debug::DrawLine(Vector3::Zero, mVertices[vertexIndex], Color32::Magenta, 120.0f);
                mColors[vertexIndex] = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
            }
        }
    }
    
    
    // Generate mesh definition.
    MeshDefinition meshDefinition;
    meshDefinition.meshUsage = MeshUsage::Static;
    
    meshDefinition.vertexDefinition.layout = VertexDefinition::Layout::Packed;
    meshDefinition.vertexDefinition.attributes.push_back(VertexAttribute::Position);
    meshDefinition.vertexDefinition.attributes.push_back(VertexAttribute::Color);
    meshDefinition.vertexDefinition.attributes.push_back(VertexAttribute::UV1);
    meshDefinition.vertexDefinition.attributes.push_back(VertexAttribute::UV2);
    
    meshDefinition.vertexCount = static_cast<int>(mVertices.size());
    
    std::vector<float*> vertexData;
    vertexData.push_back(reinterpret_cast<float*>(&mVertices[0]));
    vertexData.push_back(reinterpret_cast<float*>(&mColors[0]));
    vertexData.push_back(reinterpret_cast<float*>(&mUVs[0]));
    vertexData.push_back(reinterpret_cast<float*>(&mLightmapUVs[0]));
    meshDefinition.vertexData = &vertexData[0];
    
    meshDefinition.indexCount = static_cast<int>(mVertexIndices.size());
    meshDefinition.indexData = static_cast<unsigned short*>(&mVertexIndices[0]);
    
    // Create vertex array.
    mVertexArray = VertexArray(meshDefinition);
}
