//
// Clark Kromenaker
//
// A tool that displays debug info about raycasts.
//
#pragma once
struct RaycastHit;

class RaycastTool
{
public:
    static void StartFrame();
    static void LogRaycastHit(const RaycastHit& raycastHit);

    void Render(bool& toolActive);

private:

};