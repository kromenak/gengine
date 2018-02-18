//
// BSP.cpp
//
// Clark Kromenaker
//
#include "BSP.h"
#include "BinaryReader.h"
#include "Vector2.h"
#include "Vector3.h"
#include <iostream>
#include "Services.h"

BSP::BSP(std::string name, char* data, int dataLength) :
    Asset(name)
{
    ParseFromData(data, dataLength);
}

void BSP::Render(Vector3 fromPosition)
{
    RenderTree(mNodes[mRootNodeIndex], fromPosition);
}

void BSP::RenderTree(BSPNode *node, Vector3 position)
{
    PointLocation location = GetPointLocation(position, mPlanes[node->planeIndex]);
    if(location == PointLocation::InFrontOf)
    {
        int backNodeIndex = node->backChildIndex;
        if(backNodeIndex >= 0 && backNodeIndex < mNodes.size())
        {
            RenderTree(mNodes[backNodeIndex], position);
        }
        
        // Render polygons at current node.
        int polygonIndex = node->polygonIndex;
        for(int i = polygonIndex; i < polygonIndex + node->polygonCount; i++)
        {
            RenderPolygon(mPolygons[i]);
        }
        
        int frontNodeIndex = node->frontChildIndex;
        if(frontNodeIndex >= 0 && frontNodeIndex < mNodes.size())
        {
            RenderTree(mNodes[frontNodeIndex], position);
        }
    }
    else
    {
        int frontNodeIndex = node->frontChildIndex;
        if(frontNodeIndex >= 0 && frontNodeIndex < mNodes.size())
        {
            RenderTree(mNodes[frontNodeIndex], position);
        }
        
        // Render polygons at current node.
        int polygonIndex = node->polygonIndex;
        for(int i = polygonIndex; i < polygonIndex + node->polygonCount; i++)
        {
            RenderPolygon(mPolygons[i]);
        }
        
        int backNodeIndex = node->backChildIndex;
        if(backNodeIndex >= 0 && backNodeIndex < mNodes.size())
        {
            RenderTree(mNodes[backNodeIndex], position);
        }
    }
}

void BSP::RenderPolygon(BSPPolygon *polygon)
{
    if(polygon != nullptr)
    {
        // If we have a valid surface reference, use it to get rendering configured.
        BSPSurface* surface = mSurfaces[polygon->surfaceIndex];
        if(surface != nullptr)
        {
            // Retrieve texture and activate it, if possible.
            Texture* tex = surface->texture;
            if(tex != nullptr)
            {
                tex->Activate();
            }
        }
        
        // Draw the polygon.
        mVertexArray->Draw(polygon->vertexIndex, polygon->vertexCount);
    }
}

