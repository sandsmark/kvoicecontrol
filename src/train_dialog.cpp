/*********************************************************************************
 *
 * $Id: train_dialog.cpp,v 1.2 1999/01/02 08:07:41 kiecza Exp $
 *
 *********************************************************************************/

#include "train_dialog.moc"

#include <qlabel.h>
#include <qframe.h>
#include <qlined.h>
//#include <keditcl.h> 
#include <qpushbt.h>
#include <qlistbox.h>
#include <qdatetm.h> 
#include <qfile.h>
#include <qtstream.h>
#include <qmsgbox.h>

#include <iostream>

#include "preprocessing.h"
#include "buffer.h"

#include "speakermodel.h"


TrainDialog::TrainDialog( QString _f, SpeakerModel *_s, Preprocessing *_p, 
			  SoundBuffer *_b, QWidget *parent=0, const char *name=0 ) 
	       : QDialog( parent, name, TRUE )
{
  setCaption("Reference Trainer");

  setMinimumSize( 400, 370 );
  setMaximumSize( 400, 370 );
  resize(400, 370);
 
  text_label = new QLabel("Text:", this);
  text_label->setGeometry(5,5, 60,25);
  text_label->setAlignment( AlignVCenter | AlignRight );
  text = new QLineEdit(this, "entry_text");
  text->setGeometry(70,5, 300,25);

  cmd_label = new QLabel("Command:", this);
  cmd_label->setGeometry(5,40, 60,25);
  cmd_label->setAlignment( AlignVCenter | AlignRight );
  cmd = new QLineEdit(this, "entry_cmd");
  cmd->setGeometry(70,40, 300,25);

  smp_label = new QLabel("Sample Utterances:", this);
  smp_label->setGeometry(10,80, 150,25);
  smp_listbox = new QListBox( this, "smp_listbox" );
  smp_listbox->setGeometry(12, 103, 200, 200);

  delete_sample_utt = new QPushButton( "Delete", this, "delete_sample_utt" );
  delete_sample_utt->setGeometry(240, 143, 135, 50);
  connect(delete_sample_utt,  SIGNAL(clicked()), this, SLOT(delete_sample()));
  
  nav_begin = new QPushButton( "|<", this, "nav_begin" );
  nav_begin->setGeometry(240, 238, 25, 25);
  nav_down = new QPushButton( "<", this, "nav_down" );
  nav_down->setGeometry(265, 238, 25, 25);
  pos_entry = new KIntegerLine(this, "pos_entry");
  pos_entry->setGeometry(290, 238, 50, 25);
  nav_up = new QPushButton( ">", this, "nav_up" );
  nav_up->setGeometry(325, 238, 25, 25);
  nav_end   = new QPushButton( ">|", this, "nav_end" );
  nav_end->setGeometry(350, 238, 25, 25);

  connect (nav_begin, SIGNAL(clicked()),       this, SLOT(navigate_begin()));
  connect (nav_down,  SIGNAL(clicked()),       this, SLOT(navigate_left()));
  connect (pos_entry, SIGNAL(returnPressed()), this, SLOT(navigate_entry()));
  connect (nav_up,    SIGNAL(clicked()),       this, SLOT(navigate_right()));
  connect (nav_end,   SIGNAL(clicked()),       this, SLOT(navigate_end()));

  ok = new QPushButton( "OK", this );
  ok->setGeometry(60, 330, 100, 30);
  connect( ok, SIGNAL(clicked()), this, SLOT(my_accept()) );
  
  cancel = new QPushButton( "Cancel", this );
  cancel->setGeometry(240, 330, 100, 30);
  connect( cancel, SIGNAL(clicked()), this, SLOT(my_reject()) );
  
  reference = 0;
  position  = 0;
  pos_entry->setValue(position+1);
  
  speakermodel = _s;

  ref_list_local = new QList<Reference>();
  ref_list_local->setAutoDelete(false);

  preprocessing = _p;
  buffer = _b;

  if (load_refslist(_f))
  {
    connect(buffer, SIGNAL(end_detected()), this, SLOT(stop_recording()));
    buffer->detect_speech(true);
    buffer->do_replay(true);

    update_view();
    exec();
  }
}


TrainDialog::~TrainDialog()
{
  delete text;
  delete cmd;
  delete smp_listbox;
  delete text_label;
  delete cmd_label;
  delete smp_label;

  delete delete_sample_utt;
 
  delete nav_begin;
  delete nav_down;
  delete pos_entry;
  delete nav_up;
  delete nav_end;

  delete ok;
  delete cancel;

  //delete ref_list_local;
}


