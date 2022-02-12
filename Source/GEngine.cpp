#include "GEngine.h"

#include <SDL2/SDL.h>

#include "ActionManager.h"
#include "Actor.h"
#include "CharacterManager.h"
#include "ConsoleUI.h"
#include "Debug.h"
#include "DialogueManager.h"
#include "FileSystem.h"
#include "FootstepManager.h"
#include "GameProgress.h"
#include "InventoryManager.h"
#include "Loader.h"
#include "Localizer.h"
#include "LocationManager.h"
#include "Paths.h"
#include "Profiler.h"
#include "SaveManager.h"
#include "Scene.h"
#include "Services.h"
#include "TextInput.h"
#include "TimeblockScreen.h"
#include "Timers.h"
#include "TitleScreen.h"
#include "ThreadPool.h"
#include "VerbManager.h"

GEngine* GEngine::sInstance = nullptr;

GEngine::GEngine()
{
    assert(sInstance == nullptr);
    sInstance = this;
}

bool GEngine::Initialize()
{
    TIMER_SCOPED("GEngine::Initialize");

    // Init threads.
    ThreadUtil::Init();
    ThreadPool::Init(4);

	// Initialize reports.
	Services::SetReports(&mReportManager);
	
	// Initialize console.
	Services::SetConsole(&mConsole);
	mConsole.SetReportStream(&mReportManager.GetReportStream("Console"));
	
    // Initialize asset manager.
    Services::SetAssets(&mAssetManager);

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
        TIMER_SCOPED(barn.c_str());
        if(!mAssetManager.LoadBarn(barn))
        {
            // Generate expected path for this asset.
            std::string path = Paths::GetDataPath(Path::Combine({ "Data", barn }));
            
            // Generate error and show error box.
            std::string error = StringUtil::Format("Could not load barn %s.\n\nMake sure Data directory is populated before running the game.", path.c_str());
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                     "Gabriel Knight 3",
                                     error.c_str(),
                                     nullptr);
            return false;
        }
    }

    // Initialize input.
    Services::SetInput(&mInputManager);

    // Initialize sheep manager.
    Services::SetSheep(&mSheepManager);

    // Create layer manager.
    Services::Set<LayerManager>(&mLayerManager);

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
    
    // Load cursors and use the default one to start.
    // Must happen after barn assets are loaded.
    Services::Set<CursorManager>(&mCursorManager);
    mCursorManager.Init();
    mCursorManager.UseLoadCursor();
    
    // Create localizer.
    Services::Set<Localizer>(new Localizer());
    
	// Load verb manager.
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
	
    // Create video player.
    Services::Set<VideoPlayer>(&mVideoPlayer);
    mVideoPlayer.Initialize();
    
	// Create console UI - this persists for the entire game.
	ConsoleUI* consoleUI = new ConsoleUI(false);
	consoleUI->SetIsDestroyOnLoad(false);

    // Non-debug: do the full game presentation - company logos, intro movie, title screen.
    //#define FORCE_TITLE_SCREEN
    #if defined(NDEBUG) || defined(FORCE_TITLE_SCREEN)
    //HACK/TODO: Ideally, we'd like to either make initial loading much shorter, OR play videos while initial loading is happening.
    //HACK/TODO: However, my multithreading approach is clearly super incorrect and hacky, so that introduces race conditions between the renderer and game logic.
    //HACK/TODO: For now, we'll just do loading before moving on to movies, but this is something to look at more closely.
    Loader::DoAfterLoading([this](){

        // Play opening movie.
        mVideoPlayer.Play("Sierra.avi", true, true, [this](){

            // On first launch of game, show the intro movie before the title screen.
            // Otherwise, go straight to the title screen.
            if(gSaveManager.GetRunCount() <= 1)
            {
                mVideoPlayer.Play("intro.bik", true, true, [this](){
                    new TitleScreen();
                });
            }
            else
            {
                new TitleScreen();
            }
        });
    });
    #else
    // For dev purposes: just load right into a desired timeblock and location.
    Loader::DoAfterLoading([this]() {
        Services::Get<GameProgress>()->SetTimeblock(Timeblock("110A"));
        LoadScene("RC1");
    });
    #endif

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

    //SheepScript* script = mAssetManager.LoadSheep("LBY110A");
    //script->Decompile();
    return true;
}

