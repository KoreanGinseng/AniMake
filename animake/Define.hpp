#pragma once
constexpr  double      appVersion            = 1.22;

typedef    int         BOOL;
#define    TRUE        1
#define    FALSE       0

#define    SAFE_DELETE(x)    \
           if(x) delete x;   \
           x = nullptr

#define    SAFE_DELETEARRAY(x)    \
           if(x) delete[] x;      \
           x = nullptr

constexpr  int         windowWidth           = 1280;
constexpr  int         windowHeight          = 720;

constexpr  int         animDataWindowWidth   = 300;
constexpr  int         animDataWindowHeight  = 554;

constexpr  int         animListWindowWidth   = 180;
constexpr  int         animListWindowHeight  = animDataWindowHeight;

constexpr  int         animViewWindowWidth   = windowWidth - (animDataWindowWidth + animListWindowWidth);
constexpr  int         animViewWindowHeight  = animDataWindowHeight;