BSP::PointLocation BSP::GetPointLocation(Vector3 position, BSPPlane* plane)
{
    // We can calculate a point on the plane with the normal and distance values.
    Vector3 pointOnPlane = plane->normal * plane->distance;
    
    // We then get a vector from our position to the plane.
    Vector3 posToPlane = pointOnPlane - position;
    
    // If dot product is zero, the point is on the plane.
    // If greater than zero, the vector and normal are generally facing the same way: the point is behind the plane.
    // If less than zero, the vector and normal are facing away from each other: the point is in front of the plane.
    float dotProduct = Vector3::Dot(posToPlane, plane->normal);
    if(Math::IsZero(dotProduct))
    {
        return BSP::PointLocation::OnPlane;
    }
    else if(dotProduct > 0.0f)
    {
        return BSP::PointLocation::Behind;
    }
    else
    {
        return BSP::PointLocation::InFrontOf;
    }
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
        BSPSurface* surface = new BSPSurface();
        surface->objectIndex = reader.ReadUInt();
        surface->textureName = reader.ReadString(32);
        surface->textureName.append(".BMP");
        surface->texture = Services::GetAssets()->LoadTexture(surface->textureName);
        
        surface->uvOffset = Vector2(reader.ReadFloat(), reader.ReadFloat());
        surface->uvScale = Vector2(reader.ReadFloat(), reader.ReadFloat());
        
        surface->scale = reader.ReadFloat();
        reader.ReadUInt(); // Flags
        
        mSurfaces.push_back(surface);
    }
    
    // Iterate and read nodes.
    for(int i = 0; i < nodeCount; i++)
    {
        BSPNode* node = new BSPNode();
        node->frontChildIndex = reader.ReadUShort(); // These appear to be 65535 if invalid/null.
        node->backChildIndex = reader.ReadUShort();
        
        node->planeIndex = reader.ReadUShort();
        
        node->polygonIndex = reader.ReadUShort();
        reader.ReadUShort(); // Unknown value
        node->polygonCount = reader.ReadUShort();
        
        reader.ReadUShort(); // Unknown value
        reader.ReadUShort(); // Unknown value
        
        mNodes.push_back(node);
    }
    
    // Iterate and read polygons.
    for(int i = 0; i < polygonCount; i++)
    {
        BSPPolygon* polygon = new BSPPolygon();
        polygon->vertexIndex = reader.ReadUShort();
        reader.ReadUShort(); // Unknown value
        polygon->vertexCount = reader.ReadUShort();
        
        polygon->surfaceIndex = reader.ReadUShort();
        
        mPolygons.push_back(polygon);
    }
    
    // Iterate and read planes.
    for(int i = 0; i < planeCount; i++)
    {
        BSPPlane* plane = new BSPPlane();
        plane->normal = Vector3(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        plane->distance = reader.ReadFloat();
        mPlanes.push_back(plane);
    }
    
    // Iterate and read vertices
    std::cout << "About to read verts: " << reader.GetPosition() << std::endl;
    for(int i = 0; i < vertexCount; i++)
    {
        Vector3 v(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        mVertices.push_back(v);
    }
    
    // Iterate and read UVs
    for(int i = 0; i < uvCount; i++)
    {
        mUVs.push_back(Vector2(reader.ReadFloat(), reader.ReadFloat()));
    }
    
    // Iterate and read vertex indexes.
    for(int i = 0; i < vertexIndexCount; i++)
    {
        mVertexIndices.push_back(reader.ReadUShort());
    }
    
    // Iterate and read UV indexes.
    for(int i = 0; i < uvIndexCount; i++)
    {
        mUVIndices.push_back(reader.ReadUShort());
    }
    
    // We need to build the vertex array by passing our buffer of vertex values
    // and our buffer of vertex index values, so we can draw indexed primitives.
    float* vertsPtr = new float[mVertices.size() * 3];
    for(int i = 0; i < mVertices.size(); i++)
    {
        vertsPtr[i * 3] = mVertices[i].GetX();
        vertsPtr[i * 3 + 1] = mVertices[i].GetY();
        vertsPtr[i * 3 + 2] = mVertices[i].GetZ();
    }
    
    ushort* vertIndexesPtr = new ushort[mVertexIndices.size()];
    for(int i = 0; i < mVertexIndices.size(); i++)
    {
        vertIndexesPtr[i] = mVertexIndices[i];
    }
    
    // Create the vertex array from the verts and vert indexes.
    mVertexArray = new GLVertexArray(vertsPtr, (int)(mVertices.size() * 3), vertIndexesPtr, (int)mVertexIndices.size());
    
    // Also pass along UV data.
    float* uvsPtr = (float*)&mUVs[0];
    mVertexArray->SetUV1(uvsPtr, (int)(mUVs.size() * 2));
    
    /*
    //CK: Don't currently need this bounding sphere stuff.
    // Read in bounding spheres.
    for(int i = 0; i < nodeCount; i++)
    {
        Vector3 center(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        float radius = reader.ReadFloat();
    }
    
    // Read in more bounding sphere stuff.
    for(int i = 0; i < surfaceCount; i++)
    {
        Vector3 center(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
        float radius = reader.ReadFloat();
        
        float chrome = reader.ReadFloat();
        float grazing = reader.ReadFloat();
        float chromeColor = reader.ReadFloat();
        
        int indexCount = reader.ReadUInt();
        int triangleCount = reader.ReadUInt();
        
        for(int j = 0; j < indexCount; j++)
        {
            unsigned short index = reader.ReadUShort();
        }
        
        for(int j = 0; j < triangleCount; j++)
        {
            reader.ReadUShort();
            reader.ReadUShort();
            reader.ReadUShort();
        }
    }
    */
}
