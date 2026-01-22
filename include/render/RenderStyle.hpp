#pragma once

struct RenderStyle {
    double fontScale = 1.0;
    int thickness = 2;
    int outlineThickness = 4;

    int marginPx = 40;  
    int lineSpacingPx = 10; 

    bool drawBox = true;
    double boxAlpha = 0.35; 
    int reservedBottomPx = 0;

};
