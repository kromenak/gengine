#include "GEngine.h"

#include <cassert>

#include <SDL.h>

#include "ActionManager.h"
#include "AssetManager.h"
#include "CharacterManager.h"
#include "Clipboard.h"
#include "Console.h"
#include "ConsoleUI.h"
#include "CursorManager.h"
#include "Debug.h"
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
#include "SceneManager.h"
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

    // Init asset manager.
    gAssetManager.Init();

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
            TIMER_SCOPED_VAR(barn.c_str(), barnTimer);
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

    // Init tools.
    // Must happen before renderer b/c IMGUI renderer init depends on IMGUI context being created.
    Tools::Init();

    // Initialize renderer. Depends on AssetManager being initialized.
    // After this function executes, the game window will be visible.
    if(!gRenderer.Initialize())
    {
        return false;
    }
    
    // Initialize audio.
    if(!gAudioManager.Initialize())
    {
        return false;
    }

    // Init input system.
    gInputManager.Init();
    
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

    // INIT DONE! Move on to starting the game flow.
    // Non-debug: do the full game presentation - company logos, intro movie, title screen.
    //#define FORCE_TITLE_SCREEN
    #if defined(NDEBUG) || defined(FORCE_TITLE_SCREEN)
    // In demo mode, wait for async stuff to load and then show the title screen.
    if(mDemoMode)
    {
        ShowTitleScreen();
    }
    else // not demo
    {
        ShowOpeningMovies();
    }
    #else
    // For dev purposes: just load right into a desired timeblock and location.
    Loader::DoAfterLoading([]() {
        gGameProgress.SetTimeblock(Timeblock("110A"));
        gSceneManager.LoadScene("R25");
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

    // Shutdown scene manager (unloads scene and deletes all actors).
    gSceneManager.Shutdown();
    
    // Even though asset manager is initialized first...
    // We want to shut it down earlier b/c its assets may need to destroy data in the rendering/audio systems.
    gAssetManager.Shutdown();

    // Shutdown renderer.
    gRenderer.Shutdown();

    // Shutdown tools.
    Tools::Shutdown();

    // Shutdown audio system.
    gAudioManager.Shutdown();

    // Shutdown SDL.
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
        gSceneManager.UpdateLoading();

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

void GEngine::StartGame() const
{
    if(mDemoMode)
    {
        // Demo mode - load to Day 2, 12P at CSE.
        Timeblock timeblock("212P");
        gGameProgress.SetTimeblock(timeblock);

        gGK3UI.ShowTimeblockScreen(timeblock, 5.0f, [](){
            gSceneManager.LoadScene("CSE");
        });
    }
    else
    {
        // Not demo mode - load to Day 1, 10AM in R25.
        Timeblock timeblock("110A");
        gGameProgress.SetTimeblock(timeblock);

        gGK3UI.ShowTimeblockScreen(timeblock, 5.0f, [](){
            gSceneManager.LoadScene("R25");
        });
    }
}

void GEngine::ShowOpeningMovies()
{
    // Play opening movie.
    gVideoPlayer.Play("Sierra.avi", true, true, [this](){

        // On first launch of game, show the intro movie before the title screen.
        // Otherwise, go straight to the title screen.
        if(gSaveManager.GetRunCount() <= 1)
        {
            gVideoPlayer.Play("intro.bik", true, true, [this](){
                ShowTitleScreen();
            });
        }
        else
        {
            ShowTitleScreen();
        }
    });
}

void GEngine::ShowTitleScreen()
{
    // Make sure all background loading completes before showing title screen.
    // (not 100% technically necessary, this is just a good "airlock" to ensure all stuff is loaded)
    Loader::DoAfterLoading([](){
        gGK3UI.ShowTitleScreen();
    });
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

                // Ctrl + C: Copy text from text input.
                // Ctrl + X: Cut text from text input.
                // Ctrl + V: Past text from text input.
				else if(event.key.keysym.sym == SDLK_c)
                {
                    SDL_Keymod modState = SDL_GetModState();
                    if((modState & KMOD_CTRL) != 0)
                    {
                        TextInput* textInput = gInputManager.GetTextInput();
                        if(textInput != nullptr)
                        {
                            Clipboard::SetClipboardText(textInput->GetText().c_str());
                        }
                    }
                }
				else if(event.key.keysym.sym == SDLK_x)
                {
                    SDL_Keymod modState = SDL_GetModState();
                    if((modState & KMOD_CTRL) != 0)
                    {
                        TextInput* textInput = gInputManager.GetTextInput();
                        if(textInput != nullptr)
                        {
                            Clipboard::SetClipboardText(textInput->GetText().c_str());
                            textInput->SetText("");
                        }
                    }
                }
				else if(event.key.keysym.sym == SDLK_v)
                {
                    SDL_Keymod modState = SDL_GetModState();
                    if((modState & KMOD_CTRL) != 0)
                    {
                        TextInput* textInput = gInputManager.GetTextInput();
                        if(textInput != nullptr)
                        {
                            textInput->Insert(Clipboard::GetClipboardText());
                        }
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
    float deltaTime = deltaTimer.GetDeltaTime();

    // In debug, calculate a running average FPS and display it in the window's title bar.
    #if defined(DEBUG)
    {
        // Current FPS is inverse of delta time.
        float currentFps = 1.0f / deltaTime;

        // We can utilize a moving average calculation. (https://en.wikipedia.org/wiki/Moving_average#Cumulative_average)
        // "cumulative average equals previous cumulative average, times n, plus new data, all divided by n+1"
        // (in our case, n = frame number counter)
        static float averageFps = 0.0f;
        averageFps = ((averageFps * static_cast<float>(mFrameNumber)) + currentFps) / static_cast<float>(mFrameNumber + 1);

        // Set title to show updated value.
        Window::SetTitle(StringUtil::Format("Gabriel Knight 3 (%.2f FPS)", averageFps).c_str());
    }
    #endif

    if(!Loader::IsLoading())
    {
        // Update game logic.
        Update(deltaTime * mTimeMultiplier);
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
    // Update the scene.
    gSceneManager.Update(deltaTime);

    // Update location system.
    gLocationManager.Update();
    
    // Update timers.
    Timers::Update(deltaTime);
}

void GEngine::GenerateOutputs()
{
    PROFILER_SCOPED(GenerateOutputs);

    if(gSceneManager.IsSceneLoading()) { return; }

    // Clear screen.
    gRenderer.Clear();

    // Render the game scene.
    gRenderer.Render();

    // Render any tools over the game scene.
    Tools::Render();

    // Present the final result to screen.
    gRenderer.Present();
}
