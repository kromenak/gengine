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

BSP::BSP(std::string name, char* data, int dataLength) :
    Asset(name)
{
    ParseFromData(data, dataLength);
}

void BSP::ParseFromData(char *data, int dataLength)
{
    BinaryReader reader(data, dataLength);
    
    // First 4 bytes: file identifier "NECS" (SCEN backwards).
    char identifier[4];
    reader.Read(identifier, 4);
    if(!strcmp(identifier, "NECS"))
    {
        std::cout << "BSP file does not have SCEN identifier!" << std::endl;
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
        node->frontChildIndex = reader.ReadUShort();
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
        polygon->vertexIndexIndex = reader.ReadUShort();
        reader.ReadUShort(); // Unknown value
        polygon->vertexIndexCount = reader.ReadUShort();
        
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
    for(int i = 0; i < vertexCount; i++)
    {
        mVertices.push_back(Vector3(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat()));
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
}
