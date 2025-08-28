//
// Clark Kromenaker
//
// Handles the logic and operation of the chessboard puzzle in TE1 location.
//
#pragma once
#include "Actor.h"

class PersistState;
class Texture;

class Chessboard : public Actor
{
public:
    Chessboard();

    void Reset(bool swordsGlow);

    void Takeoff();
    void Landed();
    void HideCurrentTile();
    void CenterEgo();
    void BadLand();

    void OnPersist(PersistState& ps);

protected:
    void OnUpdate(float deltaTime) override;

private:
    // Tracks how many times Gabe has landed on each tile.
    int mLandedCounts[8][8] = { 0 };

    // Tracks how many sword tiles Gabe has landed on.
    // The goal of this puzzle is to land on all 16 tiles.
    static const int kSwordTileCount = 16;
    int mSwordCount = 0;

    // Textures for swapping the white/black swords either lit or unlit.
    Texture* unlitSwordWhite = nullptr;
    Texture* unlitSwordBlack = nullptr;
    Texture* litSwordWhite = nullptr;
    Texture* litSwordBlack = nullptr;

    bool IsWhiteSwordTile(int row, int col);
    bool IsBlackSwordTile(int row, int col);

    bool SetSwordGlow(int row, int col, bool glow);

    void OpenTrapdoor(int row, int col);

    const std::string& GetTileModelName(int row, int col);
    void SetTileVisible(int row, int col, bool visible);
};