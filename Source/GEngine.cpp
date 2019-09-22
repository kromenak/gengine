//
// GEngine.cpp
//
// Clark Kromenaker
//
#include "GEngine.h"

#include <SDL2/SDL.h>

#include "Actor.h"
#include "ButtonIconManager.h"
#include "CharacterManager.h"
#include "Console.h"
#include "FootstepManager.h"
#include "Debug.h"
#include "GameProgress.h"
#include "Scene.h"
#include "Services.h"

GEngine* GEngine::inst = nullptr;
std::vector<Actor*> GEngine::mActors;

GEngine::GEngine() : mRunning(false)
{
    inst = this;
}

bool GEngine::Initialize()
{
	// Initialize reports.
	Services::SetReports(&mReportManager);
	
    // Initialize asset manager.
    Services::SetAssets(&mAssetManager);
    
    // Add "Assets" directory as a possible location for any file.
    mAssetManager.AddSearchPath("Assets/");
    
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
    mAssetManager.LoadBarn("ambient.brn");
    mAssetManager.LoadBarn("common.brn");
    mAssetManager.LoadBarn("core.brn");
    mAssetManager.LoadBarn("day1.brn");
    mAssetManager.LoadBarn("day2.brn");
    mAssetManager.LoadBarn("day3.brn");
    mAssetManager.LoadBarn("day23.brn");
    mAssetManager.LoadBarn("day123.brn");
    
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
	
    //mAssetManager.WriteBarnAssetToFile("PINE2FLAT.BMP");
    //mAssetManager.WriteAllBarnAssetsToFile(".BMP", "Bitmaps");
	
	// Load button icon manager.
	Services::Set<ButtonIconManager>(new ButtonIconManager());
	
	// Load character configs.
	Services::Set<CharacterManager>(new CharacterManager());
	
	// Load floor configs.
	Services::Set<FootstepManager>(new FootstepManager());
	
	// Create game progress.
	Services::Set<GameProgress>(new GameProgress());
	
	LoadScene("R25");
    //LoadScene("RC1");
	
	//mReportManager.Log("Generic", "Rock & Roll");
    return true;
}

void GEngine::Shutdown()
{
	DeleteAllActors();
    
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
    static int nextTicks = 0;
    
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
    for(int i = 0; i < mActors.size(); i++)
    {
        mActors[i]->Update(deltaTime);
    }
	
	// Delete any dead actors...carefully.
	auto it = mActors.begin();
	while(it != mActors.end())
	{
		if((*it)->GetState() == Actor::State::Dead)
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
    
    // Also update audio system (before or after actors?)
    mAudioManager.Update(deltaTime);
    
	// If a sheep is running, show "wait" cursor.
	// If not, go back to normal cursor.
	if(mSheepManager.IsAnyRunning())
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

void GEngine::RemoveActor(Actor* actor)
{
    auto it = std::find(mActors.begin(), mActors.end(), actor);
    if(it != mActors.end())
    {
        mActors.erase(it);
    }
}

void GEngine::LoadSceneInternal()
{
	if(mSceneToLoad.empty()) { return; }
	
	// Delete the current scene, if any.
	if(mScene != nullptr)
	{
		delete mScene;
		mScene = nullptr;
	}
	
	// Get rid of all actors between scenes.
	DeleteAllActors();
	
	//TEMP: Create a console when entering a scene.
	//TODO: Console should persist across all scenes! Need some "Do Not Destroy On Load" style functionality...
	new Console(false);
	
	// Save this new location.
	Services::Get<GameProgress>()->SetLocation(mSceneToLoad);
	
	// Create the new scene.
	mScene = new Scene(mSceneToLoad, Services::Get<GameProgress>()->GetTimeCode());
	
	// Trigger any on-enter actions.
	mScene->OnSceneEnter();
	
	// Clear scene load request.
	mSceneToLoad.clear();
}

void GEngine::DeleteAllActors()
{
	// Delete all actors.
	// Since actor destructor removes from this list, can't iterate and delete.
	while(!mActors.empty())
	{
		delete mActors.back();
	}
}