void TrainDialog::stop_recording()
{
  QDateTime dt = QDateTime::currentDateTime();
  QString name = dt.toString();
  name.prepend("[");
  name.append("]");

  smp_listbox->insertItem(name);
  smp_listbox->setCurrentItem(smp_listbox->count()-1);
  
  reference->append_sample_utterance(preprocessing->preprocess_utterance(buffer->get_data(), 
									 buffer->get_size()));
  buffer->detect_speech(true);
}


void TrainDialog::delete_sample()
{
  // ***** delete actual list box entry if one selected

  if (-1 != smp_listbox->currentItem())
  {
    uint idx = smp_listbox->currentItem();
    smp_listbox->removeItem(idx);
    reference->remove_sample_utterance(idx);
  }
}


bool TrainDialog::load_refslist(QString filename)
{
  bool error = false;
  QString message = "";  

  QFile file(filename);
  file.open( IO_ReadOnly );   
  QTextStream s( &file );     
  QString line = s.readLine();
  while (!s.eof())
  {
    if (line.contains('\t') != 1 || 
	line.find('\t') == 0 || 
	line.find('\t') == (int)line.length()-1)
    {
      message.append(line);
      message.append("\n");

      error = true;
    }
    else
    {
      int tab = line.find('\t');
      
      QString text    = line.left(tab).stripWhiteSpace();
      QString command = line.right(line.length()-tab-1).stripWhiteSpace();
      
      Reference *r = new Reference();
      r->set_name(text);
      r->set_command(command);
      ref_list_local->append(r);
    }
    
    line = s.readLine();
  }

  file.close();               

  if (error)
  {
    // ***** tell about problems loading the file .....

    message.prepend("The following lines were skipped:\n\n" );
    message.append("\n\nUse this format per line:  name<TAB>command\n");

    if (ref_list_local->isEmpty())
    {
      QMessageBox::information( this, "File Format Error", "No valid entries found!" );    
      return false;
    }
    else
      QMessageBox::information( this, "File Format Error", message );
  }
  else if (ref_list_local->isEmpty())
  {
    QMessageBox::information( this, "File Format Error", "No valid entries found!" );    
    return false;
  }
  
  position = 0;
  reference = ref_list_local->at(0);
  
  return true;
}


void TrainDialog::navigate_begin()
{
  navigate(0);
}

void TrainDialog::navigate_left()
{
  navigate(position-1);
}

void TrainDialog::navigate_entry()
{
  navigate(pos_entry->value()-1);
}

void TrainDialog::navigate_right()
{
  navigate(position+1);
}

void TrainDialog::navigate_end()
{
  navigate(ref_list_local->count()-1);
}


void TrainDialog::update_view()
{
  text->setText(reference->get_name());
  cmd->setText(reference->get_command());

  if (smp_listbox->count() > 0)
    smp_listbox->clear();
  for (uint i = 0 ; i < reference->count() ; i++)
    smp_listbox->insertItem(*(reference->get_sample_name(i)));

  pos_entry->setValue(position+1);
}


void TrainDialog::navigate(uint pos)
{
  if (pos >= ref_list_local->count())
    return;
  
  // ***** store data of actually visible reference

  reference->set_name(text->text());
  reference->set_command(cmd->text());
  reference->reset_sample_names();
  for ( uint i = 0; i < smp_listbox->count(); i++)
    reference->append_sample_name(smp_listbox->text(i));

  // ***** navigate to new reference

  position = pos;
  reference = ref_list_local->at(position);
  
  update_view();
}


void TrainDialog::my_accept()
{
  // ***** close sound buffer

  buffer->detect_speech(false);
  buffer->do_replay(false);
  disconnect(buffer, SIGNAL(end_detected()), this, SLOT(stop_recording()));

  // ***** store data of actually visible reference

  reference->set_name(text->text());
  reference->set_command(cmd->text());
  reference->reset_sample_names();
  for ( uint i = 0; i < smp_listbox->count(); i++)
    reference->append_sample_name(smp_listbox->text(i));

  // ***** add all new references to speakermodel

  for ( uint i = 0; i < ref_list_local->count(); i++)
    speakermodel->append_reference(ref_list_local->at(i));

  accept();
}


void TrainDialog::my_reject()
{
  // ***** close sound buffer

  buffer->detect_speech(false);
  buffer->do_replay(false);
  disconnect(buffer, SIGNAL(end_detected()), this, SLOT(stop_recording()));

  // ***** delete all references

  for ( uint i = 0; i < ref_list_local->count(); i++)
    delete ref_list_local->at(i);

  reject();
}
