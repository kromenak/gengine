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

BSP::BSP(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

void BSP::Render(Vector3 cameraPosition)
{
    RenderTree(mNodes[mRootNodeIndex], cameraPosition);
}

bool RayIntersectsTriangle(Vector3 p0, Vector3 p1, Vector3 p2, Ray ray, Vector3& hitPos)
{
    Vector3 e1 = p1 - p0;
    Vector3 e2 = p2 - p0;
    Vector3 p = Vector3::Cross(ray.GetDirection(), e2);
    float a = Vector3::Dot(e1, p);
    
    // If zero, means ray is parallel to triangle plane, which is not an intersection.
    if(Math::IsZero(a)) { return false; }
    
    float f = 1.0f / a;
    
    Vector3 s = ray.GetOrigin() - p0;
    float u = f * Vector3::Dot(s, p);
    if(u < 0.0f || u > 1.0f) { return false; }
    
    Vector3 q = Vector3::Cross(s, e1);
    float v = f * Vector3::Dot(ray.GetDirection(), q);
    if(v < 0.0f || u + v > 1.0f) { return false; }
    
    float t = f * Vector3::Dot(e2, q);
    if(t < 0) { return false; }
    
    hitPos = ray.GetPosition(t);
    return true;
}

std::string* BSP::Intersects(Ray ray)
{
    float closestDist = 9999;
    std::string* closest = nullptr;
    for(auto& polygon : mPolygons)
    {
        Vector3 p0 = mVertices[mVertexIndices[polygon->vertexIndex]];
        for(int i = 1; i < polygon->vertexCount - 1; i++)
        {
            Vector3 p1 = mVertices[mVertexIndices[polygon->vertexIndex + i]];
            Vector3 p2 = mVertices[mVertexIndices[polygon->vertexIndex + i + 1]];
            Vector3 hitPoint;
            if(RayIntersectsTriangle(p0, p1, p2, ray, hitPoint))
            {
                float dist = (hitPoint - ray.GetOrigin()).GetLength();
                if(dist < closestDist)
                {
                    closestDist = dist;
                    
                    // Find surface for this polygon.
                    BSPSurface* surface = mSurfaces[polygon->surfaceIndex];
                    
                    // And then the object for this surface.
                    closest = &mObjectNames[surface->objectIndex];
                }
            }
        }
    }
    return closest;
}

void BSP::Hide(std::string objectName)
{
    // Find index of the object name.
    int index = -1;
    for(int i = 0; i < mObjectNames.size(); i++)
    {
        if(mObjectNames[i] == objectName)
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
        if(surface->objectIndex == index)
        {
            surface->visible = false;
        }
    }
}

void BSP::RenderTree(BSPNode* node, Vector3 position)
{
    // Figure out the location of the camera position relative to the plane.
    PointLocation location = GetPointLocation(position, mPlanes[node->planeIndex]);
    
    // Using "Behind" here acts as a "front-to-back" BSP renderer.
    // Pros: If depth-buffer is enabled, renders opaque graphics most efficiently.
    // Cons: Can't support non-opaque graphics. Only works if depth-buffer is enabled.
    
    // Using "InFrontOf" here acts as a "back-to-front" BSP renderer.
    // Pros: Without depth-buffer, will render both opaque and non-opaque graphics correctly.
    // Cons: More overdraw due to pixel redraw without depth-buffer.
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
        if(node->polygonIndex2 != 65535 && node->polygonCount2 > 0)
        {
            polygonIndex = node->polygonIndex2;
            for(int i = polygonIndex; i < polygonIndex + node->polygonCount2; i++)
            {
                RenderPolygon(mPolygons[i]);
            }
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
        if(node->polygonIndex2 != 65535 && node->polygonCount2 > 0)
        {
            polygonIndex = node->polygonIndex2;
            for(int i = polygonIndex; i < polygonIndex + node->polygonCount2; i++)
            {
                RenderPolygon(mPolygons[i]);
            }
        }
        
        int backNodeIndex = node->backChildIndex;
        if(backNodeIndex >= 0 && backNodeIndex < mNodes.size())
        {
            RenderTree(mNodes[backNodeIndex], position);
        }
    }
}

void BSP::RenderPolygon(BSPPolygon* polygon)
{
    // Can't render a null object DUH.
    if(polygon == nullptr) { return; }
    
    // If we have a valid surface reference, use it to get rendering configured.
    BSPSurface* surface = mSurfaces[polygon->surfaceIndex];
    if(surface != nullptr)
    {
        // Not going to render non-visible surfaces.
        if(!surface->visible) { return; }
        
        // Retrieve texture and activate it, if possible.
        Texture* tex = surface->texture;
        if(tex != nullptr)
        {
            tex->Activate();
        }
        else
        {
            Texture::Deactivate();
        }
    }
    
    // Draw the polygon.
    mVertexArray->Draw(polygon->vertexIndex, polygon->vertexCount);
}

BSP::PointLocation BSP::GetPointLocation(Vector3 position, Plane* plane)
{
    // We can calculate a point on the plane with the normal and distance values.
    Vector3 pointOnPlane = plane->FindClosestPointOnPlane(Vector3::Zero);
    //std::cout << (Vector3::Dot(pointOnPlane, plane->GetNormal()) + plane->GetDistanceFromOrigin()) << std::endl;
    
    // We then get a vector from our position to the plane.
    Vector3 posToPlane = pointOnPlane - position;
    
    // If dot product is zero, the point is on the plane.
    // If greater than zero, the vector and normal are generally facing the same way: the point is behind the plane.
    // If less than zero, the vector and normal are facing away from each other: the point is in front of the plane.
    float dotProduct = Vector3::Dot(posToPlane, plane->GetNormal());
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
        std::string name = reader.ReadString(32);
        mObjectNames.push_back(name);
    }
    
    // Iterate and read surfaces.
    for(int i = 0; i < surfaceCount; i++)
    {
        BSPSurface* surface = new BSPSurface();
        surface->objectIndex = reader.ReadUInt();
        
        surface->textureName = reader.ReadString(32);
        surface->texture = Services::GetAssets()->LoadTexture(surface->textureName);
        
        surface->uvOffset = Vector2(reader.ReadFloat(), reader.ReadFloat());
        surface->uvScale = Vector2(reader.ReadFloat(), reader.ReadFloat());
        
        surface->scale = reader.ReadFloat();
        
        /*
         1 => ???
         2 => ???
         4 => interactable?
         8 => invisible?
         16 => light source?
         32 => ???
         64 => ???
         128 => ???
         
         (RC1, rc4_pjcar uses 76 (64+8+4))
         (RC1, windows often use 12 (8+4))
         (RC1, street lamps have 16 sometimes)
         (RC1, luggage has 68 (64+4))
         (B25, toilet paper has 2)
         (B25/RC1 - many instances of 0/1 too)
        */
        uint flags = reader.ReadUInt();
        
        // Combination of flags 8+4 seems to indicate thing is not visible.
        if(flags == 12)
        {
            surface->visible = false;
        }
        //std::cout << mObjectNames[surface->objectIndex] << ", " << flags << std::endl;
        
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
        node->polygonIndex2 = reader.ReadUShort();
        node->polygonCount = reader.ReadUShort();
        node->polygonCount2 = reader.ReadUShort();
        
        reader.ReadUShort(); // Unknown value
        
        //std::cout << "Node " << mNodes.size() << ":" << std::endl;
        //std::cout << "  Polygon Idx: " << node->polygonIndex << ", Polygon Count: " << node->polygonCount << std::endl;
        //std::cout << "  Unknown Val 1: " << val1 << std::endl;
        
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
        Plane* plane = new Plane(reader.ReadFloat(), reader.ReadFloat(),
                                 reader.ReadFloat(), reader.ReadFloat());
        mPlanes.push_back(plane);
    }
    
    // Iterate and read vertices
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
