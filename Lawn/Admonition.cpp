#include "Admonition.h"

#include <utility>

#include "../Resources.h"
#include "../Sexy.TodLib/TodCommon.h"
#include "../SexyAppFramework/Font.h"
#include "../SexyAppFramework/Graphics.h"

const int TRIANGLE_SIZE = 5;
const int PADDING = 2;

Admonition::Admonition()
{
    this->timer = 600;
    this->admonish_downwards = false;
}

void Admonition::Draw(Sexy::Graphics* g)
{
    g->PushState();
    g->SetClipRect(0, 0, BOARD_WIDTH, BOARD_HEIGHT);
    auto ascent = FONT_PICO129->GetAscent();
    auto text_width = FONT_PICO129->StringWidth(this->text);
    auto text_y = this->admonish_downwards ? this->area.mY + this->area.mHeight + TRIANGLE_SIZE + PADDING : this->area.mY - TRIANGLE_SIZE - PADDING - ascent;
    auto x_half = this->area.mX + this->area.mWidth / 2;
    auto text_x = x_half - text_width / 2;
    
    Point rectangle_vertices[7];
    if (this->admonish_downwards)
    {
        rectangle_vertices[0] = Point(text_x - PADDING, text_y - PADDING);
        rectangle_vertices[1] = Point(x_half - TRIANGLE_SIZE, text_y - PADDING);
        rectangle_vertices[2] = Point(x_half, text_y - PADDING - TRIANGLE_SIZE);
        rectangle_vertices[3] = Point(x_half + TRIANGLE_SIZE, text_y - PADDING);
        rectangle_vertices[4] = Point(text_x + text_width + PADDING, text_y - PADDING);
        rectangle_vertices[5] = Point(text_x + text_width + PADDING, text_y + ascent + PADDING);
        rectangle_vertices[6] = Point(text_x - PADDING, text_y + ascent + PADDING);
    }
    else
    {
        rectangle_vertices[0] = Point(text_x - PADDING, text_y - PADDING);
        rectangle_vertices[1] = Point(text_x + text_width + PADDING, text_y - PADDING);
        rectangle_vertices[2] = Point(text_x + text_width + PADDING, text_y + ascent + PADDING);
        rectangle_vertices[3] = Point(x_half + TRIANGLE_SIZE, text_y + ascent + PADDING);
        rectangle_vertices[4] = Point(x_half, text_y + ascent + PADDING + TRIANGLE_SIZE);
        rectangle_vertices[5] = Point(x_half - TRIANGLE_SIZE, text_y + ascent + PADDING);
        rectangle_vertices[6] = Point(text_x - PADDING, text_y + ascent + PADDING);
    }
    
    g->SetColor(Color(255, 255, 200, 255));
    g->PolyFill(rectangle_vertices, 7, true);
    g->SetColor(Color::Black);
    for (int i = 0; i < 7; i++)
    {
        auto one = rectangle_vertices[i];
        auto two = rectangle_vertices[(i + 1) % 7];
        g->DrawLine(one.mX, one.mY, two.mX, two.mY);
    }
    g->SetFont(FONT_PICO129);
    g->DrawString(this->text, text_x, text_y + ascent);
    
    g->PopState();
}

void Admonition::SetText(std::string text)
{
    this->text = std::move(text);
}

void Admonition::SetArea(Sexy::Rect rect)
{
    this->area = rect;
}

void Admonition::SetAdmonishDownwards(bool admonish_downwards)
{
    this->admonish_downwards = admonish_downwards;
}

void Admonition::Update()
{
    this->timer--;
}

bool Admonition::IsDead()
{
    return this->timer <= 0;
}
