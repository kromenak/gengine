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
#include "InputManager.h"
#include "InventoryManager.h"
#include "Loader.h"
#include "Localizer.h"
#include "LocationManager.h"
#include "Paths.h"
#include "PersistState.h"
#include "Profiler.h"
#include "ReportManager.h"
#include "Renderer.h"
#include "SaveManager.h"
#include "SceneManager.h"
#include "TextInput.h"
#include "ThreadPool.h"
#include "ThreadUtil.h"
#include "Tools.h"
#include "UICanvas.h"
#include "VerbManager.h"
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
        std::vector<std::string> requiredBarns = {
            "ambient.brn",
            "common.brn",
            "core.brn",
            "day1.brn",
            "day2.brn",
            "day3.brn",
            "day23.brn",
            "day123.brn"
        };
        for(auto& barn : requiredBarns)
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

        // Official localized versions of the game also came with a Barn called "override.brn". This barn contains assets that override ordinary assets.
        // Try to load this, but since it's optional, it shouldn't show an error message.
        gAssetManager.LoadBarn("override.brn", BarnSearchPriority::High);

        // Also check if any other Barns are specified in the INI file to load.
        Config* config = gAssetManager.LoadConfig("GK3.ini");
        if(config != nullptr)
        {
            std::string customBarns = config->GetString("Custom Barns", "");
            if(!customBarns.empty())
            {
                // Multiple paths are separated by semicolons.
                std::vector<std::string> customBarnNames = StringUtil::Split(customBarns, ';');
                for(auto& barnName : customBarnNames)
                {
                    printf("Load barn %s\n", barnName.c_str());
                    gAssetManager.LoadBarn(barnName, BarnSearchPriority::High);
                }
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

    // Create console UI - this persists for the entire game.
    ConsoleUI* consoleUI = new ConsoleUI(false);
    consoleUI->SetIsDestroyOnLoad(false);

    // Init debug system.
    Debug::Init();

    // Decide whether the app will always stay active in the background.
    // For debug builds, this is almost always useful - do it.
    // For release builds, we usually don't want this, unless a debug flag was set in INI file.
    #if defined(DEBUG)
    mAlwaysActive = true;
    #else
    mAlwaysActive = Debug::GetFlag("GEngine AlwaysActive");
    #endif

    // INIT DONE! Move on to starting the game flow.
    // Non-debug: do the full game presentation - company logos, intro movie, title screen.
    //#define FORCE_TITLE_SCREEN
    #if defined(NDEBUG) || defined(FORCE_TITLE_SCREEN)
    // In demo mode, show the title screen. Normal game shows opening movies.
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
        // First, poll inputs and pump OS events.
        // This should be done even if app is not focused - we need to detect "app focus" event for one.
        ProcessInput();

        // The remainder only happen if app is focused OR if we want app to stay focused in background.
        if(mApplicationFocused || mAlwaysActive)
        {
            // Update application state (including game state).
            Update();

            // Render outputs to reflect updated game state.
            GenerateOutputs();

            // Check whether we need a scene change.
            gSceneManager.UpdateLoading();

            // Frame is done, do any save or loads actions now.
            // First, see if a quick save or quick load are desired.
            gSaveManager.HandleQuickSaveQuickLoad();

            // Then, process any save/load that was registered this frame (including the very recent quick save/load).
            gSaveManager.HandlePendingSavesAndLoads();

            // If F11 is pressed, take a screenshot and save it to file.
            if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_F11))
            {
                gRenderer.TakeScreenshotToFile();
            }

            // OK, this frame is done!
            ++mFrameNumber;
        }
        PROFILER_END_FRAME();
    }
}

void GEngine::Quit()
{
    mRunning = false;
}

void GEngine::StartGame() const
{
    if(mDemoMode)
    {
        // Demo mode - load to Day 2, 12P at CSE.
        Timeblock timeblock("212P");
        gGameProgress.SetTimeblock(timeblock);

        gGK3UI.ShowTimeblockScreen(timeblock, 5.0f, false, [](){
            gSceneManager.LoadScene("CSE");
        });
    }
    else
    {
        // Not demo mode - load to Day 1, 10AM in R25.
        Timeblock timeblock("110A");
        gGameProgress.SetTimeblock(timeblock);

        gGK3UI.ShowTimeblockScreen(timeblock, 5.0f, false, [](){
            gSceneManager.LoadScene("R25");
        });
    }
}

void GEngine::OnPersist(PersistState& ps)
{
    ps.Xfer(PERSIST_VAR(mFrameNumber));
    ps.Xfer(PERSIST_VAR(mTimeMultiplier));
    ps.Xfer(PERSIST_VAR(mDemoMode));
}

