//
// BSP.h
//
// Clark Kromenaker
//
// Geometry data for a scene. In-memory
// representation of a .BSP file.
//
#pragma once
#include "Types.h"
#include "Asset.h"
#include "Vector2.h"
#include "Vector3.h"
#include <vector>
#include <string>

// A node in the BSP tree.
struct BSPNode
{
    // Indexes of front front and back child nodes.
    // One is a BSPNode that draws in front of me, the other draws behind me.
    ushort frontChildIndex;
    ushort backChildIndex;
    
    // The plane associated with this node.
    // Used to make front/back distinction, I believe.
    ushort planeIndex;
    
    // Offset and count into the polygon list.
    // These are the polygons that should be drawn as part of this node, I guess.
    ushort polygonIndex;
    ushort polygonCount;
};

// A plane in 3D space, which we can represent with a normal and distance from origin.
struct BSPPlane
{
    Vector3 normal;
    float distance;
};

// A polygon is a renderable thing, one or more are associated with each BSP node.
// The polygon itself just references other pieces of data.
struct BSPPolygon
{
    // Index of a surface object, which conveys the polygon's texture, UV coords, etc.
    ushort surfaceIndex;
    
    // An index into the mVertexIndices array (an index to an index), plus
    // the count of values from that index that are associated with this polygon.
    // These are ALSO offsets into the UV indices array - direct correlation.
    ushort vertexIndexIndex;
    ushort vertexIndexCount;
};

struct BSPSurface
{
    // An index to an object. An "object" is really just a string name value,
    // so this is sort of a way to group surfaces logically.
    uint objectIndex;
    
    // This correlates exactly to a texture asset name, without the BMP extension.
    std::string textureName;
    
    // Each vertex has a UV coordinate, but a surface can also specify
    // an offset or scale for the texture, so it looks good graphically.
    Vector2 uvOffset;
    Vector2 uvScale;
    
    // Not yet sure what this scales - is it an overall scale on top of UV-specific scales???
    float scale;
};

class BSP : Asset
{
public:
    BSP(std::string name, char* data, int dataLength);
    
private:
    // Points to the root node in our node list.
    // Almost always 0, but maybe not in some cases?
    uint mRootNodeIndex;
    
    // List of nodes. These all reference one another to form a tree structure.
    std::vector<BSPNode*> mNodes;
    
    // Planes and polygons, referenced by nodes.
    std::vector<BSPPlane*> mPlanes;
    std::vector<BSPPolygon*> mPolygons;
    
    // Surfaces are referenced by polygons, define surface properties like texture.
    std::vector<BSPSurface*> mSurfaces;
    
    // Each BSP map is logically divided into objects. These names are
    // referenced by surfaces, so multiple surfaces can be associated with an object.
    std::vector<std::string> mObjectNames;
    
    // Vertices are vertex positions (X, Y, Z), while indices are indexes into the vertex list.
    std::vector<Vector3> mVertices;
    std::vector<uint> mVertexIndices;
    
    // UVs are UV coordinates (U, V), while indices are indexes into the UVs list.
    std::vector<Vector2> mUVs;
    std::vector<uint> mUVIndices;
    
    //TODO: bounding spheres?
    
    void ParseFromData(char* data, int dataLength);
};
