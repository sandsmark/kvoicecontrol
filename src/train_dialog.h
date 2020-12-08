/*********************************************************************************
 *
 * $Id: train_dialog.h,v 1.2 1999/01/02 08:14:13 kiecza Exp $
 *
 *********************************************************************************/

#ifndef TRAINDIALOG_H
#define TRAINDIALOG_H

#include <qdialog.h>
//#include <qstring.h>
#include <qlist.h>
#include <kintegerline.h>

class QLabel;
class QPushButton;
class QListBox;
class QLineEdit;
class Preprocessing;
class SoundBuffer;
class SpeakerModel;

#include "reference.h"

class TrainDialog : public QDialog
{
    Q_OBJECT

public:

    TrainDialog(QString _f, SpeakerModel *_s, Preprocessing *_p, SoundBuffer *_b,
                QWidget *parent = 0, const char *name = 0);
    ~TrainDialog();

public slots:

    void stop_recording();
    void delete_sample();

protected slots:

    void my_accept();
    void my_reject();

protected slots:

    void navigate_begin();
    void navigate_left();
    void navigate_entry();
    void navigate_right();
    void navigate_end();

private:

    void navigate(uint);

    bool load_refslist(QString);
    void update_view();

    //QList<Utterance> *smp_list;
    //QString *refslist_file;

    SpeakerModel     *speakermodel;
    QList<Reference> *ref_list_local;

    QListBox  *smp_listbox;
    QLabel    *smp_label;

    QLineEdit *text;
    QLabel    *text_label;
    QLineEdit *cmd;
    QLabel    *cmd_label;

    QPushButton *delete_sample_utt;

    QPushButton  *nav_begin;
    QPushButton  *nav_down;
    KIntegerLine *pos_entry;
    QPushButton  *nav_up;
    QPushButton  *nav_end;

    QPushButton *ok;
    QPushButton *cancel;

    Reference     *reference;
    uint          position;

    Preprocessing *preprocessing;
    SoundBuffer   *buffer;
};

#endif


