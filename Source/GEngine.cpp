//
// GEngine.cpp
//
// Clark Kromenaker
//
#include "GEngine.h"

#include <SDL2/SDL.h>

#include "ActionManager.h"
#include "Actor.h"
#include "VerbManager.h"
#include "CharacterManager.h"
#include "ConsoleUI.h"
#include "Debug.h"
#include "DialogueManager.h"
#include "FootstepManager.h"
#include "GameProgress.h"
#include "InventoryManager.h"
#include "LocationManager.h"
#include "Scene.h"
#include "Services.h"
#include "TextInput.h"

GEngine* GEngine::inst = nullptr;
std::vector<Actor*> GEngine::mActors;

GEngine::GEngine() :
	mRunning(false)
{
    inst = this;
}

bool GEngine::Initialize()
{
	// Initialize reports.
	Services::SetReports(&mReportManager);
	
	// Initialize console.
	Services::SetConsole(&mConsole);
	mConsole.SetReportStream(&mReportManager.GetReportStream("Console"));
	
    // Initialize asset manager.
    Services::SetAssets(&mAssetManager);
    
    // Add "Assets" directory as a possible location for any file.
    mAssetManager.AddSearchPath("Assets/");
	
	// Add "Assets/GK3" directory, which should contain the actual assets from GK3 data folder.
	mAssetManager.AddSearchPath("Assets/GK3/");
    
    // Initialize input.
    Services::SetInput(&mInputManager);
    
    // Initialize renderer.
    if(!mRenderer.Initialize())
    {
        return false;
    }
    Services::SetRenderer(&mRenderer);
    
    // Initialize audio.
    if(!mAudioManager.Initialize())
    {
        return false;
    }
    Services::SetAudio(&mAudioManager);
    
    // For simplicity right now, let's just load all barns at once.
	std::vector<std::string> barns = {
		"ambient.brn",
		"common.brn",
		"core.brn",
		"day1.brn",
		"day2.brn",
		"day3.brn",
		"day23.brn",
		"day123.brn"
	};
	for(auto& barn : barns)
	{
		if(!mAssetManager.LoadBarn(barn))
		{
			std::string error = "Could not load barn: " + barn;
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
									 "GEngine",
									 error.c_str(),
									 nullptr);
			return false;
		}
	}
    
    // Initialize sheep manager.
    Services::SetSheep(&mSheepManager);
    
    //SDL_Log(SDL_GetBasePath());
    //SDL_Log(SDL_GetPrefPath("Test", "GK3"));
	
	// Load cursors and use the default one to start.
	mDefaultCursor = mAssetManager.LoadCursor("C_POINT.CUR");
	mHighlightRedCursor = mAssetManager.LoadCursor("C_ZOOM.CUR");
	mHighlightBlueCursor = mAssetManager.LoadCursor("C_ZOOM_2.CUR");
	mWaitCursor = mAssetManager.LoadCursor("C_WAIT.CUR");
	UseDefaultCursor();
	
    //mAssetManager.WriteBarnAssetToFile("BLKMUSTACHE_3.BMP");
    //mAssetManager.WriteAllBarnAssetsToFile(".BMP", "Bitmaps");
	
	// Load button icon manager.
	Services::Set<VerbManager>(new VerbManager());
	
	// Load character configs.
	Services::Set<CharacterManager>(new CharacterManager());
	
	// Load floor configs.
	Services::Set<FootstepManager>(new FootstepManager());
	
	// Create game progress.
	Services::Set<GameProgress>(new GameProgress());
	
	// Create inventory manager.
	Services::Set<InventoryManager>(new InventoryManager());
	
	// Create locations manager.
	Services::Set<LocationManager>(new LocationManager());
	
	// Create action manager.
	Services::Set<ActionManager>(&mActionManager);
	mActionManager.Init();
	
	// Create dialogue manager.
	Services::Set<DialogueManager>(new DialogueManager());
	
	// Create console UI - this persists for the entire game.
	ConsoleUI* consoleUI = new ConsoleUI(false);
	consoleUI->SetIsDestroyOnLoad(false);
	
	//TEMP: Load scene as though starting a new game.
	//TODO: Should really show logos, show title screen, allow restore or new game choice.
	Services::Get<GameProgress>()->SetTimeblock(Timeblock("110A"));
    LoadScene("LBY");
	
	/*
	TODO: This code allows writing out a vertex animation's frames as individual OBJ files.
	TODO: Maybe move this to some exporter class or something?
	Model* gabMod = mAssetManager.LoadModel("GAB.MOD");
	gabMod->WriteToObjFile("GAB.OBJ");
	
	GKActor* actor = new GKActor("GAB");
	actor->GetMeshRenderer()->SetModel(gabMod);
	
	VertexAnimation* anim = mAssetManager.LoadVertexAnimation("GAB_GABWALK.ACT");
	for(int i = 0; i < anim->GetFrameCount(); ++i)
	{
		actor->SampleAnimation(anim, i);
		actor->GetMeshRenderer()->GetModel()->WriteToObjFile("GAB_" + std::to_string(i) + ".OBJ");
	}
	*/
	
	//SheepScript* ss = Services::GetSheep()->Compile("/Users/Clark/Projects/gengine/Assets/Conditions.shp");
	//Services::GetSheep()->Execute(ss);
	
	//mReportManager.Log("Generic", "Rock & Roll");
    return true;
}

