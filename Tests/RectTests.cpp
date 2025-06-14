//
// Clark Kromenaker
//
// Tests for Rect and RectUtil.
//
#include "catch.hh"
#include "Rect.h"
#include "RectUtil.h"

TEST_CASE("Rect constructors are correct")
{
    // Check default constructor gives correct results.
    Rect defaultRect;
    REQUIRE(defaultRect.x == 0.0f);
    REQUIRE(defaultRect.y == 0.0f);
    REQUIRE(defaultRect.width == 0.0f);
    REQUIRE(defaultRect.height == 0.0f);
    REQUIRE(defaultRect.GetSize() == Vector2::Zero);
    REQUIRE(defaultRect.GetMin() == Vector2::Zero);
    REQUIRE(defaultRect.GetMax() == Vector2::Zero);

    // Check x/y constructor gives correct results.
    Rect positionedRect(10.0f, -12.4f);
    REQUIRE(positionedRect.x == 10.0f);
    REQUIRE(positionedRect.y == -12.4f);
    REQUIRE(positionedRect.width == 0.0f);
    REQUIRE(positionedRect.height == 0.0f);
    REQUIRE(positionedRect.GetSize() == Vector2::Zero);
    REQUIRE(positionedRect.GetMin() == Vector2(10.0f, -12.4f));
    REQUIRE(positionedRect.GetMax() == Vector2(10.0f, -12.4f));

    // Check x/y/width/height constructor gives correct results.
    Rect sizedRect(100.0f, -50.0f, 100.0f, 300.0f);
    REQUIRE(sizedRect.x == 100.0f);
    REQUIRE(sizedRect.y == -50.0f);
    REQUIRE(sizedRect.width == 100.0f);
    REQUIRE(sizedRect.height == 300.0f);
    REQUIRE(sizedRect.GetSize() == Vector2(100.0f, 300.0f));
    REQUIRE(sizedRect.GetMin() == Vector2(100.0f, -50.0f));
    REQUIRE(sizedRect.GetMax() == Vector2(200.0f, 250.0f));

    // Check min/max constructor gives expected results.
    Rect minMaxRect(Vector2(-25.0f, -25.0f), Vector2(40.0f, 0.0f));
    REQUIRE(minMaxRect.x == -25.0f);
    REQUIRE(minMaxRect.y == -25.0f);
    REQUIRE(minMaxRect.width == 65.0f);
    REQUIRE(minMaxRect.height == 25.0f);
    REQUIRE(minMaxRect.GetSize() == Vector2(65.0f, 25.0f));
    REQUIRE(minMaxRect.GetMin() == Vector2(-25.0f, -25.0f));
    REQUIRE(minMaxRect.GetMax() == Vector2(40.0f, 0.0f));
}

TEST_CASE("Rect::Contains works")
{
    Rect rect(0.0f, 0.0f, 100.0f, 100.0f);

    // Check edge cases.
    REQUIRE(rect.Contains(Vector2(0.0f, 0.0f)));
    REQUIRE(rect.Contains(Vector2(100.0f, 100.0f)));

    // Check center point.
    REQUIRE(rect.Contains(Vector2(50.0f, 50.0f)));

    // Check misc point.
    REQUIRE(rect.Contains(Vector2(23.4423f, 40.3304f)));

    // Check points that should not be contained.
    REQUIRE(!rect.Contains(Vector2(-10.0f, 0.0f)));
    REQUIRE(!rect.Contains(Vector2(50.0f, 101.0f)));
}

TEST_CASE("Rect::Overlaps works")
{
    Rect rectA(0.0f, 0.0f, 100.0f, 100.0f);
    Rect rectB(-100.0f, -100.0f, 100.0f, 100.0f);
    Rect rectB1(-101.0f, -101.0f, 100.0f, 100.0f);
    Rect rectC(200.0f, 200.0f, 50.0f, 50.0f);
    Rect rectD(-50.0f, 50.0f, 100.0f, 100.0f);

    // RectA & RectB should be barely touching...but still counts.
    REQUIRE(rectA.Overlaps(rectB));
    REQUIRE(rectB.Overlaps(rectA));

    // RectB1 is one unit off from RectB, and should not overlap RectA.
    REQUIRE(!rectA.Overlaps(rectB1));
    REQUIRE(!rectB1.Overlaps(rectA));

    // RectA and RectC should not overlap.
    REQUIRE(!rectA.Overlaps(rectC));
    REQUIRE(!rectC.Overlaps(rectA));

    // RectA and RectD should overlap partially.
    REQUIRE(rectA.Overlaps(rectD));
    REQUIRE(rectD.Overlaps(rectA));

    // RectC and RectD do not overlap.
    REQUIRE(!rectC.Overlaps(rectD));
    REQUIRE(!rectD.Overlaps(rectC));
}

