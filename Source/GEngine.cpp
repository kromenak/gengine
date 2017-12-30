//
//  GEngine.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#include "GEngine.h"
#include "SDL/SDL.h"

#include "Services.h"
#include "Actor.h"

#include "Sheep/SheepDriver.h"
#include "Barn/BarnFile.h"

#include "CameraComponent.h"
#include "MeshComponent.h"

#include "Mesh.h"

extern GLfloat triangle_vertices[];
extern GLfloat triangle_colors[];
extern GLfloat cube_vertices[];
extern GLfloat cube_colors[];
extern GLushort cube_elements[];

std::vector<Actor*> GEngine::mActors;

GEngine::GEngine() : mRunning(false)
{
    
}

bool GEngine::Initialize()
{
    // Initialize renderer.
    if(!mRenderer.Initialize())
    {
        return false;
    }
    Services::SetRenderer(&mRenderer);
    
    // Initialize audio.
    if(!mAudio.Initialize())
    {
        return false;
    }
    
    // Initialize input.
    Services::SetInput(&mInputManager);
    
    //Sheep::Driver driver;
    //driver.Parse("/Users/Clark/Dropbox/GK3/Assets/test.shp");
    
    //BarnFile barnFile("/Users/Clark/Dropbox/GK3/Data/core.brn");
    //barnFile.WriteToFile("E18LCJ44QR1.YAK");
    
    //BarnFile barnFile("/Users/Clark/Dropbox/GK3/Data/ambient.brn");
    //barnFile.WriteToFile("R33SNEAKPEEK.WAV");
    
    //BarnFile barnFile("/Users/Clark/Dropbox/GK3/Data/day1.brn");
    //barnFile.WriteToFile("CASH.BMP");
    
    //BarnFile barnFile("day1.brn");
    //barnFile.WriteToFile("CASH.BMP");
    
    //BarnFile barnFile("/Users/Clark/Dropbox/GK3/Data/day3.brn");
    //barnFile.WriteToFile("TE4SUNBUTTON.BMP");
    
    mAssetManager.AddSearchPath("Assets/");
    mAssetManager.LoadBarn("day1.brn");
    //mAssetManager.LoadBarn("core.brn");
    //mAssetManager.LoadBarn("ambient.brn");
    
    Model* model = mAssetManager.LoadModel("SYRUPPACKET.MOD");
    //Model* model = mAssetManager.LoadModel("TAX.MOD");
    //Model* model = mAssetManager.LoadModel("R25HANGER.MOD");
    
    //Audio* audio = mAssetManager.LoadAudio("HALLS3.WAV");
    //audio->WriteToFile();
    
    //mAudio.Play(*audio);
    
    //SDL_Log(SDL_GetBasePath());
    //SDL_Log(SDL_GetPrefPath("Test", "GK3"));
    
    Texture* topTex = mAssetManager.LoadTexture("SYRUPTOP.BMP");
    Texture* botTex = mAssetManager.LoadTexture("SYRUPBOT.BMP");
    Texture* sidTex = mAssetManager.LoadTexture("SYRUPSIDE.BMP");
    
    // Camera example.
    Actor* camActor = new Actor();
    camActor->SetPosition(Vector3(0.0f, 0.0f, 2.0f));
    camActor->AddComponent(new CameraComponent(camActor));
    
    // Mesh example.
    //Mesh* mesh = new Mesh();
    //mesh->SetPositions(triangle_vertices, 9);
    //mesh->SetColors(triangle_colors, 12);
    //mesh->SetPositions(cube_vertices, 24);
    //mesh->SetColors(cube_colors, 32);
    //mesh->SetIndexes(cube_elements, 36);
    
    Actor* meshActor = new Actor();
    meshActor->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    //meshActor->SetScale(Vector3(5.0f, 5.0f, 5.0f));
    
    MeshComponent* meshComponent = new MeshComponent(meshActor);
    //meshComponent->SetMesh(mesh);
    meshComponent->SetModel(model);
    //meshComponent->AddTexture(sidTex);
    //meshComponent->AddTexture(sidTex);
    //meshComponent->AddTexture(sidTex);
    //meshComponent->AddTexture(topTex);
    //meshComponent->AddTexture(botTex);

    meshActor->AddComponent(meshComponent);
    return true;
}

void GEngine::Shutdown()
{
    // Delete all actors and clear actor list.
    for(auto& actor : mActors)
    {
        delete actor;
    }
    mActors.clear();
    
    mRenderer.Shutdown();
    mAudio.Shutdown();
    
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
        ProcessInput();
        Update();
        GenerateOutput();
    }
}

void GEngine::Quit()
{
    mRunning = false;
}

void GEngine::ProcessInput()
{
    // We'll poll for events here. Catch the quit event.
    //TODO: Should this be moved to InputManager?
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
    
    // Update the input manager.
    // Retrieve input device states for us to use.
    mInputManager.Update();
    
    // Quit game on escape press for now.
    if(mInputManager.IsPressed(SDL_SCANCODE_ESCAPE))
    {
        Quit();
    }
}

void GEngine::Update()
{
    // Tracks the next "GetTicks" value that is acceptable to perform an update.
    static int nextTicks = 0;
    
    // Tracks the last ticks value each time we run this loop.
    static Uint32 lastTicks = 0;
    
    // Limit the FPS to about 60. nextTicks is always +16 at start of frame.
    // If we get here again and not 16 frames have passed, we wait.
    while(!SDL_TICKS_PASSED(SDL_GetTicks(), nextTicks)) { }
    
    // Get current ticks and save next ticks as +16. Limits FPS to ~60.
    Uint32 currentTicks = SDL_GetTicks();
    nextTicks = currentTicks + 16;
    
    // Calculate the time delta.
    float deltaTime = ((float)(currentTicks - lastTicks)) * 0.001f;
    lastTicks = currentTicks;
    
    // Limit the time delta to, at most, 0.05 seconds.
    if(deltaTime > 0.05f) { deltaTime = 0.05f; }
    
    // Update all actors.
    for(auto& actor : mActors)
    {
        actor->Update(deltaTime);
    }
}

void GEngine::GenerateOutput()
{
    mRenderer.Clear();
    mRenderer.Render();
    mRenderer.Present();
}

void GEngine::AddActor(Actor *actor)
{
    mActors.push_back(actor);
}

void GEngine::RemoveActor(Actor *actor)
{
    auto it = std::find(mActors.begin(), mActors.end(), actor);
    if(it != mActors.end())
    {
        mActors.erase(it);
    }
}
