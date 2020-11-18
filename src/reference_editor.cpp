/*********************************************************************************
 *
 * $Id: reference_editor.cpp,v 1.6 1999/01/30 01:02:10 kiecza Exp $
 *
 *********************************************************************************/

#include "reference_editor.moc"

#include <qlabel.h>
#include <qframe.h>
#include <qlined.h>
#include <qpushbt.h>
#include <qlistbox.h>
#include <qdatetm.h> 
#include <qcheckbox.h>
#include <qfiledlg.h>
#include <qcolor.h>

#include <iostream.h>

#include "preprocessing.h"
#include "buffer.h"


ReferenceEditor::ReferenceEditor( Reference *_r, Preprocessing *_p, SoundBuffer *_b,
				  QWidget *parent=0, const char *name=0 ) : QDialog( parent, name, TRUE )
{
  setCaption("Reference Editor");

  setMinimumSize( 400, 370 );
  setMaximumSize( 400, 370 );
  resize(400, 370);
 
  QLabel *textLabel = new QLabel("Text:", this);
  textLabel->setGeometry(5,5, 60,25);
  textLabel->setAlignment( AlignVCenter | AlignRight );
  text = new QLineEdit(this, "entry_text");
  text->setGeometry(70,5, 300,25);

  QLabel *cmdLabel = new QLabel("Command:", this);
  cmdLabel->setGeometry(5,40, 60,25);
  cmdLabel->setAlignment( AlignVCenter | AlignRight );
  cmd = new QLineEdit(this, "entry_cmd");
  cmd->setGeometry(70,40, 300,25);

  QLabel *smpLabel = new QLabel("Sample Utterances:", this);
  smpLabel->setGeometry(10,80, 150,25);

  smp_listbox = new QListBox( this, "smp_listbox" );
  smp_listbox->setGeometry(12, 103, 200, 200);

  switch_autorecording_btn = new QPushButton("Enable Autorecording...", this, "switch_autorecording_btn");
  switch_autorecording_btn->setGeometry(220, 130, 160, 50);
  connect(switch_autorecording_btn,  SIGNAL(clicked()), this, SLOT(switch_autorecording()));

  default_button_palette = new QPalette(switch_autorecording_btn->palette());
  QColor highlight(255, 80, 80);
  action_button_palette  = new QPalette(highlight);

  QPushButton *delete_sample_utt = new QPushButton( "Delete", this, "delete_sample_utt" );
  delete_sample_utt->setGeometry(220, 220, 160, 50);
  connect(delete_sample_utt,  SIGNAL(clicked()), this, SLOT(delete_sample()));

  QPushButton *ok = new QPushButton( "Ok", this );
  ok->setGeometry(150, 330, 100, 30);
  connect( ok, SIGNAL(clicked()), this, SLOT(my_accept()) );
  
  reference = _r;
  preprocessing = _p;
  buffer = _b;

  connect(buffer, SIGNAL(end_detected()), this, SLOT(stop_recording()));
  buffer->do_replay(true);

  fdialog  = new QFileDialog(NULL, "", NULL, "dialog", TRUE);

  browse_btn = new QPushButton("...", this, "browse_btn");
  browse_btn->setGeometry(375, 43, 19, 19);
  connect(browse_btn,  SIGNAL(clicked()), this, SLOT(browse_command()));

  // ***** fill in data from reference ......

  text->setText(reference->get_name());
  cmd->setText(reference->get_command());
  for (uint i = 0 ; i < reference->count() ; i++)
    smp_listbox->insertItem(*(reference->get_sample_name(i)));
}


ReferenceEditor::~ReferenceEditor()
{
  delete text;
  delete cmd;
  delete smp_listbox;
  delete fdialog;
  delete browse_btn;
  delete switch_autorecording_btn;
  delete default_button_palette;
  delete action_button_palette;
}


void ReferenceEditor::stop_recording()
{
  QDateTime dt = QDateTime::currentDateTime();
  QString name = dt.toString();
  name.prepend("[");
  name.append("]");

  smp_listbox->insertItem(name);
  smp_listbox->setCurrentItem(smp_listbox->count()-1);
  
  reference->append_sample_utterance(preprocessing->preprocess_utterance(buffer->get_data(), 
									 buffer->get_size()));
  disable_autorecording();
}


void ReferenceEditor::delete_sample()
{
  // ***** delete actual list box entry if one selected

  if (-1 != smp_listbox->currentItem())
  {
    uint idx = smp_listbox->currentItem();
    smp_listbox->removeItem(idx);
    reference->remove_sample_utterance(idx);
  }
}


void ReferenceEditor::switch_autorecording()
{
  // ***** switch to "listen to microphone"-mode

  if (!buffer->is_detect_mode())
  {
    switch_autorecording_btn->setText("Disable Autorecording...");
    switch_autorecording_btn->setPalette( *action_button_palette );
    buffer->detect_speech(true);
  }
  else
  {
    switch_autorecording_btn->setText("Enable Autorecording...");
    switch_autorecording_btn->setPalette( *default_button_palette );
    buffer->detect_speech(false);
  }
}


void ReferenceEditor::disable_autorecording()
{
  buffer->detect_speech(false);
  switch_autorecording_btn->setText("Enable Autorecording...");
  switch_autorecording_btn->setPalette( *default_button_palette );
}


void ReferenceEditor::browse_command()
{
  if (buffer->is_detect_mode())
  {
    switch_autorecording();
  }

  QString command = fdialog->getOpenFileName(NULL, "*", NULL, "fdialog");

  if (!command.isEmpty())
  {
    if (strlen(cmd->text())>0)
    {
      QString new_command(cmd->text());
      new_command.append(" ; ");
      new_command.append(command);
      cmd->setText(new_command);
    }
    else
      cmd->setText(command);
  }
}


void ReferenceEditor::my_accept()
{
  // ***** store data into reference ....

  buffer->detect_speech(false);
  switch_autorecording_btn->setPalette( *default_button_palette );
  buffer->do_replay(false);
  disconnect(buffer, SIGNAL(end_detected()), this, SLOT(stop_recording()));
  
  reference->set_name(text->text());
  reference->set_command(cmd->text());

  reference->reset_sample_names();
  for ( uint i = 0; i < smp_listbox->count(); i++)
    reference->append_sample_name(smp_listbox->text(i));

  accept();
}