void GEngine::Shutdown()
{
	// Delete all actors.
	for(auto& actor : mActors)
	{
		delete actor;
	}
	mActors.clear();
	
    mRenderer.Shutdown();
    mAudioManager.Shutdown();
    
    //TODO: Ideally, I don't want the engine to know about SDL.
    SDL_Quit();
}

void GEngine::Run()
{
    // We are running!
    mRunning = true;
    
    // Loop until not running anymore.
    while(mRunning)
    {
		// Our main loop: inputs, updates, outputs.
        ProcessInput();
        Update();
        GenerateOutputs();
		
		// After frame is done, check whether we need a scene change.
		LoadSceneInternal();
    }
}

void GEngine::Quit()
{
    mRunning = false;
}

void GEngine::UseDefaultCursor()
{
	if(mDefaultCursor != nullptr)
	{
		mActiveCursor = mDefaultCursor;
		mActiveCursor->Activate();
	}
}

void GEngine::UseHighlightCursor()
{
	if(mHighlightRedCursor != nullptr)
	{
		mActiveCursor = mHighlightRedCursor;
		mActiveCursor->Activate();
	}
}

void GEngine::UseWaitCursor()
{
	if(mWaitCursor != nullptr)
	{
		mActiveCursor = mWaitCursor;
		mActiveCursor->Activate();
	}
}

void GEngine::ProcessInput()
{
    // Update the input manager.
    // Retrieve input device states for us to use.
    mInputManager.Update();
    
    // We'll poll for events here. Catch the quit event.
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
			case SDL_KEYDOWN:
			{
				if(event.key.keysym.sym == SDLK_BACKSPACE)
				{
					TextInput* textInput = mInputManager.GetTextInput();
					if(textInput != nullptr)
					{
						textInput->DeletePrev();
					}
				}
				else if(event.key.keysym.sym == SDLK_DELETE)
				{
					TextInput* textInput = mInputManager.GetTextInput();
					if(textInput != nullptr)
					{
						textInput->DeleteNext();
					}
				}
				else if(event.key.keysym.sym == SDLK_LEFT)
				{
					TextInput* textInput = mInputManager.GetTextInput();
					if(textInput != nullptr)
					{
						textInput->MoveCursorBack();
					}
				}
				else if(event.key.keysym.sym == SDLK_RIGHT)
				{
					TextInput* textInput = mInputManager.GetTextInput();
					if(textInput != nullptr)
					{
						textInput->MoveCursorForward();
					}
				}
				else if(event.key.keysym.sym == SDLK_HOME)
				{
					TextInput* textInput = mInputManager.GetTextInput();
					if(textInput != nullptr)
					{
						textInput->MoveCursorToStart();
					}
				}
				else if(event.key.keysym.sym == SDLK_END)
				{
					TextInput* textInput = mInputManager.GetTextInput();
					if(textInput != nullptr)
					{
						textInput->MoveCursorToEnd();
					}
				}
				break;
			}
			
			case SDL_TEXTINPUT:
			{
				//TODO: Make sure not copy or pasting.
				TextInput* textInput = mInputManager.GetTextInput();
				if(textInput != nullptr)
				{
					textInput->Insert(event.text.text);
				}
				break;
			}
				
            case SDL_QUIT:
                Quit();
                break;
        }
    }
    
    // Quit game on escape press for now.
    if(mInputManager.IsKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        Quit();
    }
}

