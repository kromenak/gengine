#include "Bridge.h"

#include "ActionManager.h"
#include "Animation.h"
#include "Animator.h"
#include "AssetManager.h"
#include "CursorManager.h"
#include "GameCamera.h"
#include "GameProgress.h"
#include "GKActor.h"
#include "GKObject.h"
#include "InputManager.h"
#include "PersistState.h"
#include "SceneManager.h"

namespace
{
    // The XZ position at which the bridge puzzle starts. Gabe needs to walk near here to start it.
    const Vector2 kStartPuzzlePosition = Vector2(-78.0f, -220.0f);

    // How close Gabe needs to get to the start point to trigger the puzzle.
    const float kStartPuzzleDistanceSq = 20.0f * 20.0f;

    // The "origin" in world space of the grid the tiles are laid out on.
    const Vector3 kOrigin(-125.0f, 0.0f, -180.0f);

    // The size of a single tile.
    const float kTileSize = 45.0f;
}

Bridge::Bridge()
{
    // Cache Gabe reference.
    mGabeActor = gSceneManager.GetScene()->GetEgo();

    // Cache tile references.
    std::string tileModelName = "te5sq01";
    std::string glintAnimName = "TE5GLINT01.ANM";
    std::string glowAnimName = "TE5GLOW01.ANM";
    for(int i = 0; i < kTileCount; ++i)
    {
        tileModelName[6] = '1' + i;
        mTiles[i].tileActor = gSceneManager.GetScene()->GetSceneObjectByModelName(tileModelName);

        glintAnimName[9] = '1' + i;
        mTiles[i].glintAnim = gAssetManager.LoadAnimation(glintAnimName, AssetScope::Scene);

        glowAnimName[8] = '1' + i;
        mTiles[i].glowAnim = gAssetManager.LoadAnimation(glowAnimName, AssetScope::Scene);
    }

    // Layout the tiles.
    SetTilePosition(0, 1, 0);
    SetTilePosition(1, 2, 1);
    SetTilePosition(2, 0, 2);
    SetTilePosition(3, 0, 4);
    SetTilePosition(4, 1, 5);
    SetTilePosition(5, 2, 7);
    SetTilePosition(6, 0, 8);
    SetTilePosition(7, 0, 9);
    SetTilePosition(8, 1, 10);
}

void Bridge::OnPersist(PersistState& ps)
{
    ps.Xfer(PERSIST_VAR(mGabeStartedPuzzle));
    ps.Xfer(PERSIST_VAR(mGabeTileIndex));

    for(int i = 0; i < kTileCount; ++i)
    {
        ps.Xfer<TileState, int>("", mTiles[i].state);
        ps.Xfer("", mTiles[i].stateTimer);
        ps.Xfer("", mTiles[i].stateDuration);
        ps.Xfer("", mTiles[i].animFrame);
    }
}

void Bridge::OnUpdate(float deltaTime)
{
    // Check if Gabe is standing at the start of the puzzle.
    if(AtPuzzleStart())
    {
        // If haven't played the start-of-puzzle cutscene yet, do that.
        if(!mGabeStartedPuzzle)
        {
            mGabeStartedPuzzle = true;
            gActionManager.ExecuteSheepAction("wait CallSheep(\"TE5\", \"PreFirstJump$\")");
        }

        // Start the puzzle by showing just the first tile to jump to.
        if(mTiles[0].state == TileState::Off)
        {
            GlintTile(0);
        }
    }

    // When landing from a jump, lerp Gabe towards the center of tile he's on.
    // Without this, Gabe "snaps" to the tile center, which can look a bit jarring.
    if(mLanding)
    {
        //TODO: Using deltaTime here is a bit of a HACK - more correct would be to use a timer with duration of the land anim perhaps?
        mGabeActor->SetPosition(Vector3::Lerp(mGabeActor->GetPosition(), mJumpToPosition, deltaTime));
    }

    UpdateTiles(deltaTime);

    // Update custom interaction for the bridge puzzle.
    UpdateInteract();
}

bool Bridge::AtPuzzleStart() const
{
    Vector3 gabePos = mGabeActor->GetPosition();
    Vector2 gabeXZPos(gabePos.x, gabePos.z);
    return (gabeXZPos - kStartPuzzlePosition).GetLengthSq() <= kStartPuzzleDistanceSq;
}

