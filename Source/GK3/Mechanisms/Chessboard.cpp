#include "Chessboard.h"

#include "ActionManager.h"
#include "Animator.h"
#include "AssetManager.h"
#include "GameCamera.h"
#include "GameProgress.h"
#include "GKActor.h"
#include "GKObject.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Timers.h"

Chessboard::Chessboard() : Actor("Chessboard")
{
    // Do nothing here - the game calls "Reset(false)" on scene enter or on retry.
}

void Chessboard::Reset(bool swordsGlow)
{
    // Reset various state game variables.
    gGameProgress.SetGameVariable("Te1GabeRow", -1);
    gGameProgress.SetGameVariable("Te1GabeColumn", -1);
    gGameProgress.SetGameVariable("Te1MoveType", 1);
    gGameProgress.SetGameVariable("Te1TileState", 0);
    gGameProgress.ClearFlag("AllSwords");
    gGameProgress.SetGameVariable("Te1TileRow", 0);
    gGameProgress.SetGameVariable("Te1TileColumn", 0);

    // Iterate all tiles and set them to initial states.
    for(int col = 0; col < 8; ++col)
    {
        for(int row = 0; row < 8; ++row)
        {
            // All should be visible to start.
            SetTileVisible(row, col, true);

            // Set glow state as desired by caller.
            SetSwordGlow(row, col, swordsGlow);

            // Clear all land counts.
            mLandedCounts[row][col] = 0;
        }
    }

    // Certain tiles are immediately "death traps" - landing on them is not allowed.
    // Set them all to a high land count so that you'd die if you used them.
    mLandedCounts[1][3] = 2;
    mLandedCounts[1][4] = 2;

    mLandedCounts[6][3] = 2;
    mLandedCounts[6][4] = 2;

    mLandedCounts[3][1] = 2;
    mLandedCounts[4][1] = 2;

    mLandedCounts[3][6] = 2;
    mLandedCounts[4][6] = 2;

    mLandedCounts[3][3] = 2;
    mLandedCounts[4][3] = 2;
    mLandedCounts[3][4] = 2;
    mLandedCounts[4][4] = 2;

    // No swords yet.
    gGameProgress.SetGameVariable("Te1SwordCount", 0);
    mSwordCount = 0;

    // Make sure goal door is closed again, if it was opened and we then died.
    Animation* doorOpenAnim = gAssetManager.LoadAnimation("Te1GoDoor", AssetScope::Scene);
    gSceneManager.GetScene()->GetAnimator()->Sample(doorOpenAnim, 0);
}

void Chessboard::Takeoff()
{
    // This function is called just as Gabe is about to jump to a new tile.

    // First, get the row/col of the tile being jumped from.
    int row = gGameProgress.GetGameVariable("Te1GabeRow");
    int col = gGameProgress.GetGameVariable("Te1GabeColumn");

    // If this is not a sword tile, it should disappear as Gabe jumps away from it.
    // Use a slight timer, or else the tile disappears when Gabe is still on it.
    if(!IsWhiteSwordTile(row, col) && !IsBlackSwordTile(row, col))
    {
        Timers::AddTimerSeconds(1.0f, [this, row, col](){
            OpenTrapdoor(row, col);
        });
    }
}

void Chessboard::Landed()
{
    // This function is called after Gabe has landed on a new tile.

    // Get row/col landed on.
    int row = gGameProgress.GetGameVariable("Te1GabeRow");
    int col = gGameProgress.GetGameVariable("Te1GabeColumn");

    // Increment land count for this tile.
    mLandedCounts[row][col]++;

    // Set tile state to the land count.
    // Scripts use this to decide if this is a death condition.
    gGameProgress.SetGameVariable("Te1TileState", mLandedCounts[row][col]);

    // If this is the first time landing on a sword tile...
    if(mLandedCounts[row][col] == 1 && SetSwordGlow(row, col, false))
    {
        // Play "sword off" SFX.
        gAudioManager.PlaySFX(gAssetManager.LoadAudio("TE1SWORDOFF.WAV", AssetScope::Scene));

        // Increment swords landed on.
        // Once we land on 16, we've got all the swords, which may finish the puzzle.
        ++mSwordCount;
        gGameProgress.SetGameVariable("Te1SwordCount", mSwordCount);
        if(mSwordCount == kSwordTileCount)
        {
            // Sheep code checks this flag to see if the puzzle was completed.
            // Gabe is also required to complete the puzzle on the correct final sword...but the Sheep code takes care of that!
            gGameProgress.SetFlag("AllSwords");
        }
    }
    else if(mLandedCounts[row][col] > 1) // we landed on this tile before (big no no)
    {
        // If you land on a tile you've already landed on before, you are dead - hide the tile while the fall anim plays.
        OpenTrapdoor(row, col);
    }
}

