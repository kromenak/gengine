//
// BSP.h
//
// Clark Kromenaker
//
// Geometry data for a scene. In-memory representation of a .BSP file.
//
#pragma once
#include "Asset.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "Material.h"
#include "Mesh.h"
#include "Plane.h"
#include "Ray.h"
#include "Collisions.h"
#include "Vector2.h"
#include "Vector3.h"

class BSPActor;
class BSPLightmap;
class Texture;

// A node in the BSP tree.
struct BSPNode
{
    // Indexes of front front and back child nodes.
    // One is a BSPNode that draws in front of me, the other draws behind me.
    // These appear to be 65535 if invalid/null.
    unsigned short frontChildIndex;
    unsigned short backChildIndex;
    
    // The plane associated with this node.
    // Used to make front/back distinction, I believe.
    unsigned short planeIndex;
    
    // Offset and count into the polygon list.
    // These are the polygons that should be drawn as part of this node, I guess.
    unsigned short polygonIndex;
    unsigned short polygonCount;
    
    // Nodes seem to optionally also define a second set of polygons.
    unsigned short polygonIndex2;
    unsigned short polygonCount2;
};

// A polygon is a renderable thing, one or more are associated with each BSP node.
// The polygon itself just references other pieces of data.
struct BSPPolygon
{
    // Index of surface this polygon belongs to.
    unsigned short surfaceIndex;
    
    // BSP is rendered using indexed geometry. A list of vertices and a list of indices are sent to the graphics system.
    // These are an offset + count into the index array, defining what vertices make up this polygon.
    unsigned short vertexIndexOffset;
    unsigned short vertexIndexCount;
	
	// Used for creating a linked list of alpha surfaces.
	BSPPolygon* next;
};

// A surface is made up of one or more polygons.
// It defined appearance (visibility, texture, lightmap info) and behavior (raycasting).
struct BSPSurface
{
    // An index to an object. An "object" is really just a string name value,
    // so this is sort of a way to group surfaces logically.
    unsigned int objectIndex;
    
    // The texture used for this surface.
    Texture* texture = nullptr;
    
    // An optional lightmap texture - applied from a lightmap texture.
    Texture* lightmapTexture = nullptr;
    
    // Each vertex has a UV coordinate, but a surface can also specify
    // an offset or scale for the texture, so it looks good graphically.
    Vector2 uvOffset;
    Vector2 uvScale;
    
    // Not yet sure what this scales - is it an overall scale on top of UV-specific scales???
    float scale = 1.0f;
    
    // If true, this surface is visible. If not, hidden.
    bool visible = true;
	
	// If true, interactive (can be hit by raycasts). If false, not interactive.
	bool interactive = true;
};

class BSP : public Asset
{
public:
    BSP(std::string name, char* data, int dataLength);
    
	BSPActor* CreateBSPActor(const std::string& objectName);
	
    bool RaycastNearest(const Ray& ray, RaycastHit& outHitInfo);
	bool RaycastSingle(const Ray& ray, std::string name, RaycastHit& outHitInfo);
	std::vector<RaycastHit> RaycastAll(const Ray& ray);
	bool RaycastPolygon(const Ray& ray, const BSPPolygon* polygon, RaycastHit& outHitInfo);
	
	void SetVisible(std::string objectName, bool visible);
	void SetTexture(std::string objectName, Texture* texture);
	
	bool Exists(std::string objectName) const;
	bool IsVisible(std::string objectName) const;
    
	Vector3 GetPosition(const std::string& objectName) const;
    
    void ApplyLightmap(const BSPLightmap& lightmap);
    
    void RenderOpaque(const Vector3& cameraPosition, const Vector3& cameraDirection);
    void RenderTranslucent();
	
private:
    // Points to the root node in our node list.
    // Almost always 0, but maybe not in some cases?
    unsigned int mRootNodeIndex = 0;
    
    // List of nodes. These all reference one another to form a tree structure.
    std::vector<BSPNode> mNodes;
    
    // Each BSP node uses a plane to calculate whether we are rendering in front of or behind the node.
    // This dictates the path taken through the node tree when rendering.
    std::vector<Plane> mPlanes;
    
    // A polygon describes a unit of renderable geometry. It references vertex indices to do this.
    // When rendering BSP, we determine which polygons are visible and render them.
    std::vector<BSPPolygon> mPolygons;
    
    // For translucent rendering, we can create a polygon chain when doing opaque rendering.
    // We then iterate the chain afterwards to properly render translucent stuff.
    // Saw this in id's Quake/Doom code and thought it was pretty cool!
    BSPPolygon* mAlphaPolygons = nullptr;
    
    // Surfaces are referenced by polygons, define surface properties like texture and lighting.
    std::vector<BSPSurface> mSurfaces;
    
    // Each BSP map is logically divided into objects. An object is made up of multiple surfaces.
    std::vector<std::string> mObjectNames;
    
    // Vertex attributes for BSP mesh.
    std::vector<Vector3> mVertices;
    std::vector<Vector4> mColors;
    std::vector<Vector2> mUVs;
    std::vector<Vector2> mLightmapUVs;
    
    // Vertex indices for BSP mesh.
    std::vector<unsigned short> mVertexIndices;
    
    // Vertex array is loaded up with vertices/uvs/indices to perform rendering.
    VertexArray mVertexArray;
    
    // Material for rendering BSP.
	Material mMaterial;
    
    void RenderTree(const BSPNode& node, const Vector3& cameraPosition, const Vector3& cameraDirection);
    void RenderPolygon(BSPPolygon& polygon, bool translucent);
    
    void ParseFromData(char* data, int dataLength);
};