void GEngine::Update()
{
    // Tracks the next "GetTicks" value that is acceptable to perform an update.
    static unsigned int nextTicks = 0;
    
    // Tracks the last ticks value each time we run this loop.
    static uint32_t lastTicks = 0;
    
    // Limit to ~60FPS. "nextTicks" is always +16 at start of frame.
    // If we get here again and 16ms have not passed, we wait.
	while(SDL_GetTicks() < nextTicks) { }
	
    // Get current ticks and save next ticks as +16. Limits FPS to ~60.
    uint32_t currentTicks = SDL_GetTicks();
    nextTicks = currentTicks + 16;
	
    // Calculate the time delta.
	uint32_t deltaTicks = currentTicks - lastTicks;
    float deltaTime = deltaTicks * 0.001f;
	
	// Save last ticks for next frame.
    lastTicks = currentTicks;
    
    // Limit the time delta. At least 0s, and at most, 0.05s.
	if(deltaTime < 0.0f) { deltaTime = 0.0f; }
    if(deltaTime > 0.05f) { deltaTime = 0.05f; }
    
    // Update all actors.
    for(size_t i = 0; i < mActors.size(); i++)
    {
        mActors[i]->Update(deltaTime);
    }
	
	// Delete any destroyed actors.
	DeleteDestroyedActors();
    
    // Also update audio system (before or after actors?)
    mAudioManager.Update(deltaTime);
    
	// If a sheep is running, show "wait" cursor.
	// If not, go back to normal cursor.
	if(mActionManager.IsActionPlaying())
	{
		UseWaitCursor();
	}
	else
	{
		if(mActiveCursor == mWaitCursor)
		{
			UseDefaultCursor();
		}
	}
	
	// Update active cursor.
    if(mCursor != nullptr)
    {
        mCursor->Update(deltaTime);
    }
	
	// Update debug visualizations.
	Debug::Update(deltaTime);
}

void GEngine::GenerateOutputs()
{
    mRenderer.Render();
}

void GEngine::AddActor(Actor* actor)
{
    mActors.push_back(actor);
}

void GEngine::LoadSceneInternal()
{
	if(mSceneToLoad.empty()) { return; }
	
	// Delete the current scene, if any.
	if(mScene != nullptr)
	{
		mScene->Unload();
		delete mScene;
		mScene = nullptr;
	}
	
	// Destroy any actors that are destroy on load.
	for(auto& actor : mActors)
	{
		if(actor->IsDestroyOnLoad())
		{
			actor->Destroy();
		}
	}
	
	// After destroy pass, delete destroyed actors.
	DeleteDestroyedActors();
	
	// Create the new scene.
	//TODO: Scene constructor should probably ONLY take a scene name.
	//TODO: Internally, we can call to GameProgress or whatnot as needed, but that's very GK3-specific stuff.
	mScene = new Scene(mSceneToLoad, Services::Get<GameProgress>()->GetTimeblock());
	
	// Load the scene - this is separate from constructor
	// b/c load operations may need to reference the scene itself!
	mScene->Load();
	
	// Clear scene load request.
	mSceneToLoad.clear();
}

void GEngine::DeleteDestroyedActors()
{
	//TODO: Maybe switch to a "swap to end then delete" strategy.
	
	// Use iterator so we can carefully erase and delete actors without too many headaches.
	auto it = mActors.begin();
	while(it != mActors.end())
	{
		if((*it)->IsDestroyed())
		{
			Actor* actor = (*it);
			it = mActors.erase(it);
			delete actor;
		}
		else
		{
			++it;
		}
	}
}