void Chessboard::HideCurrentTile()
{
    // Hides the current tile, whatever it is.
    int row = gGameProgress.GetGameVariable("Te1GabeRow");
    int col = gGameProgress.GetGameVariable("Te1GabeColumn");
    OpenTrapdoor(row, col);
}

void Chessboard::CenterEgo()
{
    // GK3's animation system isn't the most precise thing in the world.
    // After jumping around a few times, Gabe may start to "drift" away from the center of tiles.
    // To alleviate this, the developers center Gabe's position after each jump.

    // Get Gabe's row/column.
    int row = gGameProgress.GetGameVariable("Te1GabeRow");
    int col = gGameProgress.GetGameVariable("Te1GabeColumn");

    // Get the tile's position.
    GKObject* tile = gSceneManager.GetScene()->GetSceneObjectByModelName(GetTileModelName(row, col));
    if(tile != nullptr)
    {
        gSceneManager.GetScene()->GetEgo()->SetPosition(tile->GetPosition());
    }

    // Also reset Gabe to be looking straight ahead.
    gSceneManager.GetScene()->GetEgo()->SetHeading(Heading::FromDegrees(0.0f));
}

void Chessboard::BadLand()
{
    // This seems to be called if you perform an invalid move.
    // It basically just opens the trapdoor and you plummet to your death.
    int row = gGameProgress.GetGameVariable("Te1GabeRow");
    int col = gGameProgress.GetGameVariable("Te1GabeColumn");
    OpenTrapdoor(row, col);
}

void Chessboard::OnPersist(PersistState& ps)
{
    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            ps.Xfer("", mLandedCounts[x][y]);
        }
    }

    ps.Xfer(PERSIST_VAR(mSwordCount));
}