void GEngine::Shutdown()
{
    // Need to block main thread until threaded work is done.
    // Otherwise, we might get exceptions during shutdown if main thread exits before background threads.
    ThreadPool::Shutdown();
    Loader::Shutdown();

	// Delete all actors.
	for(auto& actor : mActors)
	{
		delete actor;
	}
	mActors.clear();

    // Shutdown any subsystems.
    mRenderer.Shutdown();
    mAudioManager.Shutdown();
    SDL_Quit();
}

void GEngine::Run()
{
    // We are running!
    mRunning = true;
    
    // Loop until not running anymore.
    while(mRunning)
    {
        PROFILER_BEGIN_FRAME();

		// Our main loop: inputs, updates, outputs.
        ProcessInput();
        Update();
        GenerateOutputs();
		
		// After frame is done, check whether we need a scene change.
		LoadSceneInternal();

        PROFILER_END_FRAME();
    }
}

void GEngine::Quit()
{
    mRunning = false;
}

void GEngine::ForceUpdate()
{
    Update(0.2f);
}

void GEngine::StartGame()
{
    Timeblock timeblock("110A");
    Services::Get<GameProgress>()->SetTimeblock(timeblock);

    TimeblockScreen* tbScreen = new TimeblockScreen();
    tbScreen->Show(timeblock, [this](){
        LoadScene("R25");
    });
}

void GEngine::ProcessInput()
{
    PROFILER_SCOPED(ProcessInput);

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
                // When a text input is active, controls for manipulating text and cursor pos.
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

                // Alt + Enter: toggle fullscreen mode
                else if(event.key.keysym.sym == SDLK_RETURN)
                {
                    SDL_Keymod modState = SDL_GetModState();
                    if((modState & KMOD_ALT) != 0)
                    {
                        mRenderer.ToggleFullscreen();
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
    
    // Quick quit for dev purposes.
    if(mInputManager.IsKeyPressed(SDL_SCANCODE_F4))
    {
        Quit();
    }
}

void GEngine::Update()
{
    PROFILER_SCOPED(Update);

    // Calculate delta time.
    static DeltaTimer deltaTimer;
    float deltaTime = deltaTimer.GetDeltaTime();

    // For debugging: press F5 to slow down passage of time significantly.
    if(mInputManager.IsKeyPressed(SDL_SCANCODE_F5))
    {
        deltaTime *= 0.25f;
    }

    if(!Loader::IsLoading())
    {
        // Update game logic.
        Update(deltaTime);
    }

    // Also update audio system (before or after game logic?)
    mAudioManager.Update(deltaTime);
    
    // Update video playback.
    mVideoPlayer.Update();

    // Update cursors.
    mCursorManager.Update(deltaTime);
	
	// Update debug visualizations.
	Debug::Update(deltaTime);

    // Update thread pool.
    ThreadUtil::RunFunctionsOnMainThread();
}

void GEngine::Update(float deltaTime)
{
    // Update the scene (kind of temp - just for convenience).
    // (or maybe actors should belong to a scene??? hmmmm)
    if(mScene != nullptr)
    {
        mScene->Update(deltaTime);
    }

    // Update all actors.
    for(size_t i = 0; i < mActors.size(); i++)
    {
        mActors[i]->Update(deltaTime);
    }

    // Delete any destroyed actors.
    DeleteDestroyedActors();

    // Update timers.
    Timers::Update(deltaTime);
}

void GEngine::GenerateOutputs()
{
    PROFILER_SCOPED(GenerateOutputs);
    if(!Loader::IsLoading())
    {
        mRenderer.Render();
    }
}

void GEngine::LoadSceneInternal()
{
    // Obviously no need to do anything if no load scene is defined.
	if(mSceneToLoad.empty()) { return; }

    // If background thread is performing loading, wait until that's done.
    // If the loader is loading the scene, we don't want to "double load" or something.
    if(Loader::IsLoading()) { return; }

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

    // Initiate scene load on background thread.
    Loader::Load([this]() {
        // Create the new scene.
        //TODO: Scene constructor should probably ONLY take a scene name.
        //TODO: Internally, we can call to GameProgress or whatnot as needed, but that's very GK3-specific stuff.
        mScene = new Scene(mSceneToLoad, Services::Get<GameProgress>()->GetTimeblock());

        // Load the scene - this is separate from constructor
        // b/c load operations may need to reference the scene itself!
        mScene->Load();

        // Clear scene load request.
        mSceneToLoad.clear();
    });

    // Once loading is done, init scene and away we go.
    Loader::DoAfterLoading([this](){
        mScene->Init();
    });
}

void GEngine::AddActor(Actor* actor)
{
    mActors.push_back(actor);
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
