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
    // Index of a surface object, which conveys the polygon's texture, UV coords, etc.
    unsigned short surfaceIndex;
    
    // An index into the mVertexIndices array (an index to an index), plus
    // the count of values from that index that are associated with this polygon.
    // These are ALSO offsets into the UV indices array - direct correlation.
    unsigned short vertexIndex;
    unsigned short vertexCount;
	
	// Used for creating a linked list of alpha surfaces.
	BSPPolygon* next;
};

struct BSPSurface
{
    // An index to an object. An "object" is really just a string name value,
    // so this is sort of a way to group surfaces logically.
    unsigned int objectIndex;
    
    // This correlates exactly to a texture asset name, without the BMP extension.
    std::string textureName;
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
    // When identifying the position of a point relative to a plane in the
    // BSP tree, the result can be Front, Back, or on the plane itself.
    enum class PointLocation
    {
        InFrontOf,
        Behind,
        OnPlane
    };
	
public:
    BSP(std::string name, char* data, int dataLength);
    
    void ApplyLightmap(const BSPLightmap& lightmap);
    
    void Render(Vector3 cameraPosition);
	void RenderOpaque(Vector3 cameraPosition);
	void RenderTranslucent(Vector3 cameraPosition);
    
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
	
private:
	enum class RenderType
	{
		Any,
		Opaque,
		Translucent
	};
	
    // Points to the root node in our node list.
    // Almost always 0, but maybe not in some cases?
    unsigned int mRootNodeIndex;
    
    // List of nodes. These all reference one another to form a tree structure.
    std::vector<BSPNode*> mNodes;
    
    // Planes and polygons, referenced by nodes.
    std::vector<Plane*> mPlanes;
    std::vector<BSPPolygon*> mPolygons;
    
    // Surfaces are referenced by polygons, define surface properties like texture.
    std::vector<BSPSurface*> mSurfaces;
    
    // Each BSP map is logically divided into objects. These names are
    // referenced by surfaces, so multiple surfaces can be associated with an object.
    std::vector<std::string> mObjectNames;
    
    // Vertices are vertex positions (X, Y, Z), while indices are indexes into the vertex list.
    std::vector<Vector3> mVertices;
    std::vector<unsigned int> mVertexIndices;
    
    // UVs are UV coordinates (U, V), while indices are indexes into the UVs list.
    std::vector<Vector2> mUVs;
    std::vector<unsigned int> mUVIndices;
    
    // Mesh for rendering BSP.
    Mesh* mMesh = nullptr;
	
	// Material for rendering BSP.
	// We rely on a few assumptions right now - no shader is set, so we assume default shader (3D-Tex-Diffuse) is used.
	// We also rely on textures being activated separately from the material.
	Material mMaterial;
	
	// For translucent rendering, we can create a polygon chain when doing opaque rendering.
	// We then iterate the chain afterwards to properly render translucent stuff.
	// Saw this in id's Quake/Doom code and thought it was pretty cool!
	BSPPolygon* mAlphaPolygons = nullptr;
    
    //TODO: bounding spheres?
	
    void RenderTree(BSPNode* node, Vector3 cameraPosition, RenderType renderType);
    void RenderPolygon(BSPPolygon* polygon, RenderType renderType);
    
    PointLocation GetPointLocation(Vector3 position, Plane* plane);
    
    void ParseFromData(char* data, int dataLength);
};
