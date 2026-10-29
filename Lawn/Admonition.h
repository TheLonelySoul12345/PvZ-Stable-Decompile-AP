#pragma once
#include <string>

#include "../SexyAppFramework/Rect.h"

namespace Sexy
{
    class Graphics;
}

class Admonition
{
public:
    std::string text;
    long timer;
    Sexy::Rect area;
    bool admonish_downwards;
    
    Admonition();
    void                Draw(Sexy::Graphics* g);
    void                SetText(std::string text);
    void                SetArea(Sexy::Rect rect);
    void                SetAdmonishDownwards(bool admonish_downwards);
    void                Update();
    bool                IsDead();
    void                Die();
};
