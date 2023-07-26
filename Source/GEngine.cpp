#include "GEngine.h"

#include <cassert>

#include <SDL.h>

#include "ActionManager.h"
#include "AssetManager.h"
#include "Actor.h"
#include "CharacterManager.h"
#include "Console.h"
#include "ConsoleUI.h"
#include "CursorManager.h"
#include "Debug.h"
#include "DialogueManager.h"
#include "FileSystem.h"
#include "FootstepManager.h"
#include "GameProgress.h"
#include "GK3UI.h"
#include "InventoryManager.h"
#include "Loader.h"
#include "Localizer.h"
#include "LocationManager.h"
#include "Paths.h"
#include "Profiler.h"
#include "ReportManager.h"
#include "Renderer.h"
#include "SaveManager.h"
#include "Scene.h"
#include "TextInput.h"
#include "Timers.h"
#include "ThreadPool.h"
#include "Tools.h"
#include "VerbManager.h"
#include "VideoPlayer.h"
#include "Window.h"

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
	
	// Tell console to log itself to the "Console" report stream.
	gConsole.SetReportStream(&gReportManager.GetReportStream("Console"));

    // See if the demo barn is present. If so, we'll load the game in demo mode.
    mDemoMode = gAssetManager.LoadBarn("Gk3demo.brn");

    // For simplicity right now, let's just load all barns at once.
    if(!mDemoMode)
    {
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
            if(!gAssetManager.LoadBarn(barn))
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
    }

    // Initialize renderer.
    if(!gRenderer.Initialize())
    {
        return false;
    }

    // Init tools.
    Tools::Init();
    
    // Initialize audio.
    if(!gAudioManager.Initialize())
    {
        return false;
    }
    
    // Load cursors and use the default one to start.
    // Must happen after barn assets are loaded.
    gCursorManager.Init();
    gCursorManager.UseLoadCursor();
    
    // Create localizer.
    gLocalizer.Load("STRINGS.TXT");
    
	// Init verb manager.
    gVerbManager.Init();
	
	// Load character configs.
    gCharacterManager.Init();
	
	// Load floor configs.
    gFootstepManager.Init();

    // Init game progress.
    gGameProgress.Init();
	
	// Init inventory manager.
    gInventoryManager.Init();

    // Init location manager.
    gLocationManager.Init();
	
	// Create action manager.
    gActionManager.Init();
	
    // Create video player.
    gVideoPlayer.Initialize();
    
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

        // Demo mode only shows the title screen.
        if(mDemoMode)
        {
            gGK3UI.ShowTitleScreen();
        }
        else
        {
            // Play opening movie.
            gVideoPlayer.Play("Sierra.avi", true, true, [this](){

                // On first launch of game, show the intro movie before the title screen.
                // Otherwise, go straight to the title screen.
                if(gSaveManager.GetRunCount() <= 1)
                {
                    gVideoPlayer.Play("intro.bik", true, true, [this](){
                        gGK3UI.ShowTitleScreen();
                    });
                }
                else
                {
                    gGK3UI.ShowTitleScreen();
                }
            });
        }
    });
    #else
    // For dev purposes: just load right into a desired timeblock and location.
    Loader::DoAfterLoading([this]() {
        //gGameProgress.SetTimeblock(Timeblock("210A"));
        //LoadScene("R29");
        //gGameProgress.SetGameVariable("MaidCleaningPath210a", 8);
        gGameProgress.SetTimeblock(Timeblock("110A"));
        LoadScene("R25");
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

    // Shutdown tools.
    Tools::Shutdown();

    // Shutdown any subsystems.
    gRenderer.Shutdown();
    gAudioManager.Shutdown();
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
		
		// Check whether we need a scene change.
		LoadSceneInternal();

        // OK, this frame is done!
        ++mFrameNumber;
        PROFILER_END_FRAME();
    }
}

void GEngine::Quit()
{
    mRunning = false;
}

void GEngine::ForceUpdate()
{
    Update(10.0f);
}

void GEngine::LoadScene(const std::string& name, std::function<void()> callback)
{
    mSceneToLoad = name;
    mSceneLoadedCallback = callback;
}

void GEngine::UnloadScene(std::function<void()> callback)
{
    mUnloadScene = true;
    mSceneUnloadedCallback = callback;
}

void GEngine::StartGame()
{
    if(mDemoMode)
    {
        // Demo mode - load to Day 2, 12P at CSE.
        Timeblock timeblock("212P");
        gGameProgress.SetTimeblock(timeblock);

        gGK3UI.ShowTimeblockScreen(timeblock, 5.0f, [this](){
            LoadScene("CSE");
        });
    }
    else
    {
        // Not demo mode - load to Day 1, 10AM in R25.
        Timeblock timeblock("110A");
        gGameProgress.SetTimeblock(timeblock);

        gGK3UI.ShowTimeblockScreen(timeblock, 5.0f, [this](){
            LoadScene("R25");
        });
    }
}

