//
// Services.h
//
// Clark Kromenaker
//
// A locator for globally available services in the game.
//
// Some services have static accessors, but there are also general purpose
// "Get" and "Set" function that should allow *any* instance to be globally available.
//
// This is similar to a Singleton, but is more loosely coupled. A benefit
// of this would be the use of Interfaces to swap services.
//
#pragma once
#include <unordered_map>

#include "AssetManager.h"
#include "AudioManager.h"
#include "InputManager.h"
#include "Renderer.h"
#include "SheepManager.h"
#include "Type.h"

class Services
{
public:
    static AssetManager* GetAssets() { return sAssetManager; }
    static void SetAssets(AssetManager* assets) { sAssetManager = assets; }
    
    static InputManager* GetInput() { return sInputManager; }
    static void SetInput(InputManager* input) { sInputManager = input; }
    
    static Renderer* GetRenderer() { return sRenderer; }
    static void SetRenderer(Renderer* rend) { sRenderer = rend; }
    
    static AudioManager* GetAudio() { return sAudio; }
    static void SetAudio(AudioManager* aud) { sAudio = aud; }
    
    static SheepManager* GetSheep() { return sSheep; }
    static void SetSheep(SheepManager* shp) { sSheep = shp; }
	
	template<class T> static void Set(T* instance);
	template<class T> static T* Get();
    
private:
    static AssetManager* sAssetManager;
    static InputManager* sInputManager;
    static Renderer* sRenderer;
    static AudioManager* sAudio;
    static SheepManager* sSheep;
	
	// General-purpose mapping from class Type to class instance.
	// Use "Set" to add an entry and "Get" to retrieve an entry.
	static std::unordered_map<Type, void*> sTypeToInstancePointer;
};

template<class T> void Services::Set(T* instance)
{
	// Make this "static" so we can AVOID doing the type generation every time this function
	// is called with this class type T. This will only run the first time the function is called.
	// Note: we *could* limit use of Set/Get to only classes that make use of TYPE_DECL and TYPE_DEF...but I'd rather not for now!
	Type type = T::GetType();
	
	// Just create a mapping from the type to the instance!
	sTypeToInstancePointer[type] = instance;
}

template<class T> T* Services::Get()
{
	// See note in "Set" about why this is static.
	Type type = T::GetType();
	
	// Attempt to retrieve and return the instance from the type.
	auto it = sTypeToInstancePointer.find(type);
	if(it != sTypeToInstancePointer.end())
	{
		return static_cast<T*>(it->second);
	}
	return nullptr;
}
