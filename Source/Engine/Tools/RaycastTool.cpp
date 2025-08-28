#include "RaycastTool.h"

#include <cstdio>
#include <imgui.h>
#include <vector>

#include "Actor.h"
#include "Collisions.h"
#include "Queue.h"

namespace
{
    bool paused = false;

    struct RaycastFrame
    {
        std::vector<RaycastHit> hits;
    };
    Queue<RaycastFrame, 100> frames;
}

/*static*/ void RaycastTool::StartFrame()
{
    if(!paused)
    {
        if(frames.Full())
        {
            frames.Pop();
        }
        frames.Emplace();
    }
}

/*static*/ void RaycastTool::LogRaycastHit(const RaycastHit& raycastHit)
{
    if(!paused)
    {
        if(!frames.Empty())
        {
            frames.Back().hits.push_back(raycastHit);
        }
    }
}

void RaycastTool::Render(bool& toolActive)
{
    if(!toolActive) { return; }

    // Sets the default size of the window on first open.
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);

    // Begin the window. Early out if collapsed.
    if(ImGui::Begin("Raycasts", &toolActive))
    {
        if(paused && ImGui::Button("Resume"))
        {
            paused = false;
        }
        else if(!paused && ImGui::Button("Pause"))
        {
            paused = true;
        }

        if(paused)
        {
            ImGui::SameLine();
            if(ImGui::Button("Snapshot"))
            {

            }
        }

        if(!frames.Empty())
        {
            ImGui::Text("Raycasts This Frame:");
            ImGui::Indent();

            std::sort(frames.Back().hits.begin(), frames.Back().hits.end(), [](const RaycastHit& a, const RaycastHit& b){
                return a.t < b.t;
            });

            for(RaycastHit& hit : frames.Back().hits)
            {
                if(hit.actor != nullptr)
                {
                    ImGui::Text("%s (%s) - %f", hit.name.c_str(), hit.actor->GetTypeName(), hit.t);
                }
                else
                {
                    ImGui::Text("%s - %f", hit.name.c_str(), hit.t);
                }
            }
            ImGui::Unindent();
        }
    }

    // End window.
    ImGui::End();
}