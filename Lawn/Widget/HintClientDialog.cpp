#include "HintClientDialog.h"

#include "AlmanacDialog.h"
#include "GameButton.h"
#include "../Board.h"
#include "../../SexyAppFramework/WidgetManager.h"
#include "../System/Music.h"
#include "../../Resources.h"
#include "../../Sexy.TodLib/TodStringFile.h"
#include "../../SexyAppFramework/APWrapper.h"

struct HintClientDialogPrivate
{
    std::vector<Hint> hints;
    int hoveredHint = -1;
    MusicTune music;
};

void DrawStringBetween(Graphics* g, const SexyString& theText, int thePosX, int thePosXLimit, int thePosY, Font* theFont, const Color& theColor)
{
    auto string = theText;
    auto max_width = thePosXLimit - thePosX - 10;
    auto width = theFont->StringWidth(theText);
    if (width > max_width)
    {
        while (width > max_width)
        {
            string = string.substr(0, string.length() - 1);
            width = theFont->StringWidth(string + "...");
        }
        TodDrawString(g, string + "...", thePosX, thePosY, theFont, theColor, DrawStringJustification::DS_ALIGN_LEFT);
    }
    else
    {
        TodDrawString(g, string, thePosX, thePosY, theFont, theColor, DrawStringJustification::DS_ALIGN_LEFT);
    }
}

HintClientDialog::HintClientDialog(LawnApp* theApp) : LawnDialog(theApp, DIALOG_ALMANAC, true, _S("Hints"),
                                                                 _S(""), _S(""), BUTTONS_NONE)
{
    d = new HintClientDialogPrivate();
    
    mApp = (LawnApp*)gSexyAppBase;
    mDrawStandardBack = false;
    TodLoadResources("DelayLoad_Almanac");
	TodLoadResources("DelayLoad_ChallengeScreen");
    LawnDialog::Resize(0, 0, BOARD_WIDTH, BOARD_HEIGHT);

    mCloseButton = new GameButton(HintClientDialog::ALMANAC_BUTTON_CLOSE);
    mCloseButton->SetLabel(_S("[CLOSE_BUTTON]"));
    mCloseButton->mButtonImage = Sexy::IMAGE_ALMANAC_CLOSEBUTTON;
    mCloseButton->mOverImage = Sexy::IMAGE_ALMANAC_CLOSEBUTTONHIGHLIGHT;
    mCloseButton->mDownImage = nullptr;
    mCloseButton->SetFont(Sexy::FONT_BRIANNETOD12);
    Color aColor = Color(42, 42, 90);
    mCloseButton->mColors[ButtonWidget::COLOR_LABEL] = aColor;
    mCloseButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = aColor;
    mCloseButton->Resize(676, 567, 89, 26);
    mCloseButton->mParentWidget = this;
    mCloseButton->mTextOffsetX = -8;
    mCloseButton->mTextOffsetY = 1;
    
    mNewHintButton = new GameButton(HintClientDialog::ALMANAC_BUTTON_INDEX);
    mNewHintButton->SetLabel("REQUEST HINT");
    mNewHintButton->mButtonImage = Sexy::IMAGE_ALMANAC_INDEXBUTTON;
    mNewHintButton->mOverImage = Sexy::IMAGE_ALMANAC_INDEXBUTTONHIGHLIGHT;
    mNewHintButton->mDownImage = nullptr;
    mNewHintButton->SetFont(Sexy::FONT_BRIANNETOD12);
    mNewHintButton->mColors[ButtonWidget::COLOR_LABEL] = aColor;
    mNewHintButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = aColor;
    mNewHintButton->Resize(32, 567, 164, 26);
    mNewHintButton->mParentWidget = this;
    mNewHintButton->mTextOffsetX = 8;
    mNewHintButton->mTextOffsetY = 1;

    mScrollbar = new ScrollbarWidget(mApp, ScrollbarMode::VERTICAL);
    mScrollbar->mScrollRange = 100;
    mScrollbar->Resize(BOARD_WIDTH - 28, 110, 8, BOARD_HEIGHT - 150);
    mScrollbar->mViewport = Rect(20, 110, BOARD_WIDTH - 40, BOARD_HEIGHT - 150);

    d->music = mApp->mMusic->mCurMusicTune;
    if (!mApp->mBoard || !mApp->mBoard->mPaused)
        mApp->mMusic->MakeSureMusicIsPlaying(MUSIC_TUNE_CHOOSE_YOUR_SEEDS);
    
    mScrollAmount = 0;
    
    mAPDataStorageValueChangedListener = mApp->mAP->AddDataStorageValueChangeListener([this](const std::string& key, const nlohmann::json& value)
    {
        this->UpdateHints();
        this->Update();
    });
    this->UpdateHints();
}

