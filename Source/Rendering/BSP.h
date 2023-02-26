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
    // Indexes of front and back nodes.
    // Front is in front of the plane for this node; back is behind the plane.
    // These appear to be 65535 (ushort max value) if invalid/null.
    unsigned short frontChildIndex;
    unsigned short backChildIndex;
    
    // Each node has a plane that divides child geometry into "front" and "back".
    // By testing what side of the plane the camera is on, correct order to draw geometry can be determined.
    unsigned short planeIndex;
    
    // Polygons associated with this node (offset + count into polygon list).
    // This makes up the majority of the BSP geometry.
    unsigned short polygonIndex;
    unsigned short polygonCount;
    
    // Less commonly used, but a second set of polygons for this node.
    // These appear to be used for rendering 2-sided polygons (though I haven't totally figured that out yet).
    unsigned short polygonIndex2;
    unsigned short polygonCount2;
};

// A polygon is made up of at least three vertices and can be rendered.
struct BSPPolygon
{
    // Index of surface this polygon belongs to.
    // The surface defines appearance (texture, lightmap) and other properties.
    unsigned short surfaceIndex;
    
    // BSP is rendered using indexed geometry.
    // These are an offset + count into the index array, defining what vertices make up this polygon.
    unsigned short vertexIndexOffset;
    unsigned short vertexIndexCount;
	
	// Used for creating a linked list of alpha surfaces.
    BSPPolygon* next = nullptr;
};

// A surface consists of one or more polygons.
// It defined appearance (visibility, texture, lightmap info) and behavior (raycasting).
struct BSPSurface
{
    // An "object" is really just a name - a way to group surfaces logically.
    // For example, several surfaces could make up a "door" object.
    unsigned int objectIndex = 0;
    
    // The texture used for this surface.
    Texture* texture = nullptr;
    
    // An optional lightmap texture - applied from a lightmap asset.
    //TODO: this works OK, but it'd be more efficient to use a lightmap atlas instead of many individual textures.
    Texture* lightmapTexture = nullptr;
    
    // UVs used for the lightmap are often different from the UVs used for diffuse textures.
    // The surface defines offset/scale to apply to each UV to properly render a lightmap on that surface.
    Vector2 lightmapUvOffset;
    Vector2 lightmapUvScale;
    
    // Flags defining surface properties.
    unsigned int flags = 0;
    static const unsigned int kUnknownFlag1 = 1; // applied on certain walls, ceilings, and floors
    static const unsigned int kUnknownFlag2 = 2; // seems to correlate to surfaces that are hard to see or not very noticeable
    static const unsigned int kUnknownFlag3 = 4; // surfaces that emit light, hit tests
    static const unsigned int kIgnoreLightmapFlag = 8; // surfaces that emit light, shadow casters, "hide these models", hit tests
    static const unsigned int kUnknownFlag5 = 16; // lamp shades, light fixtures, lanterns, stained glass, chandilier, sconces, etc.
    static const unsigned int kUnknownFlag6 = 32; // possibly never used - couldn't find in any BSP
    static const unsigned int kUnknownFlag7 = 64; // some RC1, ARM, CDB, LHE, PLO, TE5 (shadow bridge) objects have this
    
    // If true, this surface is rendered.
    bool visible = true;
	
	// If true, interactive (can be hit by raycasts).
	bool interactive = true;

    // HACK: The polygons that belong to this surface.
    // Not required for normal BSP rendering, but needed for alternative method.
    std::vector<BSPPolygon> polygons;

    void Activate(const Material& materialS);
};

// Represents an amount of ambient light emitted from a BSP surface.
// As dynamic models navigate the scene, they can query the BSP to calculate an approximate "ambient color" at the current position.
struct BSPAmbientLight
{
    // The surface this ambient light corresponds to.
    int surfaceIndex = 0;

    // The position/radius, which defines a "sphere of influence" for the ambient light.
    Vector3 position;
    float radius = 1.0f;

    // The color of the ambient light in this sphere, derived/divined from lightmap data.
    Color32 color;
};

class BSP : public Asset
{
public:
    BSP(const std::string& name, char* data, int dataLength);
    
	BSPActor* CreateBSPActor(const std::string& objectName);
	
    bool RaycastNearest(const Ray& ray, RaycastHit& outHitInfo);
	bool RaycastSingle(const Ray& ray, const std::string& name, RaycastHit& outHitInfo);
	std::vector<RaycastHit> RaycastAll(const Ray& ray);
	bool RaycastPolygon(const Ray& ray, const BSPPolygon* polygon, RaycastHit& outHitInfo);

    void SetFloorObjectName(const std::string& floorObjectName) { mFloorObjectName = floorObjectName; }
    void GetFloorInfo(const Vector3& position, float& height, Texture*& texture);
	
	void SetVisible(std::string objectName, bool visible);
	void SetTexture(std::string objectName, Texture* texture);
	
	bool Exists(std::string objectName) const;
	bool IsVisible(std::string objectName) const;
    
	Vector3 GetPosition(const std::string& objectName) const;

    void ApplyLightmap(const BSPLightmap& lightmap);
    void DebugDrawAmbientLights(const Vector3& position);
    Color32 CalculateAmbientLightColor(const Vector3& position);
    
    void RenderOpaque(const Vector3& cameraPosition, const Vector3& cameraDirection);
    void RenderTranslucent();
	
private:
    // Identifies the root node in the node list.
    // Rendering always starts from this node.
    unsigned int mRootNodeIndex = 0;
    
    // List of nodes. These all reference one another to form a tree structure.
    std::vector<BSPNode> mNodes;
    
    // Planes used by nodes to determine camera location during tree traversal.
    std::vector<Plane> mPlanes;
    
    // All the polygons that make up the scene - nodes traversed during rendering,
    // reference polygons to be rendered at that node.
    std::vector<BSPPolygon> mPolygons;
    
    // Opaque rendering occurs front-to-back, and a linked list of translucent polygons are generated.
    // The linked list can be iterated afterwards to render translucent geometry in the correct order.
    // Saw this in id's Quake/Doom code and thought it was pretty cool!
    BSPPolygon* mAlphaPolygons = nullptr;
    
    // Surfaces are referenced by polygons, define surface properties like texture and lighting.
    std::vector<BSPSurface> mSurfaces;
    
    // Each BSP map is logically divided into objects.
    std::vector<std::string> mObjectNames;
    
    // Vertex attributes for BSP mesh.
    std::vector<Vector3> mVertices;
    std::vector<Vector2> mUVs;
    
    // Vertex indices for BSP mesh.
    std::vector<unsigned short> mVertexIndices;
    
    // Vertex array is loaded up with vertices/uvs/indices to perform rendering.
    VertexArray mVertexArray;
    
    // Material for rendering BSP.
	Material mMaterial;

    // Data used for determining ambient lighting for dynamic models navigating the BSP environment.
    // Kind of like light probes, but way simpler/jankier.
    std::vector<BSPAmbientLight> mLights;

    // The name of the BSP object that represents the floor.
    std::string mFloorObjectName;

    void RenderTree(const BSPNode& node, const Vector3& cameraPosition, const Vector3& cameraDirection);
    void RenderPolygon(BSPPolygon& polygon, bool translucent);
    
    void ParseFromData(char* data, int dataLength);
};
