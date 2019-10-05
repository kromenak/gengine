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
#include "Console.h"
#include "GEngine.h"
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
	
	static ReportManager* GetReports() { return sReportManager; }
	static void SetReports(ReportManager* reportManager) { sReportManager = reportManager; }
	
	static Console* GetConsole() { return sConsole; }
	static void SetConsole(Console* console) { sConsole = console; }
	
	template<class T> static void Set(T* instance);
	template<class T> static T* Get();
    
private:
    static AssetManager* sAssetManager;
    static InputManager* sInputManager;
    static Renderer* sRenderer;
    static AudioManager* sAudio;
    static SheepManager* sSheep;
	static ReportManager* sReportManager;
	static Console* sConsole;
	
	// General-purpose mapping from class Type to class instance.
	// Use "Set" to add an entry and "Get" to retrieve an entry.
	//TODO: Should "Services" be in charge of deleting these on exit?
	static std::unordered_map<Type, void*> sTypeToInstancePointer;
};

template<class T> void Services::Set(T* instance)
{
	// Get type of class. So, note that this only works if the class has RTTI.
	Type type = T::GetType();
	
	// Just create a mapping from the type to the instance!
	sTypeToInstancePointer[type] = instance;
}

template<class T> T* Services::Get()
{
	Type type = T::GetType();
	
	// Attempt to retrieve and return the instance from the type.
	auto it = sTypeToInstancePointer.find(type);
	if(it != sTypeToInstancePointer.end())
	{
		return static_cast<T*>(it->second);
	}
	return nullptr;
}
