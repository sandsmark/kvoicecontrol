/*********************************************************************************
 *
 * $Id: speakermodel.h,v 1.12 1999/01/30 01:13:13 kiecza Exp $
 *
 *********************************************************************************/

#ifndef SPEAKERMODEL_H
#define SPEAKERMODEL_H

#include <qwidget.h>
#include <qstring.h>
#include <qfile.h>
#include <qlist.h>
#include <kconfig.h>
#include <qtimer.h>

#include <kapp.h>

class QFileDialog;
class QListBox;
class QPushButton;

class ReferenceEditor;
class Preprocessing;
class SoundBuffer;
class Score;
class Options;

#include "reference.h"


class SpeakerModel : public QWidget
{
  Q_OBJECT

public:

  SpeakerModel(KConfig *config, QWidget *parent=0, const char *name=0);
  ~SpeakerModel();

  bool has_changed();
  bool ask_save_changes();
  bool check_references();

  void save_config(KConfig *);

  void load(char *f=0, bool reset_first=true);

public slots:

  bool reset();
  void load();
  void import();
  void save();
  void save_as();
  
  void test_utterance();

  void new_reference();
  void edit_reference();
  void delete_reference();

  void append_reference(Reference *);

  void detect_mode_on();
  void detect_mode_off();
  void toggle_detect_mode();

  void show_options();

  void train_references();

  void calibrate_micro();
  
  //void set_min_distance(float val) {min_distance = val;};

signals:
  
  void new_title(QString title);
  void detect_mode_changed(bool detect);

private:

  QString test_command(QString command, QString cname);
  void    execute_command(QString command);
  int     end;

  QString          *filename;
  QFileDialog      *fdialog;

  QListBox         *ref_listbox;
  QList<Reference> *ref_list;

  ReferenceEditor  *ref_editor;

  Score            *score;
  Preprocessing    *preprocessing;
  SoundBuffer      *buffer;

  bool             do_detect;
  bool             changed;

  Options          *options_dlg;

  QTimer           *led_timer;

  QPushButton *new_reference_btn;
  QPushButton *edit_reference_btn;
  QPushButton *delete_reference_btn;
};

#endif