void Bridge::Die(bool duringJump)
{
    std::string actionStr = duringJump ?
        "wait CallSheep(\"TE5\", \"BishopFallDie$\")" :
        "wait CallSheep(\"TE5\", \"FallDie$\")";
    gActionManager.ExecuteSheepAction(actionStr, [this](const Action* action){
        ResetOnDeath();
    });
}

void Bridge::ResetOnDeath()
{
    mGabeTileIndex = -1;
    mJumping = false;
    mJumpTileIndex = -1;

    // This fixes a rather complex bug that stops Gabe from appearing after a death retry. Here's a summary of the problem:
    // 1) Gabe gets reset to the start pos after death animation is finished.
    // 2) The death animation is finished after Animator detects all frames have played.
    // 3) However, the death vertex animation length is LONGER than the animation length, so it keeps playing.
    // 4) When resetting Gabe, we sample his walk anim so he will appear in the expected spot with a normal (non-death pose). This is triggered by the anim finishing.
    // 5) However, because the death vertex animation is still playing, it OVERWRITES the sampled walk anim. Gabe doesn't appear in the correct spot.
    // There may be a better systematic fix for this to try in the future, but for now, simply stopping all animations here resolves the problem.
    mGabeActor->StopAnimation(nullptr);
}

void Bridge::SetTilePosition(int index, int x, int y)
{
    if(index >= 0 && index < kTileCount)
    {
        Vector3 pos = kOrigin + (Vector3::UnitX * kTileSize * x) + (Vector3::UnitZ * kTileSize * y);
        mTiles[index].tileActor->SetPosition(pos);
        mTiles[index].gridPos.x = x;
        mTiles[index].gridPos.y = y;
    }
}

void Bridge::GlintTile(int index)
{
    if(index >= 0 && index < kTileCount)
    {
        mTiles[index].state = TileState::Glinting;
        mTiles[index].stateTimer = 0.0f;
        mTiles[index].stateDuration = mTiles[index].glintAnim->GetDuration();
        mTiles[index].animFrame = -1;
    }
}

void Bridge::GlowTile(int index)
{
    if(index >= 0 && index < kTileCount)
    {
        mTiles[index].state = TileState::Glowing;
        mTiles[index].stateTimer = 0.0f;
        mTiles[index].stateDuration = mTiles[index].glowAnim->GetDuration();
        mTiles[index].animFrame = -1;
    }
}

void Bridge::SleepTile(int index, float sleepTime)
{
    if(index >= 0 && index < kTileCount)
    {
        mTiles[index].state = TileState::Sleeping;
        mTiles[index].stateTimer = 0.0f;
        mTiles[index].stateDuration = sleepTime;
    }
}

void Bridge::StartTilePattern()
{
    // Set each tile to sleeping with different durations.
    // This creates the tile pattern to navigate from the start to the end of the puzzle.

    // This is surely not identical to the pattern in the original game, but it seems pretty close.
    //TODO: Could be made better?
    SleepTile(1, 5.0f);
    SleepTile(2, 3.0f);
    SleepTile(3, 6.0f);
    SleepTile(4, 3.2f);
    SleepTile(5, 0.5f);
    SleepTile(6, 2.0f);
    SleepTile(7, 0.01f);
    SleepTile(8, 0.8f);
}

