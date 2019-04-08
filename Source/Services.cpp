//
// Services.cpp
//
// Clark Kromenaker
//
#include "Services.h"

// Define static variables for Services here.
AssetManager* Services::sAssetManager = nullptr;
InputManager* Services::sInputManager = nullptr;
Renderer* Services::sRenderer = nullptr;
AudioManager* Services::sAudio = nullptr;
SheepManager* Services::sSheep = nullptr;
ReportManager* Services::sReportManager = nullptr;

std::unordered_map<Type, void*> Services::sTypeToInstancePointer;
