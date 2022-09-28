#include <Xm/Xm.h>
#include "UIComponent.h"
#include "Cmd.h"
#include "AttachArea.h"
#include "DialogManager.h"

#ifndef ATTACHADDCMD_H
#define ATTACHADDCMD_H

typedef enum { OK, ERR, CONFIRM } SaveFileState;

class AttachAddCmd : public Cmd {
    
private:

    Widget _clipWindow;
    Widget _parent;
    void cancel( XtPointer );	// Called when the Cancel button is pressed
    void ok( Widget, XtPointer );// Called when the OK button is pressed
    void add_file(char *);
    AttachArea *_attachArea;

    static void okCallback( Widget, XtPointer, XtPointer );
    static void cancelCallback( Widget, XtPointer, XtPointer );

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    AttachAddCmd ( AttachArea *, Widget, Widget, char *, int );

    virtual const char *const className () { return "AttachAddCmd"; }
};
#endif

#ifndef ATTACHFETCHCMD_H
#define ATTACHFETCHCMD_H

class AttachFetchCmd : public Cmd {
    
private:

    Widget _clipWindow;
    AttachArea *_attachArea;

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    AttachFetchCmd ( AttachArea *, char *, int );

    virtual const char *const className () { return "AttachFetchCmd"; }
};
#endif


#ifndef ATTACHDELETECMD_H
#define ATTACHDELETECMD_H

class AttachDeleteCmd : public Cmd {
    
private:

    AttachArea *_attachArea;

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    AttachDeleteCmd ( AttachArea *, char *, int );

    virtual const char *const className () { return "AttachDeleteCmd"; }
};
#endif


#ifndef ATTACHOPENCMD_H
#define ATTACHOPENCMD_H

class AttachOpenCmd : public Cmd {
    
private:

    Widget _clipWindow;
    void cancel( XtPointer );	// Called when the Cancel button is pressed
    void ok( XtPointer );	// Called when the OK button is pressed

    static void okCallback( Widget, XtPointer, XtPointer );
    static void cancelCallback( Widget, XtPointer, XtPointer );
    AttachArea *_attachArea;

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    AttachOpenCmd ( AttachArea *, char *, int );

    virtual const char *const className () { return "AttachOpenCmd"; }
};
#endif


#ifndef ATTACHRENAMECMD_H
#define ATTACHRENAMECMD_H

class AttachRenameCmd : public Cmd {
    
private:

    AttachArea *_attachArea;
    void cancel( XtPointer );	// Called when the Cancel button is pressed
    static void cancelCallback( Widget, XtPointer, XtPointer );
    void ok( XtPointer );	// Called when the OK button is pressed
    static void okCallback( Widget, XtPointer, XtPointer );

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    AttachRenameCmd ( AttachArea *, Widget, char *, int );

    virtual const char *const className () { return "AttachRenameCmd"; }
};
#endif

#ifndef ATTACHDESCRIPTIONCMD_H
#define ATTACHDESCRIPTIONCMD_H

class AttachDescriptionCmd : public Cmd {
    
private:

    AttachArea *_attachArea;
    void cancel( XtPointer );	// Called when the Cancel button is pressed
    static void cancelCallback( Widget, XtPointer, XtPointer );
    void ok( XtPointer );	// Called when the OK button is pressed
    static void okCallback( Widget, XtPointer, XtPointer );

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    AttachDescriptionCmd ( AttachArea *, Widget, char *, int );

    virtual const char *const className () { return "AttachDescriptionCmd"; }
};
#endif


#ifndef ATTACHSAVEASCMD_H
#define ATTACHSAVEASCMD_H

class AttachSaveAsCmd : public Cmd {
    
private:

    Widget _parent;
    Widget _clipWindow;
    void cancel( XtPointer );	// Called when the Cancel button is pressed
    void ok( XtPointer );	// Called when the OK button is pressed
    AttachArea *_attachArea;

    static void okCallback( Widget, XtPointer, XtPointer );
    static void cancelCallback( Widget, XtPointer, XtPointer );

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    AttachSaveAsCmd ( AttachArea *, Widget, Widget, char *, int );

    virtual const char *const className () { return "AttachSaveAsCmd"; }
};
#endif


#ifndef ATTACHSELECTALLCMD_H
#define ATTACHSELECTALLCMD_H

class AttachSelectAllCmd : public Cmd {
    
private:

    AttachArea *_attachArea;

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    AttachSelectAllCmd ( AttachArea *, char *, int );

    virtual const char *const className () { return "AttachSelectAllCmd"; }
};
#endif


#ifndef ATTACHUNDELETECMD_H
#define ATTACHUNDELETECMD_H

class AttachUndeleteCmd : public Cmd {
    
private:

    AttachArea *_attachArea;

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    AttachUndeleteCmd ( AttachArea *, char *, int );

    virtual const char *const className () { return "AttachUndeleteCmd"; }
};
#endif


#ifndef ATTACHUNSELECTALLCMD_H
#define ATTACHUNSELECTALLCMD_H

class AttachUnselectAllCmd : public Cmd {
    
private:

    AttachArea *_attachArea;

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    AttachUnselectAllCmd ( AttachArea *, char *, int );

    virtual const char *const className () { return "AttachUnselectAllCmd"; }
};
#endif

#ifndef ATTACHINFOCMD_H
#define ATTACHINFOCMD_H

class AttachInfoCmd : public Cmd {
    
private:

    AttachArea *_attachArea;
    Widget      _info_dialog;
    DialogManager *_attachInfoDialogManager;

  protected:
    
    virtual void doit();   
    virtual void undoit(); 

  public:
    
    AttachInfoCmd ( AttachArea *, char *, int );
    ~AttachInfoCmd();

    virtual const char *const className () { return "AttachInfoCmd"; }
};
#endif