void Bridge::JumpToTile(int index)
{
    // What we'll do here is figure out which turn/jump/land anims to use depending on where we are, and where we need to jump to.
    Animation* turnAnim = nullptr;
    Animation* jumpAnim = nullptr;
    Animation* landAnim = gAssetManager.LoadAnimation("GABTE5STAND", AssetScope::Scene);
    Animation* turnBackAnim = nullptr;

    // Figure out what grid pos is being jumped to.
    Vector2 toGridPos;
    Vector3 toWorldPosition;
    if(index >= 0 && index < kTileCount)
    {
        toGridPos = mTiles[index].gridPos;
        toWorldPosition = mTiles[index].tileActor->GetPosition();
    }
    else if(index == -1) // -1 means "start of puzzle" - the spot before Gabe jumps to the first tile
    {
        toGridPos = Vector2(1, -1);
        toWorldPosition = Vector3(kStartPuzzlePosition.x, 0.0f, kStartPuzzlePosition.y);
    }
    else if(index == 10) // 10 means "end of puzzle" - the opposite end of the bridge
    {
        toGridPos = Vector2(1, 11);
        toWorldPosition = mTiles[8].tileActor->GetPosition() + Vector3::UnitZ * kTileSize;
    }

    // Move the "to" position up just a bit so Gabe's boots aren't embedded inside the tile.
    const float kTileHeight = 2.0f;
    toWorldPosition.y += kTileHeight;
    mJumpToPosition = toWorldPosition;

    // First, figure out if we need to turn to face the next tile.
    if(mGabeTileIndex == -1)
    {
        // When not on the board, you're only allowed to jump to tile index 0.
        if(index != 0) { return; }

        // In this case, the jump anim is hardcoded, and no turns are needed.
        jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP01SQ", AssetScope::Scene);

        // Gabe's heading also immediately snaps to look straight ahead.
        mGabeActor->SetHeading(Heading::FromDegrees(0.0f));

        // Move Gabe up slightly so the animation looks better (boots actually land on the tile, not in it).
        // Doing this during camera cut hides any jankiness.
        Vector3 pos = mGabeActor->GetPosition();
        pos.y += kTileHeight;
        mGabeActor->SetPosition(pos);

        // When jumping to first tile, the camera is set to a certain position overlooking the puzzle.
        gSceneManager.GetScene()->SetCameraPosition("TILES");
    }
    else if(mGabeTileIndex == 9 && index == 10) // this means you are about to finish the puzzle
    {
        jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP01SQ", AssetScope::Scene);
    }
    else if(mGabeTileIndex >= 0 && mGabeTileIndex < kTileCount) // normal jump in middle of puzzle
    {
        // Figure out x/y change of the desired jump.
        Vector2 gabeGridPos = mTiles[mGabeTileIndex].gridPos;
        Vector2 jumpDir = toGridPos - gabeGridPos;

        // Based on x/y change, figure out which anims to use.
        // This is similar to the movement on the chessboard puzzle, and some animations are actually reused.
        if(jumpDir.x == 0 && jumpDir.y > 0) // jump forward
        {
            jumpAnim = jumpDir.y > 1 ?
                gAssetManager.LoadAnimation("GABTE5JUMP02SQ", AssetScope::Scene) :
                gAssetManager.LoadAnimation("GABTE5JUMP01SQ", AssetScope::Scene);
        }
        else if(jumpDir.x == 0 && jumpDir.y < 0) // jump backward
        {
            turnAnim = gAssetManager.LoadAnimation("GABTE1TURNC180", AssetScope::Scene);
            jumpAnim = jumpDir.y < -1 ?
                gAssetManager.LoadAnimation("GABTE5JUMP02SQ", AssetScope::Scene) :
                gAssetManager.LoadAnimation("GABTE5JUMP01SQ", AssetScope::Scene);
            turnBackAnim = gAssetManager.LoadAnimation("GABTE1TURNCC180", AssetScope::Scene);
        }
        else if(jumpDir.y == 0)
        {
            // I don't think this puzzle has any opportunity for left/right jumps like this.
            // Going to leave this unimplemented for now.
            printf("Left/right jump during bridge puzzle!?\n");
        }
        else if(jumpDir.x == jumpDir.y) // diagonal
        {
            if(jumpDir.x > 0 && jumpDir.y > 0) // forward right
            {
                turnAnim = gAssetManager.LoadAnimation("GABTE1TURNC45", AssetScope::Scene);
                jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP01SQ", AssetScope::Scene);
                turnBackAnim = gAssetManager.LoadAnimation("GABTE1TURNCC45", AssetScope::Scene);
            }
            else if(jumpDir.x > 0 && jumpDir.y < 0) // back right
            {
                turnAnim = gAssetManager.LoadAnimation("GABTE1TURNC135", AssetScope::Scene);
                jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP01SQ", AssetScope::Scene);
                turnBackAnim = gAssetManager.LoadAnimation("GABTE1TURNCC135", AssetScope::Scene);
            }
            else if(jumpDir.x < 0 && jumpDir.y < 0) // back left
            {
                turnAnim = gAssetManager.LoadAnimation("GABTE1TURNCC135", AssetScope::Scene);
                jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP01SQ", AssetScope::Scene);
                turnBackAnim = gAssetManager.LoadAnimation("GABTE1TURNC135", AssetScope::Scene);
            }
            else if(jumpDir.x < 0 && jumpDir.y > 0) // forward left
            {
                turnAnim = gAssetManager.LoadAnimation("GABTE1TURNCC45", AssetScope::Scene);
                jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP01SQ", AssetScope::Scene);
                turnBackAnim = gAssetManager.LoadAnimation("GABTE1TURNC45", AssetScope::Scene);
            }
        }
        else if(jumpDir.x != 0 && jumpDir.y != 0 && Math::Abs(jumpDir.x) + Math::Abs(jumpDir.y) == 3) // L-shaped knight movement
        {
            if(jumpDir.x == 1 && jumpDir.y == 2) // forward-right
            {
                turnAnim = gAssetManager.LoadAnimation("GabTe1TurnC26", AssetScope::Scene);
                jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP26KNIGHT", AssetScope::Scene);
                turnBackAnim = gAssetManager.LoadAnimation("GabTe1TurnCC26", AssetScope::Scene);
            }
            else if(jumpDir.x == -1 && jumpDir.y == 2) // forward-left
            {
                turnAnim = gAssetManager.LoadAnimation("GabTe1TurnCC26", AssetScope::Scene);
                jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP26KNIGHT", AssetScope::Scene);
                turnBackAnim = gAssetManager.LoadAnimation("GabTe1TurnC26", AssetScope::Scene);
            }
            else if(jumpDir.x == 1 && jumpDir.y == -2) // backward-right
            {
                turnAnim = gAssetManager.LoadAnimation("GabTe1TurnC153", AssetScope::Scene);
                jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP26KNIGHT", AssetScope::Scene);
                turnBackAnim = gAssetManager.LoadAnimation("GabTe1TurnCC153", AssetScope::Scene);
            }
            else if(jumpDir.x == -1 && jumpDir.y == -2) // backward-left
            {
                turnAnim = gAssetManager.LoadAnimation("GabTe1TurnCC153", AssetScope::Scene);
                jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP26KNIGHT", AssetScope::Scene);
                turnBackAnim = gAssetManager.LoadAnimation("GabTe1TurnC153", AssetScope::Scene);
            }
            else if(jumpDir.x == 2 && jumpDir.y == 1) // right-forward
            {
                turnAnim = gAssetManager.LoadAnimation("GabTe1TurnC63", AssetScope::Scene);
                jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP26KNIGHT", AssetScope::Scene);
                turnBackAnim = gAssetManager.LoadAnimation("GabTe1TurnCC63", AssetScope::Scene);
            }
            else if(jumpDir.x == 2 && jumpDir.y == -1) // right-backward
            {
                turnAnim = gAssetManager.LoadAnimation("GabTe1TurnC116", AssetScope::Scene);
                jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP26KNIGHT", AssetScope::Scene);
                turnBackAnim = gAssetManager.LoadAnimation("GabTe1TurnCC116", AssetScope::Scene);
            }
            else if(jumpDir.x == -2 && jumpDir.y == 1) // left-forward
            {
                turnAnim = gAssetManager.LoadAnimation("GabTe1TurnCC63", AssetScope::Scene);
                jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP26KNIGHT", AssetScope::Scene);
                turnBackAnim = gAssetManager.LoadAnimation("GabTe1TurnC63", AssetScope::Scene);
            }
            else if(jumpDir.x == -2 && jumpDir.y == -1) // left-backward
            {
                turnAnim = gAssetManager.LoadAnimation("GabTe1TurnCC116", AssetScope::Scene);
                jumpAnim = gAssetManager.LoadAnimation("GABTE5JUMP26KNIGHT", AssetScope::Scene);
                turnBackAnim = gAssetManager.LoadAnimation("GabTe1TurnC116", AssetScope::Scene);
            }
        }
        else // attempted an invalid move
        {
            // So this means you tried to jump too far - Gabe can only at most moves L-shaped.
            // So...you die!
            Die(true);
            return;
        }
    }

    // Set this as the tile we are jumping to.
    mJumpTileIndex = index;
    mJumping = true;

    // Play the turn/jump/land/unturn anims.
    // Note that if a null anim is passed to "Start", the finish callback just gets called right away - exactly what we want here!
    mJumpAnimParams.allowMove = true;

    // Turn to desired jump direction...
    mJumpAnimParams.animation = turnAnim;
    gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this, index, toWorldPosition, jumpAnim, landAnim, turnBackAnim](){

        // Jump to the other tile...
        mJumpAnimParams.animation = jumpAnim;
        gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this, index, toWorldPosition, landAnim, turnBackAnim](){

            // If jumping onto the first tile of the puzzle, start the other tiles animation.
            if(mGabeTileIndex == -1)
            {
                StartTilePattern();
            }

            // Gabe's tile has changed.
            mGabeTileIndex = index;
            mJumpTileIndex = -1;

            // Make this tile Glow.
            GlowTile(mGabeTileIndex);

            // Play land animation.
            mJumpAnimParams.animation = landAnim;
            mLanding = true;
            gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this, turnBackAnim](){

                // Turn back to looking straight ahead.
                mJumpAnimParams.animation = turnBackAnim;
                gSceneManager.GetScene()->GetAnimator()->Start(mJumpAnimParams, [this](){

                    // Done jumping.
                    mJumping = false;
                    mLanding = false;

                    // Force Gabe to look straight ahead.
                    // After jumping a bunch, a bit of drift can start to appear without this.
                    mGabeActor->SetHeading(Heading::FromDegrees(0.0f));
                    mGabeActor->SetPosition(mJumpToPosition);

                    // If jumped to tile 10 (the end of the puzzle), play the end of puzzle cutscene.
                    if(mGabeTileIndex == 10)
                    {
                        gSceneManager.GetScene()->SetCameraPosition("SOLVED");
                        gGameProgress.SetFlag("CrossedBridge");
                        gActionManager.ExecuteSheepAction("wait CallSheep(\"TE5\", \"GabeCrossedBridge$\")");
                    }
                });
            });
        });
    });
}

