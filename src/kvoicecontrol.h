/*********************************************************************************
 *
 * $Id: kvoicecontrol.h,v 1.6 1999/01/02 08:14:59 kiecza Exp $
 *
 *********************************************************************************/

#ifndef KVOICECONTROL_H
#define KVOICECONTROL_H

#include <qpopmenu.h>
#include <kapp.h>
#include <kmenubar.h>
#include <ktopwidget.h>
#include <qrect.h>

#include "speakermodel.h"


class KVoiceControl : public KTopLevelWidget
{
  Q_OBJECT

public:

    KVoiceControl(char *speakerfile);
    ~KVoiceControl();

    SpeakerModel  *speakermodel;

public slots:

    void about();
    void help();
    void quit();

    void set_title(QString title);
    void set_detect(bool do_detect);

private:

    void create_menu();
    void closeEvent(QCloseEvent *e);

    KMenuBar      *main_menu;
    QPopupMenu    *file_menu;
    QPopupMenu    *rec_menu;
    QPopupMenu    *opt_menu;
    QPopupMenu    *help_menu;
    
    int detect_ID;

    QRect *menu_geometry;
};

#endif