void GEngine::ProcessInput()
{
    PROFILER_SCOPED(ProcessInput);

    // Update the input manager.
    // Retrieve input device states for us to use.
    gInputManager.Update();
    
    // We'll poll for events here. Catch the quit event.
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        // Send all events to tools to handle.
        Tools::ProcessEvent(event);

        switch(event.type)
        {
			case SDL_KEYDOWN:
			{
                // Ignore if tool overlay is eating keyboard inputs.
                if(Tools::EatingKeyboardInputs()) { break; }

                // When a text input is active, controls for manipulating text and cursor pos.
				if(event.key.keysym.sym == SDLK_BACKSPACE)
				{
					TextInput* textInput = gInputManager.GetTextInput();
					if(textInput != nullptr)
					{
						textInput->DeletePrev();
					}
				}
				else if(event.key.keysym.sym == SDLK_DELETE)
				{
					TextInput* textInput = gInputManager.GetTextInput();
					if(textInput != nullptr)
					{
						textInput->DeleteNext();
					}
				}
				else if(event.key.keysym.sym == SDLK_LEFT)
				{
					TextInput* textInput = gInputManager.GetTextInput();
					if(textInput != nullptr)
					{
						textInput->MoveCursorBack();
					}
				}
				else if(event.key.keysym.sym == SDLK_RIGHT)
				{
					TextInput* textInput = gInputManager.GetTextInput();
					if(textInput != nullptr)
					{
						textInput->MoveCursorForward();
					}
				}
				else if(event.key.keysym.sym == SDLK_HOME)
				{
					TextInput* textInput = gInputManager.GetTextInput();
					if(textInput != nullptr)
					{
						textInput->MoveCursorToStart();
					}
				}
				else if(event.key.keysym.sym == SDLK_END)
				{
					TextInput* textInput = gInputManager.GetTextInput();
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
                        Window::ToggleFullscreen();
                    }
                }
				break;
			}
			
			case SDL_TEXTINPUT:
			{
                // Ignore if tool overlay is eating keyboard inputs.
                if(Tools::EatingKeyboardInputs()) { break; }

				//TODO: Make sure not copy or pasting.
				TextInput* textInput = gInputManager.GetTextInput();
				if(textInput != nullptr)
				{
					textInput->Insert(event.text.text);
				}
				break;
			}

            case SDL_MOUSEWHEEL:
            {
                gInputManager.OnMouseWheelScroll(Vector2(static_cast<float>(event.wheel.x),
                                                         static_cast<float>(event.wheel.y)));
                break;
            }

            case SDL_WINDOWEVENT:
            {
                if(event.window.event == SDL_WINDOWEVENT_MOVED)
                {
                    // Let renderer know so it can process and take any action.
                    Window::OnPositionChanged();
                }
                break;
            }
				
            case SDL_QUIT:
            {
                Quit();
                break;
            }
        }
    }
    
    // Quick quit for dev purposes.
    if(gInputManager.IsKeyPressed(SDL_SCANCODE_F4))
    {
        Quit();
    }
}

void GEngine::Update()
{
    PROFILER_SCOPED(Update);

    // Calculate delta time.
    static DeltaTimer deltaTimer;
    float deltaTime = deltaTimer.GetDeltaTime() * mTimeMultiplier;

    if(!Loader::IsLoading())
    {
        // Update game logic.
        Update(deltaTime);
    }

    // Also update audio system (before or after game logic?)
    gAudioManager.Update(deltaTime);
    
    // Update video playback.
    gVideoPlayer.Update();

    // Update cursors.
    gCursorManager.Update(deltaTime);
	
	// Update debug visualizations.
	Debug::Update(deltaTime);

    // Update tools.
    Tools::Update();

    // Run any waiting functions on the main thread.
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
        // Clear screen.
        gRenderer.Clear();

        // Render the game scene.
        gRenderer.Render();

        // Render any tools over the game scene.
        Tools::Render();

        // Present the final result to screen.
        gRenderer.Present();
    }
}

void GEngine::LoadSceneInternal()
{
    // Obviously no need to do anything if no load scene is defined.
	if(mSceneToLoad.empty() && !mUnloadScene) { return; }

    // If background thread is performing loading, wait until that's done.
    // If the loader is loading the scene, we don't want to "double load" or something.
    if(Loader::IsLoading()) { return; }

    // Delete the current scene, if any.
    UnloadSceneInternal();
    mUnloadScene = false; // we did it

    // It's possible we just wanted to unload the current scene without loading a new scene.
    // So, early out in that case.
    if(mSceneToLoad.empty()) { return; }
    
    // Initiate scene load on background thread.
    Loader::Load([this]() {
        TIMER_SCOPED("GEngine::LoadSceneInternal::Load");
        // Create the new scene.
        //TODO: Scene constructor should probably ONLY take a scene name.
        //TODO: Internally, we can call to GameProgress or whatnot as needed, but that's very GK3-specific stuff.
        mScene = new Scene(mSceneToLoad, gGameProgress.GetTimeblock());

        // Load the scene - this is separate from constructor
        // b/c load operations may need to reference the scene itself!
        mScene->Load();

        // Clear scene load request.
        mSceneToLoad.clear();
    });

    // Once loading is done, init scene and away we go.
    Loader::DoAfterLoading([this](){
        mScene->Init();

        // Execute scene load callback, if any.
        if(mSceneLoadedCallback != nullptr)
        {
            mSceneLoadedCallback();
            mSceneLoadedCallback = nullptr;
        }
    });
}

void GEngine::UnloadSceneInternal()
{
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

    // Unload any assets scoped to just the current scene.
    gAssetManager.UnloadAssets(AssetScope::Scene);

    // Do callback if any.
    if(mSceneUnloadedCallback != nullptr)
    {
        std::function<void()> callback = mSceneUnloadedCallback;
        mSceneUnloadedCallback = nullptr;
        callback();
    }
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