void Bridge::UpdateTiles(float deltaTime)
{
    // Update each tile in turn.
    for(int index = 0; index < kTileCount; ++index)
    {
        // Each tile is active as long as not in OFF or SLEEP states.
        Tile& tile = mTiles[index];
        tile.tileActor->SetActive(tile.state != TileState::Off && tile.state != TileState::Sleeping);

        // Increment current state timer.
        tile.stateTimer += deltaTime;

        // Update based on current state.
        int currentFrame = 0;
        switch(tile.state)
        {
            case TileState::Off:
            default:
                break;

            case TileState::Glinting:
                // Go through the animation as time passes.
                currentFrame = static_cast<int>(tile.stateTimer / tile.glintAnim->GetFrameDuration());
                for(int i = tile.animFrame + 1; i <= currentFrame; ++i)
                {
                    gSceneManager.GetScene()->GetAnimator()->Sample(tile.glintAnim, i);
                }

                // Once the animation ends, as long as not jumping to this tile, go to sleep state.
                if(tile.stateTimer >= tile.stateDuration)
                {
                    if(mJumpTileIndex != index)
                    {
                        SleepTile(index, 2.0f);
                    }
                }
                break;

            case TileState::Glowing:
                // Go through the animation as time passes.
                currentFrame = static_cast<int>(tile.stateTimer / tile.glowAnim->GetFrameDuration());
                for(int i = tile.animFrame + 1; i <= currentFrame; ++i)
                {
                    gSceneManager.GetScene()->GetAnimator()->Sample(tile.glowAnim, i);
                }

                // Once the animation ends, as long as not jumping to this tile, go to sleep state.
                if(tile.stateTimer >= tile.stateDuration)
                {
                    // After glowing, sleep for a bit.
                    SleepTile(index, 2.0f);

                    // If Gabe is still standing on this platform when it finishes glowing, he unfortunately MUST DIE!
                    if(!mJumping && mGabeTileIndex == index)
                    {
                        Die(false);
                    }
                }
                break;

            case TileState::Sleeping:
                // If not in the puzzle, tiles just stay in the sleep state.
                // But when you do enter the puzzle, they jump back to life.
                bool inPuzzle = mGabeTileIndex >= 0 || AtPuzzleStart();
                if(inPuzzle)
                {
                    if(tile.stateTimer >= tile.stateDuration)
                    {
                        // When done sleeping, glint the tile.
                        // The first tile ALWAYS glints, even if Gabe isn't on any tile yet.
                        // Other tiles glint as long as Gabe is standing on one of the tiles.
                        if(index == 0 || mGabeTileIndex >= 0)
                        {
                            GlintTile(index);
                        }
                    }
                }
                break;
        }
    }
}

