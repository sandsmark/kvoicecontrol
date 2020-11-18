/*********************************************************************************
 *
 * $Id: reference_editor.h,v 1.4 1999/01/30 01:02:02 kiecza Exp $
 *
 *********************************************************************************/

#ifndef REFERENCEEDITOR_H
#define REFERENCEEDITOR_H

#include <qdialog.h>
//#include <qstring.h>
#include <qlist.h>

class QPushButton;
class QListBox;
class QLineEdit;
class Preprocessing;
class SoundBuffer;
class QFileDialog;
class QColor;

#include "reference.h"

class ReferenceEditor : public QDialog
{
  Q_OBJECT

public:

  ReferenceEditor( Reference *_r, Preprocessing *_p, SoundBuffer *_b,
		   QWidget *parent=0, const char *name=0);
  ~ReferenceEditor();
 
public slots:

  void stop_recording();
  void delete_sample();
  void my_accept();
  void switch_autorecording();
  void disable_autorecording();
  void browse_command();

private:

  QList<Utterance> *smp_list;
  QListBox *smp_listbox;

  QFileDialog *fdialog;
  QPushButton *browse_btn;

  QLineEdit *text;
  QLineEdit *cmd;

  QPushButton *switch_autorecording_btn;
  QPalette    *default_button_palette;
  QPalette    *action_button_palette;

  Reference     *reference;
  Preprocessing *preprocessing;
  SoundBuffer   *buffer;
};

#endif