TEST_CASE("Rect::GetPoint works")
{
    Rect rect(45.0f, -30.0f, 240.0f, 50.0f);

    // Check all edges.
    REQUIRE(rect.GetPoint(Vector2(0.0f, 0.0f)) == Vector2(45.0f, -30.0f));
    REQUIRE(rect.GetPoint(Vector2(1.0f, 1.0f)) == Vector2(285.0f, 20.0f));
    REQUIRE(rect.GetPoint(Vector2(1.0f, 0.0f)) == Vector2(285.0f, -30.0f));
    REQUIRE(rect.GetPoint(Vector2(0.0f, 1.0f)) == Vector2(45.0f, 20.0f));

    // Check some inner values.
    REQUIRE(rect.GetPoint(Vector2(0.5f, 0.5f)) == Vector2(165.0f, -5.0f));
    REQUIRE(rect.GetPoint(Vector2(0.75f, 0.75f)) == Vector2(225.0f, 7.5f));

    // Should still work for not-really-normalized values!
    REQUIRE(rect.GetPoint(Vector2(-0.5f, -0.2f)) == Vector2(-75.0f, -40.0f));
    REQUIRE(rect.GetPoint(Vector2(2.0f, 2.0f)) == Vector2(525.0f, 70.0f));
}

TEST_CASE("Rect::GetNormalizedPoint works")
{
    Rect rect(-15.0f, 20.0f, 90.0f, 64.0f);

    // Check some edge values.
    REQUIRE(rect.GetNormalizedPoint(Vector2(-15.0f, 20.0f)) == Vector2::Zero);
    REQUIRE(rect.GetNormalizedPoint(Vector2(75.0f, 84.0f)) == Vector2::One);

    // Check some inner values.
    REQUIRE(rect.GetNormalizedPoint(Vector2(0.0f, 50.0f)) == Vector2(0.166667f, 0.46875f));
    REQUIRE(rect.GetNormalizedPoint(Vector2(60.0f, 80.0f)) == Vector2(0.833333f, 0.9375f));

    // Check some out-of-range values (should still work!).
    REQUIRE(rect.GetNormalizedPoint(Vector2(-100.0f, 0.0f)) == Vector2(-0.9444444444f, -0.3125f));
    REQUIRE(rect.GetNormalizedPoint(Vector2(165.0f, 148.0f)) == Vector2(2.0f, 2.0f));

    // Check edge-case with a zero-sized rect.
    Rect zeroRect(10.0f, 10.0f);
    REQUIRE(zeroRect.GetNormalizedPoint(Vector2(10.0f, 10.0f)) == Vector2::Zero);
    REQUIRE(zeroRect.GetNormalizedPoint(Vector2(99.0f, 34.34f)) == Vector2::Zero);
}

TEST_CASE("RectUtil::CalcAnchorRect works")
{
    Rect parentRect(10.0f, 10.0f, 100.0f, 100.0f);

    // Check that the anchor rect in base case matches the parent rect.
    REQUIRE(RectUtil::CalcAnchorRect(parentRect, Vector2::Zero, Vector2::One) == parentRect);

    // Check that anchor rect when anchor min/max are equal is actually a point.
    Rect pointRect = RectUtil::CalcAnchorRect(parentRect, Vector2::Zero, Vector2::Zero);
    REQUIRE(pointRect.x == 10.0f);
    REQUIRE(pointRect.y == 10.0f);
    REQUIRE(pointRect.width == 0.0f);
    REQUIRE(pointRect.height == 0.0f);

    pointRect = RectUtil::CalcAnchorRect(parentRect, Vector2::One, Vector2::One);
    REQUIRE(pointRect.x == 110.0f);
    REQUIRE(pointRect.y == 110.0f);
    REQUIRE(pointRect.width == 0.0f);
    REQUIRE(pointRect.height == 0.0f);

    pointRect = RectUtil::CalcAnchorRect(parentRect, Vector2(0.5f, 0.5f), Vector2(0.5f, 0.5f));
    REQUIRE(pointRect.x == 60.0f);
    REQUIRE(pointRect.y == 60.0f);
    REQUIRE(pointRect.width == 0.0f);
    REQUIRE(pointRect.height == 0.0f);

    // Check that anchors with a matching X/Y value make a rect with zero width/height.
    Rect lineRect = RectUtil::CalcAnchorRect(parentRect, Vector2::Zero, Vector2(0.0f, 1.0f));
    REQUIRE(lineRect.x == 10.0f);
    REQUIRE(lineRect.y == 10.0f);
    REQUIRE(lineRect.width == 0.0f);
    REQUIRE(lineRect.height == 100.0f);

    lineRect = RectUtil::CalcAnchorRect(parentRect, Vector2(0.0f, 0.5f), Vector2(1.0f, 0.5f));
    REQUIRE(lineRect.x == 10.0f);
    REQUIRE(lineRect.y == 60.0f);
    REQUIRE(lineRect.width == 100.0f);
    REQUIRE(lineRect.height == 0.0f);

    // Check anchor rect for an actual rect.
    Rect anchorRect = RectUtil::CalcAnchorRect(parentRect, Vector2(0.25f, 0.4f), Vector2(0.75f, 0.6f));
    REQUIRE(anchorRect.x == 35.0f);
    REQUIRE(anchorRect.y == 50.0f);
    REQUIRE(anchorRect.width == 50.0f);
    REQUIRE(anchorRect.height == 20.0f);
}