void Bridge::UpdateInteract()
{
    // Assume no forced scene interaction disabled.
    gSceneManager.GetScene()->GetCamera()->SetSceneInteractEnabled(true);

    // Early out of interaction if scene interact is disabled (due to mouse lock, action playing, etc).
    if(!gSceneManager.GetScene()->GetCamera()->IsSceneInteractAllowed())
    {
        return;
    }

    // Disallow normal scene interaction when in the bridge puzzle.
    gSceneManager.GetScene()->GetCamera()->SetSceneInteractEnabled(mGabeTileIndex == -1);

    // Can't interact while mid-jump.
    if(mJumping)
    {
        return;
    }

    // Cast a ray into the scene, see what we hit.
    Ray ray = gSceneManager.GetScene()->GetCamera()->GetSceneRayAtMousePos();
    SceneCastResult result = gSceneManager.GetScene()->Raycast(ray, false);

    // If hit an object, see if it's one of the tiles in this puzzle.
    GKObject* hoveredObject = result.hitObject;
    int hoveredTileIndex = -1;
    if(hoveredObject != nullptr)
    {
        for(int i = 0; i < kTileCount; ++i)
        {
            if(hoveredObject == mTiles[i].tileActor)
            {
                hoveredTileIndex = i;
                break;
            }
        }

        // If hovering the end area, consider that to be a "secret/special" tile with index 10.
        if(hoveredTileIndex == -1 && StringUtil::EqualsIgnoreCase(hoveredObject->GetName(), "te5hittestend"))
        {
            hoveredTileIndex = 10;
        }
    }

    // If Gabe is not yet on the puzzle, but tile index 0 is hovered, allow jumping to that tile on click.
    if(mGabeTileIndex == -1)
    {
        if(hoveredTileIndex == 0)
        {
            gCursorManager.UseRedHighlightCursor(1);
            if(gInputManager.IsMouseButtonTrailingEdge(InputManager::MouseButton::Left))
            {
                JumpToTile(hoveredTileIndex);
            }
        }
    }
    else // in the puzzle
    {
        // When in the puzzle, allow Gabe to attempt to jump to ANY tile.
        // If it's a bad choice though, he might not make the jump...
        if(hoveredTileIndex >= 0 && hoveredTileIndex != mGabeTileIndex)
        {
            gCursorManager.UseRedHighlightCursor(1);
            if(gInputManager.IsMouseButtonTrailingEdge(InputManager::MouseButton::Left))
            {
                JumpToTile(hoveredTileIndex);
            }
        }
        else if(StringUtil::EqualsIgnoreCase(result.hitInfo.name, "te5_floor"))
        {
            // If the player clicks on the floor area when on tile 0 or 1, Gabe is able to "jump back" off the puzzle, back to the beginning.
            gCursorManager.UseDefaultCursor();
            if(gInputManager.IsMouseButtonTrailingEdge(InputManager::MouseButton::Left))
            {
                if(mGabeTileIndex == 0 || mGabeTileIndex == 1)
                {
                    JumpToTile(-1);
                }
            }
        }
        else if(StringUtil::EqualsIgnoreCase(result.hitInfo.name, "te5_hittest_floor"))
        {
            // In this case, the player clicked on the empty chasm beneath the bridge. Probably accidentally, as a tile was disappearing.
            // Buuuuut, that's a death! Jump into the chasm? OK!
            gCursorManager.UseDefaultCursor();
            if(gInputManager.IsMouseButtonTrailingEdge(InputManager::MouseButton::Left))
            {
                Die(true);
            }
        }
        else
        {
            gCursorManager.UseDefaultCursor();
        }
    }
}