void Chessboard::OnUpdate(float deltaTime)
{
    // The code below updates which tile the mouse is hovering, whether it's a valid move, and what animation to use when jumping to it.
    // That code is here (rather than directly in Scene::Interact) to avoid polluting the Scene class with scene-specific logic.
    // We want to do these updates before the player clicks on a tile, but once they do, we should stop until the action bar is gone and any subsequent action is done.
    if(gActionManager.IsActionBarShowing() || gActionManager.IsActionPlaying()) { return; }
    gSceneManager.GetScene()->GetCamera()->SetSceneInteractEnabled(true);

    // Get the object hovered by the mouse cursor, if any.
    Ray ray = gSceneManager.GetScene()->GetCamera()->GetSceneRayAtMousePos();
    SceneCastResult result = gSceneManager.GetScene()->Raycast(ray, false);

    const int kValidMoveType = 1;
    const int kInvalidMoveType = 2;
    GKObject* hoveredObject = result.hitObject;
    if(hoveredObject != nullptr)
    {
        // We're interested if the hovered object is one of the floor tiles.
        const std::string& hoveredObjectName = hoveredObject->GetName();
        if(StringUtil::StartsWithIgnoreCase(hoveredObject->GetName(), "te1floor"))
        {
            // Derive row/column from the floor tile name.
            int col = hoveredObjectName[8] - 'a';
            int row = hoveredObjectName[9] - '1';
            //printf("Row %d, Col %d\n", row, col);

            // Save last hovered row/col.
            gGameProgress.SetGameVariable("Te1TileRow", row);
            gGameProgress.SetGameVariable("Te1TileColumn", col);

            // Figure out valid row/col moves from Gabe's current position.
            int gabeRow = gGameProgress.GetGameVariable("Te1GabeRow");
            int gabeCol = gGameProgress.GetGameVariable("Te1GabeColumn");

            // Here's an easy case for the start of the puzzle:
            // If Gabe's not yet on the board, the only valid move is to the first row.
            if(gabeRow < 0)
            {
                if(row == 0)
                {
                    gGameProgress.SetGameVariable("Te1MoveType", kValidMoveType);
                }
                else
                {
                    gGameProgress.SetGameVariable("Te1MoveType", kInvalidMoveType);
                }
            }
            else // Gabe is on the board
            {
                // Valid moves on the board are L-shaped Knight moves.
                // Fortunately, these can be detected broadly with a bit of math:
                // Both row/col diff must be greater than zero, and the sub of the diffs must be three. Think about it!
                int colDiff = Math::Abs(gabeCol - col);
                int rowDiff = Math::Abs(gabeRow - row);
                if(colDiff > 0 && rowDiff > 0 && colDiff + rowDiff == 3)
                {
                    gGameProgress.SetGameVariable("Te1MoveType", kValidMoveType);
                }
                else
                {
                    gGameProgress.SetGameVariable("Te1MoveType", kInvalidMoveType);
                }

                // When Gabe moves to a new tile, he needs to turn to face that tile before jumping.
                // Rather than do this in any mathematical way, the game logic uses a pretty convoluted numbering system.
                // Depending on where the tile is relative to the current tile, a specific number is assigned to a variable.
                // In Sheepscript, that number correlates to a specific rotation animation.

                // Cardinal Directions
                const int kNone = 12;
                const int kFront1 = 17;
                const int kFront2 = 22;
                const int kBack1 = 7;
                const int kBack2 = 2;
                const int kRight1 = 13;
                const int kRight2 = 14;
                const int kLeft1 = 11;
                const int kLeft2 = 10;

                // Diagonals
                const int kDiagFrontLeft = 16;
                const int kDiagFrontRight = 18;
                const int kDiagBackLeft = 6;
                const int kDiagBackRight = 8;

                // Knight L-Shaped Moves (and any invalid moves beyond that)
                const int kLFrontRight = 23;
                const int kLFrontLeft = 21;
                const int kLBackRight = 3;
                const int kLBackLeft = 1;
                const int kLRightFront = 19;
                const int kLRightBack = 9;
                const int kLLeftFront = 15;
                const int kLLeftBack = 5;

                int moveCode = 0;
                if(colDiff == 0 && rowDiff == 0)
                {
                    // Clicked on the tile Gabe is currently on, so no move happens.
                    moveCode = kNone;
                }
                else if(colDiff == 0)
                {
                    // Tile is either directly in front of or directly behind Gabe.
                    if(row < gabeRow) // move backward
                    {
                        moveCode = rowDiff == 1 ? kBack1 : kBack2;
                    }
                    else if(row > gabeRow) // move forward
                    {
                        moveCode = rowDiff == 1 ? kFront1 : kFront2;
                    }
                }
                else if(rowDiff == 0)
                {
                    // Tile is either directly to the left or directly to the right.
                    if(col < gabeCol) // move left
                    {
                        moveCode = colDiff == 1 ? kLeft1 : kLeft2;
                    }
                    else if(col > gabeCol) // move right
                    {
                        moveCode = colDiff == 1 ? kRight1 : kRight2;
                    }
                }
                else if(rowDiff == colDiff) // tile is on a diagonal from the current tile
                {
                    if(row < gabeRow && col < gabeCol)
                    {
                        moveCode = kDiagBackLeft;
                    }
                    else if(row < gabeRow && col > gabeCol)
                    {
                        moveCode = kDiagBackRight;
                    }
                    else if(row > gabeRow && col < gabeCol)
                    {
                        moveCode = kDiagFrontLeft;
                    }
                    else
                    {
                        moveCode = kDiagFrontRight;
                    }
                }
                else
                {
                    // This move is either an L-shape or some other crazy invalid move, but in the general direction of the L-shape.
                    // In either case, we can fortunately use the same move code to get the right animations.
                    if(row < gabeRow && col < gabeCol)
                    {
                        moveCode = rowDiff > colDiff ? kLBackLeft : kLLeftBack;
                    }
                    else if(row < gabeRow && col > gabeCol)
                    {
                        moveCode = rowDiff > colDiff ? kLBackRight : kLRightBack;
                    }
                    else if(row > gabeRow && col < gabeCol)
                    {
                        moveCode = rowDiff > colDiff ? kLFrontLeft : kLLeftFront;
                    }
                    else
                    {
                        moveCode = rowDiff > colDiff ? kLFrontRight : kLRightFront;
                    }
                }
                //printf("Move code %d\n", moveCode);
                gGameProgress.SetGameVariable("Te1MoveCode", moveCode);
            }
        }
    }

    // If Gabe is on the Chessboard, deal with clicks on the floor area.
    // This requires some special processing so that Gabe doesn't just try to walk in an invalid way for the puzzle.
    if(gGameProgress.GetGameVariable("Te1GabeRow") >= 0)
    {
        bool hoveringTileFloor = StringUtil::EqualsIgnoreCase(result.hitInfo.name, "te1tilefloor");
        bool hoveringHitTestFloor = StringUtil::EqualsIgnoreCase(result.hitInfo.name, "te1_hittestfloor") ||
                                    StringUtil::EqualsIgnoreCase(result.hitInfo.name, "te1flooredges") ||
                                    StringUtil::EqualsIgnoreCase(result.hitInfo.name, "te1_floorstand");

        // Turn off normal scene interaction so that clicking doesn't elicit the normal walk behavior.
        if(hoveringTileFloor || hoveringHitTestFloor)
        {
            gSceneManager.GetScene()->GetCamera()->SetSceneInteractEnabled(false);
        }

        // Gabe can only attempt to escape by clicking the tile floor.
        if(hoveringTileFloor)
        {
            // If a click is done, attempt to jump off the chessboard, back to safety.
            if(gInputManager.IsMouseButtonTrailingEdge(InputManager::MouseButton::Left))
            {
                // If not in the first row, it's impossible to jump back. Set as an invalid move.
                if(gGameProgress.GetGameVariable("Te1GabeRow") == 0)
                {
                    gGameProgress.SetGameVariable("Te1MoveType", kValidMoveType);
                }
                else
                {
                    gGameProgress.SetGameVariable("Te1MoveType", kInvalidMoveType);
                }

                // Execute the scene jump action, which will either send you to safety or illicit an "it's too far away" response.
                gActionManager.ExecuteAction("SCENE", "JUMP");
            }
        }
    }
}