HintClientDialog::~HintClientDialog()
{
    if (d->music != MusicTune::MUSIC_TUNE_NONE)
    {
        mApp->mMusic->MakeSureMusicIsPlaying(d->music);
    }
    
    if (mCloseButton) delete mCloseButton;
	if (mNewHintButton)	delete mNewHintButton;
	if (mScrollbar)		delete mScrollbar;
    delete mAPDataStorageValueChangedListener;
    delete d;
}

void HintClientDialog::Update()
{
    mCloseButton->Update();
	mNewHintButton->Update();
    mScrollbar->Update();
    
    auto cursor = CURSOR_POINTER;

    if (mApp->mWidgetManager->mOverWidget)
    {
        if (mCloseButton->IsMouseOver() || mNewHintButton->IsMouseOver())
        {
            cursor = CURSOR_HAND;
        }
    }

    Widget::Update();

    if (!mScrollbar->mDisabled)
    {
    	float aScrollSpeed = 1 + abs(mScrollAmount) * 0.1f;
    	mScrollbar->mScrollValue = ClampFloat(mScrollbar->mScrollValue += mScrollAmount * aScrollSpeed, 0, mScrollbar->mScrollRange);
    	mScrollAmount *= 0.9f;
    }
    
    if (d->hoveredHint != -1)
    {
        auto hint = d->hints[d->hoveredHint];
        if (hint.receiving_player == mApp->mAP->MySlot() && !hint.found)
        {
            cursor = CURSOR_HAND;
        }
    }

    mApp->SetCursor(cursor);
    MarkDirty();
}

void HintClientDialog::AddedToManager(WidgetManager* theWidgetManager)
{
    LawnDialog::AddedToManager(theWidgetManager);
    this->AddWidget(mScrollbar);
}

void HintClientDialog::RemovedFromManager(WidgetManager* theWidgetManager)
{
    LawnDialog::RemovedFromManager(theWidgetManager);
    this->RemoveWidget(mScrollbar);
}

void HintClientDialog::Draw(Graphics* g)
{
    g->SetLinearBlend(true);

    g->DrawImage(Sexy::IMAGE_CHALLENGE_BACKGROUND, 0, 0);
    TodDrawString(g, "Hints", BOARD_WIDTH / 2, 60, Sexy::FONT_HOUSEOFTERROR28, Color(220, 220, 220),
                  DrawStringJustification::DS_ALIGN_CENTER);


    mCloseButton->Draw(g);
    mNewHintButton->Draw(g);
    
    auto black = Color(0, 0, 0);
    auto black_gray = Color(0, 0, 0, 100);
    
    auto x_sender = 20;
    auto x_receiver = 100;
    auto x_item = 220;
    auto x_location = 400;
    auto x_status = 620;
    
    auto y = 110;
    
    TodDrawString(g, "Sender", x_sender, y, Sexy::FONT_TINYBOLD, black, DrawStringJustification::DS_ALIGN_LEFT);
    TodDrawString(g, "Receiver", x_receiver, y, Sexy::FONT_TINYBOLD, black, DrawStringJustification::DS_ALIGN_LEFT);
    TodDrawString(g, "Item", x_item, y, Sexy::FONT_TINYBOLD, black, DrawStringJustification::DS_ALIGN_LEFT);
    TodDrawString(g, "Location", x_location, y, Sexy::FONT_TINYBOLD, black, DrawStringJustification::DS_ALIGN_LEFT);
    TodDrawString(g, "Status", x_status, y, Sexy::FONT_TINYBOLD, black, DrawStringJustification::DS_ALIGN_LEFT);
    
    y += 18;
    y -= mScrollbar->mScrollValue;
    
    g->PushState();
    g->SetClipRect(Rect(20, 115, BOARD_WIDTH - 40, BOARD_HEIGHT - 150));
    for (auto hint : d->hints)
    {
        auto sender = mApp->mAP->PlayerDisplayName(hint.finding_player);
        auto receiver = mApp->mAP->PlayerDisplayName(hint.receiving_player);
        auto item = mApp->mAP->ItemName(hint.item, hint.receiving_player);
        auto location = mApp->mAP->LocationName(hint.location, mApp->mAP->PlayerGameName(hint.finding_player));
        
        std::string status = "Unspecified";
        if (hint.status == 10)
        {
            status = "No Priority";
        }
        else if (hint.status == 20)
        {
            status = "Avoid";
        }
        else if (hint.status == 30)
        {
            status = "Priority";
        }
        else if (hint.status == 40)
        {
            status = "Found";
        }
        
        
        auto fg = black;
        if (hint.found)
        {
            fg = black_gray;
        }
        
        
        DrawStringBetween(g, sender, x_sender, x_receiver, y, Sexy::FONT_PICO129, fg);
        DrawStringBetween(g, receiver, x_receiver, x_item, y, Sexy::FONT_PICO129, fg);
        DrawStringBetween(g, item, x_item, x_location, y, Sexy::FONT_PICO129, fg);
        DrawStringBetween(g, location, x_location, x_status, y, Sexy::FONT_PICO129, fg);
        DrawStringBetween(g, status, x_status, BOARD_WIDTH - 20, y, Sexy::FONT_PICO129, fg);

        y += 20;
    }
    g->PopState();

    if (!mScrollbar->mDisabled)
    {
    	mScrollbar->mButtonNoDraw = false;
    	g->PushState();
    	g->mTransX = g->mTransX + mScrollbar->mX;
    	g->mTransY = g->mTransY + mScrollbar->mY;
    	mScrollbar->Draw(g);
    	mScrollbar->mButtonNoDraw = true;
    	g->PopState();
    }
}