TEST_CASE("RectUtil::CalcChildRect works")
{
    Rect parentRect(-480.0f, -269.86f, 960.0f, 539.73f);
    Vector2 anchorMin(0.2f, 0.3f);
    Vector2 anchorMax(0.8f, 0.4f);
    Vector2 sizeDelta(20.0f, -10.0f);
    Vector2 pivot(0.5f, 0.2f);

    Rect rect = RectUtil::CalcLocalRect(parentRect, anchorMin, anchorMax, sizeDelta, pivot);
    //REQUIRE(rect == Rect(-298.0f, -8.79459857f, 596.0f, 43.9729919f));
    REQUIRE(rect == Rect(-298.0f, -9.0f, 596.0f, 43.0f)); // since CalcLocalRect now rounds for pixel-perfect placement

    Rect parentRect2(-512.0f, -384.0f, 1024.0f, 768.0f);
    Vector2 anchorMin2(0.5f, 0.5f);
    Vector2 anchorMax2(1.0f, 0.5f);
    Vector2 sizeDelta2(256.0f, 80.0f);
    Vector2 pivot2(0.0f, 0.8f);

    rect = RectUtil::CalcLocalRect(parentRect2, anchorMin2, anchorMax2, sizeDelta2, pivot2);
    REQUIRE(rect == Rect(0.0f, -64.0f, 768.0f, 80.0f));
}

TEST_CASE("RectUtil::CalcLocalPosition works")
{
    Rect parentRect(-512.0f, -384.0f, 1024.0f, 768.0f);
    Vector2 parentPivot(0.5f, 0.5f);
    Vector2 anchorMin(0.0f, 0.0f);
    Vector2 anchorMax(1.0f, 1.0f);
    Vector2 anchoredPosition(0.0f, 0.0f);
    Vector2 pivot(0.5f, 0.5f);

    // A child rect with same pivot, full anchor min/max, and zero anchored position has no local pos offset.
    Vector3 localPos = RectUtil::CalcLocalPosition(parentRect, parentPivot, anchorMin, anchorMax, anchoredPosition, pivot);
    REQUIRE(localPos == Vector3::Zero);

    // If the parent pivot changes, the local pos should change.
    Vector2 offcenterParentPivot(0.0f, 0.5f);
    localPos = RectUtil::CalcLocalPosition(parentRect, offcenterParentPivot, anchorMin, anchorMax, anchoredPosition, pivot);
    REQUIRE(localPos == Vector3(512.0f, 0.0f, 0.0f));

    // If the anchored position changes, it should change the local pos.
    Vector2 adjustedAnchoredPosition(95.0f, -200.0f);
    localPos = RectUtil::CalcLocalPosition(parentRect, parentPivot, anchorMin, anchorMax, adjustedAnchoredPosition, pivot);
    REQUIRE(localPos == Vector3(95.0f, -200.0f, 0.0f));

    // Try a very unusual one!
    Vector2 anchorMin2(0.3f, 0.1f);
    Vector2 anchorMax2(0.4f, 0.2f);
    Vector3 anchoredPosition2(-104.0f, 323.0f);
    Vector3 pivot2(0.2f, 0.6f);
    localPos = RectUtil::CalcLocalPosition(parentRect, parentPivot, anchorMin2, anchorMax2, anchoredPosition2, pivot2);
    REQUIRE(localPos == Vector3(-288.320007f, 61.8800049f, 0.0f));
}