void GEngine::ShowOpeningMovies()
{
    // Play opening movie.
    gGK3UI.GetVideoPlayer()->Play("Sierra.avi", true, true, [this](){

        // On first launch of game, show the intro movie before the title screen.
        // Otherwise, go straight to the title screen.
        if(gSaveManager.GetRunCount() <= 1)
        {
            gGK3UI.GetVideoPlayer()->Play("intro.bik", true, true, [this](){
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

    // If the UI system can receive mouse input, update UI mouse input.
    if(!Tools::EatingKeyboardInputs() && !gInputManager.IsMouseLocked() && (!gActionManager.IsActionPlaying() || mAllowInteractDuringActions) && !Loader::IsLoading())
    {
        UICanvas::UpdateMouseInput();
    }

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

                        // On some platforms (Linux for one), it's likely that the OS doesn't TRULY enter "exclusive fullscreen" mode.
                        // Instead, it just emulates the fullscreen request by making a fullscreen desktop window.
                        // In this case, we need to also change the game's resolution, or else we'll have a tiny game in a big window.
                        if(Window::GetFullscreenMode() != Window::Mode::FullscreenExclusive)
                        {
                            gRenderer.ChangeResolution(Window::GetResolution());
                        }
                    }
                }

                /*
                else if(event.key.keysym.sym == SDLK_u)
                {
                    printf("UNLOAD START\n");
                    gSceneManager.UnloadScene([]() {
                        printf("UNLOAD COMPLETE\n");
                    });
                }
                else if(event.key.keysym.sym == SDLK_l)
                {
                    printf("LOAD\n");
                    gSceneManager.LoadScene(gLocationManager.GetLocation(), [](){
                        printf("LOAD COMPLETE\n");
                    });
                }
                */
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
                else if(event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                {
                    // Keep track of wether the application is focused.
                    mApplicationFocused = true;

                    // Resume audio playback.
                    if(!mAlwaysActive)
                    {
                        gAudioManager.Resume();
                    }
                }
                else if(event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                {
                    // App is no longer focused.
                    mApplicationFocused = false;

                    // Unless we want to stay active when not focused, pause audio playback.
                    if(!mAlwaysActive)
                    {
                        gAudioManager.Pause();
                    }
                }
                break;
            }

            // Called when user hits X in corner of window.
            case SDL_QUIT:
            {
                gGK3UI.ShowQuitPopup();
                break;
            }
        }
    }

    // Quick quit for dev purposes.
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_F4))
    {
        Quit();
    }
}

void GEngine::Update()
{
    PROFILER_SCOPED(Update);

    // Calculate delta time.
    float deltaTime = mDeltaTimer.GetDeltaTimeWithFpsThrottle(60, 0.05f);
    //printf("%f ms\n", deltaTime);

    // In debug, calculate an average FPS and display it in the window's title bar.
    {
        // Current FPS is inverse of delta time.
        float currentFps = 1.0f / deltaTime;

        // We'll do an average by saving the last X data points and calculating the average.
        const size_t kDataSetSize = 60;
        static float sFpsDataSet[kDataSetSize] = { 0 };
        static size_t sFpsDataSetIndex = 0;

        // Put FPS in data set.
        sFpsDataSet[sFpsDataSetIndex] = currentFps;

        // Calculate average FPS. This uses a weighted average, so recent readings have a higher weight than older readings.
        // This gives better output than a straight average, where one slow frame makes the average drop to a constant value until it exits the data set.
        size_t weight = kDataSetSize;
        size_t weightsSum = 0;
        size_t currentIndex = sFpsDataSetIndex; // start at most recent FPS sampled
        float sum = 0.0f;
        for(int i = 0; i < kDataSetSize; ++i)
        {
            sum += sFpsDataSet[currentIndex] * static_cast<float>(weight);
            weightsSum += weight;
            --weight;
            currentIndex = (currentIndex == 0 ? kDataSetSize - 1 : currentIndex - 1);
        }
        float averageFps = sum / static_cast<float>(weightsSum);

        // Increment data set index (with wraparound) for next frame.
        ++sFpsDataSetIndex;
        sFpsDataSetIndex %= kDataSetSize;

        // NOTE: this is cool and efficient, but it is too conservative; if FPS ever dips, the displayed value will never again reach 60.
        // We can utilize a moving average calculation. (https://en.wikipedia.org/wiki/Moving_average#Cumulative_average)
        // "cumulative average equals previous cumulative average, times n, plus new data, all divided by n+1" (in our case, n = frame number counter)
        //static float averageFps = 0.0f;
        //averageFps = ((averageFps * static_cast<float>(mFrameNumber + 1)) + currentFps) / static_cast<float>(mFrameNumber + 2);

        // Set title to show updated value.
        Window::SetTitle(StringUtil::Format("Gabriel Knight 3 (%.2f FPS)", averageFps).c_str());
    }

    if(!Loader::IsLoading())
    {
        // Update game logic.
        UpdateGameWorld(deltaTime * mTimeMultiplier);

        // Perform any pending action skips.
        gActionManager.PerformPendingActionSkip();
    }

    // Update cursor based on high-level game state.
    // If loading or playing an action, the load/wait cursors are higher priority than anything else.
    if(Loader::IsLoading())
    {
        gCursorManager.UseLoadCursor();
    }
    else if(gActionManager.IsActionPlaying() && !mAllowInteractDuringActions)
    {
        gCursorManager.UseWaitCursor();
    }
    else if(gCursorManager.IsUsingWaitCursor())
    {
        gCursorManager.UseDefaultCursor();
    }

    // Update location system.
    gLocationManager.Update();

    // Also update audio system (before or after game logic?)
    gAudioManager.Update(deltaTime);

    // Update cursors.
    gCursorManager.Update(deltaTime);

    // Update debug visualizations.
    Debug::Update(deltaTime);

    // Update tools.
    Tools::Update();

    // Run any waiting functions on the main thread.
    ThreadUtil::RunFunctionsOnMainThread();
}

void GEngine::UpdateGameWorld(float deltaTime)
{
    // Update the scene.
    gSceneManager.Update(deltaTime);

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