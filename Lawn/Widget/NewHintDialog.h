#ifndef __NEWHINTDIALOG_H__
#define __NEWHINTDIALOG_H__

#include "LawnDialog.h"
#include "../../SexyAppFramework/EditListener.h"

class ListenerHandle;
class Admonition;

class NewHintDialog : public LawnDialog, public EditListener
{
public:
    LawnApp*			mApp;
    LawnEditWidget*			mHintEditWidget;
    
    bool mProcessing;
    Admonition* mAdmonition;
    ListenerHandle* mUserCommandListener;
    ListenerHandle* mDataStorageListener;

public:
    NewHintDialog(LawnApp* theApp);
    virtual ~NewHintDialog();

    virtual int			GetPreferredHeight(int theWidth);
    virtual void		Resize(int theX, int theY, int theWidth, int theHeight);
    virtual void		AddedToManager(WidgetManager* theWidgetManager);
    virtual void		RemovedFromManager(WidgetManager* theWidgetManager);
    virtual void		Draw(Graphics* g);
    virtual void		EditWidgetText(int theId, const SexyString& theString);
    virtual bool		AllowChar(int, SexyChar theChar);
    void ButtonDepress(int theId) override;
    void Update() override;
};

#endif
