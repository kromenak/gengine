#include "BSP.h"

#include <bitset>
#include <iostream>

#include "AssetManager.h"
#include "BinaryReader.h"
#include "BSPActor.h"
#include "BSPLightmap.h"
#include "Debug.h"
#include "ReportManager.h"
#include "Shader.h"
#include "StringUtil.h"
#include "Texture.h"
#include "Vector2.h"
#include "Vector3.h"

void BSPSurface::Activate(const Material& material)
{
    // Activate texture to use for diffuse color.
    if(texture != nullptr)
    {
        texture->Activate(0);
    }
    else
    {
        Texture::Deactivate();
    }

    // Activate lightmap texture and multiplier.
    if((flags & BSPSurface::kIgnoreLightmapFlag) != 0 ||
       (flags & BSPSurface::kShadowTextureFlag)  != 0)
    {
        // Some surfaces ignore lightmaps.
        // Just use "plain white" and multiplier of 1 to effectively "do nothing" in lightmap calcs.
        Texture::White.Activate(1);
        material.GetShader()->SetUniformFloat("uLightmapMultiplier", 1.0f);
    }
    else
    {
        // This surface DOES use lightmaps, so activate it!
        // GK3 also implements a feature called "2x Lighting" - basically, just double lightmap colors to make the scene look brighter!
        if(lightmapTexture != nullptr)
        {
            lightmapTexture->Activate(1);
        }
        material.GetShader()->SetUniformFloat("uLightmapMultiplier", 2.0f);
    }

    // Lightmap scale/offsets are used in shaders to calculate proper lightmap UVs.
    Vector4 lightmapUvScaleOffset(lightmapUvScale.x,
                                  lightmapUvScale.y,
                                  lightmapUvOffset.x,
                                  lightmapUvOffset.y);
    material.GetShader()->SetUniformVector4("uLightmapScaleOffset", lightmapUvScaleOffset);
}

void BSP::Load(uint8_t* data, uint32_t dataLength)
{
    ParseFromData(data, dataLength);

    // Use lightmap shader for BSP rendering.
    mMaterial.SetShader(gAssetManager.LoadShader("3D-Lightmap"));
}

