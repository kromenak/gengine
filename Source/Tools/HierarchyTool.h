//
// Clark Kromenaker
//
// A tool that shows the current scene hierarchy.
//
#pragma once

class Actor;

class HierarchyTool
{
public:
    void Render(bool& toolActive);

private:
    // The actor in the hierarchy that is currently selected.
    Actor* mSelectedActor = nullptr;

    void AddTreeNodeForActor(Actor* actor);
};