bool Chessboard::IsWhiteSwordTile(int row, int col)
{
    return (col == 3 && row == 0) ||
        (col == 2 && row == 1) ||
        (col == 1 && row == 2) ||
        (col == 0 && row == 3) ||
        (col == 7 && row == 4) ||
        (col == 6 && row == 5) ||
        (col == 5 && row == 6) ||
        (col == 4 && row == 7);
}

bool Chessboard::IsBlackSwordTile(int row, int col)
{
    return (col == 4 && row == 0) ||
        (col == 5 && row == 1) ||
        (col == 6 && row == 2) ||
        (col == 7 && row == 3) ||
        (col == 0 && row == 4) ||
        (col == 1 && row == 5) ||
        (col == 2 && row == 6) ||
        (col == 3 && row == 7);
}

bool Chessboard::SetSwordGlow(int row, int col, bool glow)
{
    // If any is not loaded, load them now!
    if(unlitSwordWhite == nullptr)
    {
        unlitSwordWhite = gAssetManager.LoadTexture("TE1SWORDW.BMP", AssetScope::Scene);
        unlitSwordBlack = gAssetManager.LoadTexture("TE1SWORDB.BMP", AssetScope::Scene);
        litSwordWhite = gAssetManager.LoadTexture("TE1SWORDW_GLOW.BMP", AssetScope::Scene);
        litSwordBlack = gAssetManager.LoadTexture("TE1SWORDB_GLOW.BMP", AssetScope::Scene);
    }

    if(IsWhiteSwordTile(row, col))
    {
        gSceneManager.GetScene()->ApplyTextureToSceneModel(GetTileModelName(row, col), glow ? litSwordWhite : unlitSwordWhite);
        return true;
    }
    if(IsBlackSwordTile(row, col))
    {
        gSceneManager.GetScene()->ApplyTextureToSceneModel(GetTileModelName(row, col), glow ? litSwordBlack : unlitSwordBlack);
        return true;
    }
    return false;
}

void Chessboard::OpenTrapdoor(int row, int col)
{
    // Disable the graphic and play a sound effect.
    SetTileVisible(row, col, false);
    gAudioManager.PlaySFX(gAssetManager.LoadAudio("TE1TRAPDOOROPEN.WAV", AssetScope::Scene));
}

const std::string& Chessboard::GetTileModelName(int row, int col)
{
    static std::string tileName = "te1floora1";
    tileName[8] = 'a' + col;
    tileName[9] = '1' + row;
    return tileName;
}

void Chessboard::SetTileVisible(int row, int col, bool visible)
{
    gSceneManager.GetScene()->SetSceneModelVisibility(GetTileModelName(row, col), visible);
}