BSPActor* BSP::CreateBSPActor(const std::string& objectName)
{
    // Find index for object name or fail.
    uint32_t objectIndex = GetObjectIndex(objectName);
    if(objectIndex == UINT32_MAX)
    {
        gReportManager.Log("Error", StringUtil::Format("Error: scene '%s' does not have a scene model of name '%s'", "", objectName.c_str()));
        return nullptr;
    }

    // OK, we found it! Create the actor.
    BSPActor* actor = new BSPActor(this, objectName);

    // Generate AABB from this BSP object's position data.
    bool firstPoint = true;
    AABB aabb;
    for(size_t surfaceIndex = 0; surfaceIndex < mSurfaces.size(); surfaceIndex++)
    {
        if(mSurfaces[surfaceIndex].objectIndex == objectIndex)
        {
            actor->AddSurface(&mSurfaces[surfaceIndex]);

            for(size_t polygonIndex = 0; polygonIndex < mPolygons.size(); polygonIndex++)
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
		
        Vector3& p0 = mVertices[mVertexIndices[polygon.vertexIndexOffset]];
        for(int i = 1; i < polygon.vertexIndexCount - 1; i++)
        {
            Vector3& p1 = mVertices[mVertexIndices[polygon.vertexIndexOffset + i]];
            Vector3& p2 = mVertices[mVertexIndices[polygon.vertexIndexOffset + i + 1]];
			RaycastHit hitInfo;
			if(Intersect::TestRayTriangle(ray, p0, p1, p2, hitInfo.t))
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

bool BSP::RaycastSingle(const Ray& ray, const std::string& name, RaycastHit& outHitInfo)
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
			if(Intersect::TestRayTriangle(ray, p0, p1, p2, outHitInfo.t))
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
			if(Intersect::TestRayTriangle(ray, p0, p1, p2, hitInfo.t))
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
		if(Intersect::TestRayTriangle(ray, p0, p1, p2, outHitInfo.t))
		{
			return true;
		}
	}
	return false;
}

void BSP::SetFloorObjectName(const std::string& floorObjectName)
{
    mFloorObjectIndex = GetObjectIndex(floorObjectName);
}

void BSP::GetFloorInfo(const Vector3& position, float& outHeight, Texture*& outTexture)
{
    // Didn't hit anything by default.
    outHeight = 0.0f;
    outTexture = nullptr;

    // No floor was defined - early out.
    if(mFloorObjectIndex == UINT32_MAX) { return; }

    // Calculate ray origin using passed position, but really high in the air!
    Vector3 rayOrigin = position;
    rayOrigin.y = 10000.0f;

    // Create ray with origin high in the sky and pointing straight down.
    Ray ray(rayOrigin, -Vector3::UnitY);

    // Iterate polygons.
    float nearestT = FLT_MAX;
    for(auto& polygon : mPolygons)
    {
        // Only consider polygons that are part of the floor object.
        BSPSurface& surface = mSurfaces[polygon.surfaceIndex];
        if(surface.objectIndex != mFloorObjectIndex) { continue; }

        // See if ray intersects any triangles in this polygon.
        Vector3 p0 = mVertices[mVertexIndices[polygon.vertexIndexOffset]];
        for(int i = 1; i < polygon.vertexIndexCount - 1; i++)
        {
            Vector3 p1 = mVertices[mVertexIndices[polygon.vertexIndexOffset + i]];
            Vector3 p2 = mVertices[mVertexIndices[polygon.vertexIndexOffset + i + 1]];
            float t = 0.0f;
            if(Intersect::TestRayTriangle(ray, p0, p1, p2, t) && t < nearestT)
            {
                nearestT = t;
                outHeight = ray.GetPoint(t).y;
                outTexture = surface.texture;
            }
        }
    }
}

void BSP::SetVisible(const std::string& objectName, bool visible)
{
	// Find index of the object name.
    uint32_t index = GetObjectIndex(objectName);
	if(index == UINT32_MAX) { return; }
	
	// All surfaces belonging to this object will be hidden.
	for(auto& surface : mSurfaces)
	{
        if(surface.objectIndex == index)
		{
            surface.visible = visible;
		}
	}
}

void BSP::SetTexture(const std::string& objectName, Texture* texture)
{
    // Find index of the object name.
    uint32_t index = GetObjectIndex(objectName);
    if(index == UINT32_MAX) { return; }
	
	// All surfaces belonging to this object will be hidden.
	for(auto& surface : mSurfaces)
	{
        if(surface.objectIndex == index)
		{
            surface.texture = texture;
		}
	}
}

bool BSP::Exists(const std::string& objectName) const
{
    return GetObjectIndex(objectName) != UINT32_MAX;
}

bool BSP::IsVisible(const std::string& objectName) const
{
	// Find index of the object name.
	uint32_t index = GetObjectIndex(objectName);
	if(index == UINT32_MAX) { return false; }
	
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
    uint32_t objectIndex = GetObjectIndex(objectName);

	//TODO: Maybe we should return true/false with an out parameter?
	if(objectIndex == UINT32_MAX) { return Vector3::Zero; }
	
	// Find all surfaces that belong to this object and sum their positions.
	Vector3 pos = Vector3::Zero;
	int vertexCount = 0;
	for(size_t i = 0; i < mSurfaces.size(); i++)
	{
        if(mSurfaces[i].objectIndex == objectIndex)
		{
			for(size_t j = 0; j < mPolygons.size(); j++)
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
	return pos / static_cast<float>(vertexCount);
}

void BSP::ApplyLightmap(const BSPLightmap& lightmap)
{
    // Apply lightmap textures to each surface.
    const std::vector<Texture*>& lightmapTextures = lightmap.GetLightmapTextures();
    for(size_t i = 0; i < mSurfaces.size(); ++i)
    {
        mSurfaces[i].lightmapTexture = lightmapTextures[i];
    }

    // Update light colors now that lightmap textures are populated.
    for(auto& light : mLights)
    {
        if(mSurfaces[light.surfaceIndex].lightmapTexture != nullptr)
        {
            int width = mSurfaces[light.surfaceIndex].lightmapTexture->GetWidth();
            int height = mSurfaces[light.surfaceIndex].lightmapTexture->GetHeight();

            // Use a single center point to calculate the color?
            //light.color = mSurfaces[light.surfaceIndex].lightmapTexture->GetPixelColor32(width / 2, height / 2);

            // Or sum and average all pixels in the lightmap?
            Vector3 sums;
            for(int i = 0; i < width; ++i)
            {
                for(int j = 0; j < height; ++j)
                {
                    Color32 color = mSurfaces[light.surfaceIndex].lightmapTexture->GetPixelColor32(i, j);
                    sums.x += color.GetR();
                    sums.y += color.GetG();
                    sums.z += color.GetB();
                }
            }
            sums /= static_cast<float>(width * height);
            light.color = Color32(static_cast<int>(sums.x), static_cast<int>(sums.y), static_cast<int>(sums.z));
        }
    }
}

void BSP::DebugDrawAmbientLights(const Vector3& position)
{
    // Visualized what BSP ambient lights affect an object at a given position.
    for(auto& light : mLights)
    {
        float distSq = (position - light.position).GetLengthSq();
        float radiusSq = light.radius * light.radius;
        if(distSq <= radiusSq)
        {
            Debug::DrawSphere(light.position, light.radius, light.color);
        }
    }
}

Color32 BSP::CalculateAmbientLightColor(const Vector3& position)
{
    //TODO: Unclear if this logic is right - may need more work.
    // For each ambient light, see if the position lands inside the light's "sphere of influence."
    Color32 color = Color32::Black;
    for(auto& light : mLights)
    {
        float distSq = (position - light.position).GetLengthSq();
        float radiusSq = light.radius * light.radius;
        if(distSq <= radiusSq)
        {
            // If so, this ambient color contributes to the result, based on how close to center of sphere we are.
            color += Color32::Lerp(light.color, Color32::Black, distSq / radiusSq);
        }
    }
    return color;

    /*
    Vector3 sum;
    int count = 0;
    for(auto& light : mLights)
    {
        float distSq = (position - light.position).GetLengthSq();
        float radiusSq = light.radius * light.radius;
        if(distSq <= radiusSq)
        {
            // If so, this ambient color contributes to the result, based on how close to center of sphere we are.
            Color32 color = Color32::Lerp(light.color, Color32::Black, distSq / radiusSq);
            sum.x += color.GetR() * color.GetR();
            sum.y += color.GetG() * color.GetG();
            sum.z += color.GetB() * color.GetB();
            count++;
        }
    }
    
    return Color32(static_cast<int>(Math::Sqrt(sum.x / count)),
                   static_cast<int>(Math::Sqrt(sum.y / count)),
                   static_cast<int>(Math::Sqrt(sum.z / count)));
    */
}

// For debugging BSP issues, helpful to track polygons rendered and tree depth.
namespace
{
    int renderedPolygonCount = 0;
    int treeDepth = 0;
}

void BSP::RenderOpaque(const Vector3& cameraPosition, const Vector3& cameraDirection)
{
    // Activate material for rendering.
    mMaterial.Activate(Matrix4::Identity);

    // Reset render stat values.
    renderedPolygonCount = 0;
    treeDepth = 0;

    #if defined(USE_TRUE_BSP_RENDERING)
    // NORMAL BSP RENDERING
    // Process the tree and nodes to only render what's in front of the camera.
    // Seems like it'd be quite efficient...BUT modern graphics hardware is actually quite bad at this, due to the number of draw calls!
    RenderTree(mNodes[mRootNodeIndex], cameraPosition, cameraDirection);
    #else
    // ALTERNATIVE BSP RENDERING
    // Just render every surface lol.
    // Surprisingly more efficient than "correct" BSP rendering, since it's fewer draw calls.
    for(auto& surface : mSurfaces)
    {
        if(!surface.visible) { continue; }
        if(surface.IsTranslucent()) { continue; }

        // Activate
        surface.Activate(mMaterial);

        // Draw
        for(auto& polygon : surface.polygons)
        {
            mVertexArray.DrawTriangleFans(polygon.vertexIndexOffset, polygon.vertexIndexCount);
            renderedPolygonCount++;
        }
    }
    #endif
    
    //std::cout << "Rendered " << renderedPolygonCount << " polygons." << std::endl;
}

void BSP::RenderTranslucent()
{
    // Activate material for rendering.
    mMaterial.Activate(Matrix4::Identity);

    #if defined(USE_TRUE_BSP_RENDERING)
    BSPPolygon* polygon = mAlphaPolygons;
    while(polygon != nullptr)
    {
        RenderPolygon(*polygon, true);
        polygon = polygon->next;
    }
    mAlphaPolygons = nullptr;
    #else
    for(auto& surface : mSurfaces)
    {
        if(!surface.visible) { continue; }
        if(!surface.IsTranslucent()) { continue; }

        // Activate
        surface.Activate(mMaterial);

        // Draw
        for(auto& polygon : surface.polygons)
        {
            mVertexArray.DrawTriangleFans(polygon.vertexIndexOffset, polygon.vertexIndexCount);
        }
    }
    #endif
}

uint32_t BSP::GetObjectIndex(const std::string& objectName) const
{
    // Even though there could be "size_t" elements in an array, the BSP file format only supports "uint32_t" elements.
    for(size_t i = 0; i < mObjectNames.size(); i++)
    {
        if(StringUtil::EqualsIgnoreCase(mObjectNames[i], objectName))
        {
            return static_cast<uint32_t>(i);
        }
    }
    return UINT32_MAX;
}

void BSP::ParseFromData(uint8_t* data, uint32_t dataLength)
{
    BinaryReader reader(data, dataLength);
    
    // 4 bytes: file identifier "NECS" (SCEN backwards).
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
    
    // 36 bytes: Read in all the header count values.
    uint32_t nameCount = reader.ReadUInt();
    uint32_t vertexCount = reader.ReadUInt();
    uint32_t uvCount = reader.ReadUInt();
    uint32_t vertexIndexCount = reader.ReadUInt();
    uint32_t otherIndexCount = reader.ReadUInt();
    uint32_t surfaceCount = reader.ReadUInt();
    uint32_t planeCount = reader.ReadUInt();
    uint32_t nodeCount = reader.ReadUInt();
    uint32_t polygonCount = reader.ReadUInt();
    
    // Iterate and read all names.
    mObjectNames.resize(nameCount);
    for(uint32_t i = 0; i < nameCount; ++i)
    {
        reader.ReadString(32, mObjectNames[i]);
    }

    // When reading surfaces, we need to keep track of whether we've previously processed a shadow texture.
    // We don't want to process a single texture more than once!
    std::vector<Texture*> processedShadowTextures;
    
    // Iterate and read surfaces.
    mSurfaces.resize(surfaceCount);
    for(uint32_t i = 0; i < surfaceCount; ++i)
    {
        BSPSurface& surface = mSurfaces[i];
        surface.objectIndex = reader.ReadUInt();

        surface.texture = gAssetManager.LoadSceneTexture(reader.ReadString(32), GetScope());
        
        surface.lightmapUvOffset = reader.ReadVector2();
        surface.lightmapUvScale = reader.ReadVector2();
        
        reader.ReadFloat(); // Unknown - I had assumed this was a scale earlier, but I'm not sure.
        
        // Read in flags. See header for known flags.
        surface.flags = reader.ReadUInt();
        /*
        if(surface.flags != 0)
        {
            std::bitset<32> bs(surface.flags);
            std::cout << mObjectNames[surface.objectIndex] << ": " << bs << std::endl;
        }
        */
    }
    
    // Iterate and read nodes.
    mNodes.resize(nodeCount);
    for(uint32_t i = 0; i < nodeCount; ++i)
    {
        BSPNode& node = mNodes[i];
        node.frontChildIndex = reader.ReadUShort(); 
        node.backChildIndex = reader.ReadUShort();
        
        node.planeIndex = reader.ReadUShort();
        
        node.polygonIndex = reader.ReadUShort();
        node.polygonIndex2 = reader.ReadUShort();
        
        node.polygonCount = reader.ReadUShort();
        node.polygonCount2 = reader.ReadUShort();
        
        // Unknown value - only known values: 0 for root node, 348, 1007, 1009, 1010, 1012, 1074, 30723 for other nodes.
        // Values seem to be the same for all nodes in a single BSP file?
        reader.ReadUShort();
    }
    
    // Iterate and read polygons.
    mPolygons.resize(polygonCount);
    for(uint32_t i = 0; i < polygonCount; ++i)
    {
        BSPPolygon& polygon = mPolygons[i];
        polygon.vertexIndexOffset = reader.ReadUShort();
        
        // Unknown value - sometimes zero, but almost always 1073.
        // Mysteriously stuck right in the middle of each polygon hmm...
        reader.ReadUShort();
        
        polygon.vertexIndexCount = reader.ReadUShort();
        polygon.surfaceIndex = reader.ReadUShort();

        // With "true" BSP rendering, maintaining a list of polygons for a surface is not needed.
        // But otherwise, it's handy to have.
        #if !defined(USE_TRUE_BSP_RENDERING)
        mSurfaces[polygon.surfaceIndex].polygons.push_back(polygon);
        #endif
    }
    
    // Iterate and read planes.
    mPlanes.reserve(planeCount);
    for(uint32_t i = 0; i < planeCount; ++i)
    {
        float normalX = reader.ReadFloat();
        float normalY = reader.ReadFloat();
        float normalZ = reader.ReadFloat();
        float distance = reader.ReadFloat();
        mPlanes.emplace_back(normalX, normalY, normalZ, distance);
    }
    
    // Iterate and read vertices.
    mVertices.resize(vertexCount);
    for(uint32_t i = 0; i < vertexCount; ++i)
    {
        mVertices[i] = reader.ReadVector3();
    }
    
    // Iterate and read UVs.
    mUVs.resize(uvCount);
    for(uint32_t i = 0; i < uvCount; ++i)
    {
        mUVs[i] = reader.ReadVector2();
    }
    
    // Iterate and read vertex indexes.
    mVertexIndices.resize(vertexIndexCount);
    for(uint32_t i = 0; i < vertexIndexCount; ++i)
    {
        mVertexIndices[i] = reader.ReadUShort();
    }
    
    // Iterate and read other indexes.
    // After reviewing all BSP files, these always exactly match the vertex indexes? Why bother?
    // Skipped for now - not sure if we'll ever need these.
    reader.Skip(otherIndexCount * 2); // 2 bytes per index.
    
    // Next up are spheres centers with radiis for each node. Not sure what these are for.
    reader.Skip(nodeCount * 16); // 4 floats per node, each float is 4 bytes

    // The remaining data is NOT present in all BSP files (ex: DEFAULT.BSP).
    // So, only read it if there appears to be more data in the file.
    uint32_t position = reader.GetPosition();
    if(position < dataLength)
    {
        for(uint32_t i = 0; i < surfaceCount; ++i)
        {
            Vector3 lightPos = reader.ReadVector3();
            float radius = reader.ReadFloat();
            reader.Skip(12);

            if(radius > 0.0f) // && (mSurfaces[i].flags & 1) != 0)
            {
                mLights.emplace_back();
                mLights.back().surfaceIndex = i;
                mLights.back().position = lightPos;
                mLights.back().radius = radius;
                mLights.back().color = Color32::Black;
            }

            // Number of indices and triangles for this surface.
            uint32_t indexCount = reader.ReadUInt();
            uint32_t triangleCount = reader.ReadUInt();

            // Next, a certain number of vertex & triangle indexes...for what?
            /*
            for(uint32_t j = 0; j < indexCount; j++)
            {
                reader.ReadUShort(); // vertexIndex
            }
            for(uint32_t j = 0; j < triangleCount; j++)
            {
                reader.ReadUShort(); // p0Index
                reader.ReadUShort(); // p1Index
                reader.ReadUShort(); // p2Index
            }
            */
            reader.Skip(indexCount * 2); // 2 bytes per index
            reader.Skip(triangleCount * 6); // 6 bytes per triangle
        }
    }

    // Generate mesh definition.
    MeshDefinition meshDefinition(MeshUsage::Static, mVertices.size());
    meshDefinition.SetVertexLayout(VertexLayout::Packed);

    meshDefinition.AddVertexData(VertexAttribute::Position, &mVertices[0]);
    meshDefinition.AddVertexData(VertexAttribute::UV1, &mUVs[0]);

    meshDefinition.SetIndexData(mVertexIndices.size(), &mVertexIndices[0]);
    meshDefinition.ownsData = false;
    
    // Create vertex array.
    mVertexArray = VertexArray(meshDefinition);
}

#if defined(USE_TRUE_BSP_RENDERING)
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

        // Render first set of polygons.
        if(hasPolygon1)
        {
            for(int i = node.polygonIndex; i < node.polygonIndex + node.polygonCount; i++)
            {
                RenderPolygon(mPolygons[i], false);
            }
        }

        // Render second set of polygons.
        if(hasPolygon2)
        {
            for(int i = node.polygonIndex2; i < node.polygonIndex2 + node.polygonCount2; i++)
            {
                RenderPolygon(mPolygons[i], false);
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

    // Activate
    surface.Activate(mMaterial);

    // If has alpha, don't render it now, but add it to the alpha chain.
    if(surface.texture != nullptr)
    {
        if(surface.texture->GetRenderType() == Texture::RenderType::Translucent && !translucent)
        {
            polygon.next = mAlphaPolygons;
            mAlphaPolygons = &polygon;
            return;
        }
    }

    // Draw the polygon.
    mVertexArray.DrawTriangleFans(polygon.vertexIndexOffset, polygon.vertexIndexCount);
    ++renderedPolygonCount;
}
#endif