void HintClientDialog::MouseDown(int x, int y, int theClickCount)
{
    if (mCloseButton->IsMouseOver() || mNewHintButton->IsMouseOver())
        mApp->PlaySample(Sexy::SOUND_TAP);
    
    if(mScrollbar->isThumbOver())
    {
    	mScrollbar->MouseDown(x, y, theClickCount);
    }
}

void HintClientDialog::MouseMove(int x, int y)
{
    LawnDialog::MouseMove(x, y);
    
    d->hoveredHint = -1;
    
    // Hit test the table
    if (Rect(20, 115, BOARD_WIDTH - 40, BOARD_HEIGHT - 150).Contains(x, y))
    {
        auto x_sender = 20;
        auto x_receiver = 100;
        auto x_item = 220;
        auto x_location = 400;
        auto x_status = 620;
        
        // Find out which hint we are hovering over
        auto firstY = 113 - mScrollbar->mScrollValue;
        auto itemHeight = 20;
        
        auto yOffset = y - firstY;
        auto index = static_cast<int>(yOffset / itemHeight);
        if (index < 0) index = -1;
        if (index >= d->hints.size()) index = -1;
        
        if (index != -1)
        {
            d->hoveredHint = index;
        }
    }
}

void HintClientDialog::MouseUp(int x, int y, int theClickCount)
{
    if (mCloseButton->IsMouseOver()) mApp->KillHintClient();
    if (mNewHintButton->IsMouseOver())
    {
		mApp->DoDialog(Dialogs::DIALOG_INFO, true, "Create New Hint", "Use !hint in the text client to create a new hint!", "OK", Dialog::BUTTONS_FOOTER);
    }
    if (d->hoveredHint != -1)
    {
        auto hint = &d->hints[d->hoveredHint];
        if (hint->receiving_player == mApp->mAP->MySlot())
        {
            if (hint->status == 30) // Priority
            {
                mApp->mAP->SetNoPriorityHint(*hint);
                hint->status = 10;
		        mApp->PlaySample(Sexy::SOUND_TAP);
                
            }
            else if (hint->status == 10) // No Priority
            {
                mApp->mAP->SetAvoidHint(*hint);
                hint->status = 20;
		        mApp->PlaySample(Sexy::SOUND_TAP);
            }
            else
            {
                mApp->mAP->SetPriorityHint(*hint);
                hint->status = 30;
		        mApp->PlaySample(Sexy::SOUND_TAP);
            }
        }
    }
}

void HintClientDialog::MouseWheel(int theDelta)
{
    Widget::MouseWheel(theDelta);

    if (mScrollbar->mDisabled) return;

    mScrollAmount -= theDelta;
    mScrollAmount -= mScrollAmount * 0.1f;
}

void HintClientDialog::UpdateHints()
{
    d->hints = mApp->mAP->Hints();
    std::stable_sort(d->hints.begin(), d->hints.end(), [](const Hint& a, const Hint& b)
    {
        return b.found;
    });
    
    auto total_hint_height = d->hints.size() * 20;
    auto viewport_height = BOARD_HEIGHT - 150;
    auto required_scroll = total_hint_height - viewport_height;
    if (required_scroll < 0) required_scroll = 0;
    mScrollbar->mScrollRange = required_scroll;
}