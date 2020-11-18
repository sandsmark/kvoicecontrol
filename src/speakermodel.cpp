/*********************************************************************************
 *
 * $Id: speakermodel.cpp,v 1.17 1999/01/30 01:13:19 kiecza Exp $
 *
 *********************************************************************************/

#include <unistd.h>    

#include <qfiledlg.h>
#include <qpushbt.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qfile.h>
#include <qdstream.h>
#include <qdatetm.h> 
#include <qmsgbox.h>
#include <stdlib.h>

#include "speakermodel.moc"

#include<iostream.h>

#include "reference.h"
#include "reference_editor.h"
#include "preprocessing.h"
#include "buffer.h"
#include "score.h"
#include "options.h"
#include "train_dialog.h"

#include "docking.h"
#include "kvoicecontrol.h"
extern DockWidget *dock_widget;

SpeakerModel::SpeakerModel(KConfig *config, 
			   QWidget *parent, const char *name) : QWidget( parent, name )
{
  resize(300, 200);
  
  ref_listbox = new QListBox( this, "ref_listbox" );
  ref_listbox->setGeometry(0, 0, 200, 200);
  ref_list = new QList<Reference>();
  ref_list->setAutoDelete(true);
  
  new_reference_btn = new QPushButton( "New", this, "new_reference" );
  new_reference_btn->setGeometry(220, 35, 60, 30);
  connect(new_reference_btn,  SIGNAL(clicked()), this, SLOT(new_reference()));
  edit_reference_btn = new QPushButton( "Edit ...", this, "edit_reference" );
  edit_reference_btn->setGeometry(220, 90, 60, 30);
  connect(edit_reference_btn,  SIGNAL(clicked()), this, SLOT(edit_reference()));
  delete_reference_btn = new QPushButton( "Delete", this, "delete_reference" );
  delete_reference_btn->setGeometry(220, 145, 60, 30);
  connect(delete_reference_btn,  SIGNAL(clicked()), this, SLOT(delete_reference()));

  fdialog  = new QFileDialog(NULL, ".spk", NULL, "dialog", TRUE);

  preprocessing= new Preprocessing();
  score        = new Score();
  buffer       = new SoundBuffer();

  led_timer = new QTimer( this );
  connect( led_timer, SIGNAL(timeout()), dock_widget, SLOT(toggle_led_recognition()) );

  do_detect = false;
  changed = false;

  // ***** load configuration
  // ***** not working yet !?

  QString s; 
  QString oldgroup = config->group();
  config->setGroup("Sound");
  s = config->readEntry("RecLevelThreshold", "10");
  buffer->set_rec_level_threshold(s.toInt());
  s = config->readEntry("AcceptedSilenceFrames", "4");
  buffer->set_accept_silence(s.toInt());
  config->setGroup("Recognizer");
  s = config->readEntry("AdjustmentWindowWidth", "70");
  score->set_adjust_width(s.toInt());
  s = config->readEntry("ScoreThreshold", "15.0" );
  score->set_threshold(s.toFloat());
  s = config->readEntry("MinimumScoreDistance", "2.4" );
  score->set_min_distance(s.toFloat());
  config->setGroup( oldgroup );

  
  options_dlg = new Options();

  connect(options_dlg, SIGNAL(reject_thresh_changed(float)), 
	  score, SLOT(set_threshold(float)));
  connect(options_dlg, SIGNAL(min_dist_changed(float)), 
	  score, SLOT(set_min_distance(float)));
  connect(options_dlg, SIGNAL(adj_win_width_changed(int)), 
	  score, SLOT(set_adjust_width(int)));

  connect(options_dlg, SIGNAL(acc_sil_frames_changed(int)), 
	  buffer, SLOT(set_accept_silence(int)));
  connect(options_dlg, SIGNAL(rec_level_thresh_changed(int)), 
	  buffer, SLOT(set_rec_level_threshold(int)));

}


SpeakerModel::~SpeakerModel()
{
  delete filename;
  delete fdialog;

  delete preprocessing;
  delete score;
  delete buffer;

  delete ref_listbox;
  delete ref_list;
}

/* -------------------------------------------------------------------------------- */

bool SpeakerModel::reset()
{
  bool ok = true;

  if (has_changed())
    ok = ask_save_changes();
  else
    ok = true;

  if (ok)
  {
    detect_mode_off();
    
    filename = 0;
    ref_listbox->clear();
    ref_list->clear();
    
    changed = false;
    
    emit new_title(QString("KVoiceControl: [none]"));

    return true;
  }

  return false;
}


void SpeakerModel::import()
{
  load(NULL, false);
}


void SpeakerModel::load()
{
  load(NULL, true);
}


void SpeakerModel::load(char *f=0, bool reset_first)
{
  detect_mode_off();
  
  QString filename_tmp;

  if (f == 0)
    filename_tmp = fdialog->getOpenFileName(NULL, "*.spk", NULL, "fdialog");
  else
    filename_tmp = QString(f);

  //cerr << "F " << filename_tmp << endl;
  
  if (filename_tmp != NULL)
  {
    if (!reset_first || reset())
    {
      //filename = new QString(filename_tmp);
      QFile file(filename_tmp);
      file.open( IO_ReadOnly );   
      QDataStream s( &file );     
      uint count;
      s >> count;
      for (uint count_tmp=0; count_tmp < count; count_tmp++)
      {
	Reference *r = new Reference();
	s >> r;
	ref_listbox->insertItem(r->get_name());
	ref_listbox->setCurrentItem(ref_listbox->count()-1);
	ref_list->append(r);
      }
      file.close();               
      
      if (reset_first)
      {	
	filename = new QString(filename_tmp);
	QString title = filename->right(filename->length()-filename->findRev('/')-1);
	title.prepend("KVoiceControl: ");
	emit new_title(title);
      }
    }
  }
}


void SpeakerModel::save()
{
  detect_mode_off();

  if (NULL == filename)
    save_as();
  else
  {
    QFile file(*filename);

    file.open( IO_WriteOnly );   
    QDataStream s( &file );     

    s << ref_list->count();

    for (uint count=0; count < ref_list->count(); count++)
    {
      s << *(ref_list->at(count));
    }

    changed = false;

    file.close();               
  }
}

 
void SpeakerModel::save_as()
{
  detect_mode_off();

  QString filename_tmp = fdialog->getSaveFileName(NULL, "*.spk", NULL, "fdialog");

  if (filename_tmp != NULL)
  {
    filename = new QString(filename_tmp);
    if (filename->right(4) != ".spk")
      filename->append(".spk");
    
    save();

    QString title = filename->right(filename->length()-filename->findRev('/')-1);
    title.prepend("KVoiceControl: ");
    emit new_title(title);
  }
}


void SpeakerModel::detect_mode_on()
{
  if (!do_detect && check_references())  
  {
    if (!buffer->detect_speech(true))
      return;

    connect(buffer, SIGNAL(end_detected()), this, SLOT(test_utterance()));
    do_detect = true;
    emit detect_mode_changed(true);
    connect(buffer, SIGNAL(recording_active()), dock_widget, SLOT(toggle_led_record()));
    dock_widget->led_record_on();
    KApplication::flushX();
    kapp->processEvents();
  }
}


void SpeakerModel::detect_mode_off()
{
  if (do_detect)
  {
    do_detect = false;
    
    disconnect(buffer, SIGNAL(end_detected()), this, SLOT(test_utterance()));
    buffer->detect_speech(false);
    
    emit detect_mode_changed(false);
    disconnect(buffer, SIGNAL(recording_active()), dock_widget, SLOT(toggle_led_record()));
    dock_widget->led_record_off();
    KApplication::flushX();
    kapp->processEvents();
  }
};


void SpeakerModel::toggle_detect_mode()
{
  dock_widget->led_recognition_off();
  KApplication::flushX();
  kapp->processEvents();

  if (do_detect)
    detect_mode_off();
  else
    detect_mode_on();
}


void SpeakerModel::test_utterance()
{
  detect_mode_off();
  dock_widget->led_recognition_on();
  led_timer->start(150);
  KApplication::flushX();
  kapp->processEvents();

  Utterance *test_utterance = preprocessing->preprocess_utterance(buffer->get_data(), 
								  buffer->get_size());

  // ***** branch&bound-score is by factor HELL faster ! ;-)
  //QString *result = score->score(test_utterance, ref_list);
  //QDateTime dt = QDateTime::currentDateTime();
  //cerr << dt.toString() << endl;
  QString *result = score->branchNbound_score(test_utterance, ref_list);
  //dt = QDateTime::currentDateTime();
  //cerr << dt.toString() << endl;

  led_timer->stop();
  
  if (NULL == result)
  {
    dock_widget->led_recognition_off();
    detect_mode_on();
  }
  else
  {
    QString command = result->copy();
    delete result;

    dock_widget->led_recognition_success();

    KApplication::flushX();
    kapp->processEvents();

    command = command.stripWhiteSpace();
    execute_command(command);
  }
}


/* -------------------------------------------------------------------------------- */

/*---------------------------------------*/
/* test_command                          */
/* params: command = string to work on   */
/*         cname   = command to look for */
/* purpose: helper to look for special   */
/*          commands and handle them     */
/*---------------------------------------*/

QString SpeakerModel::test_command(QString command, QString command_name)
{
   int start;

   //#ifdef DEBUG
   // /*debug*/printf("test_command(\"%s\",\"%s\")\n",(const char *)command,(const char *)command_name);    
   //#endif

   start = command.find(command_name, 0, FALSE);

   if (start < 0) 
     return(NULL);			  // ***** given command not found
   
   if (start > 1) 
     execute_command(command.left(start));	  // ***** if other commands should be executed before
   
   end = command.find(";", start, FALSE); // ***** find the end of the command
   
   start = start + command_name.length();

   if (end < 0)
   {
     return(command.right(command.length() - start));	// ***** return their arguments
   }
   else
   {
     return(command.mid(start, end - start));
   }
};


#define IFCOMMAND(cmd) args = test_command(command,cmd) ; if (!args.isEmpty())
#define NEXT_COMMAND   if ((end > 0) && (((command.length() - end) - 1) > 0)) execute_command(command.right((command.length() - end) - 1)); return;

/*---------------------------------------*/
/*  execute_command                         */
/* params: command = string to work on   */
/* purpose: just what the name says      */
/*---------------------------------------*/

void SpeakerModel::execute_command(QString command)
 {
   QString args;

   //#ifdef DEBUG
   // /*debug*/printf("execute_command(%s)\n",(const char *)command);  
   //#endif
   
   if(command.isEmpty()) return;
  
   // ***** opend a file named index.spk in the given dir. 
   // ***** this makes ,A4(Bcommand-trees,A4(B like 
   // ***** "[get [time | date] | sleep | start [editor | terminal] ]"  possible

  IFCOMMAND("openDir=")		
  {
    if (chdir(((const char *)args)) == 0 )
    {
      load("index.spk", true);
    }
    else
    {
      printf("could not find directory!\n");
    };
    
    detect_mode_on();
    NEXT_COMMAND 
  };
  
  // ***** open another spk-file

  IFCOMMAND("openFile=")	   
  {
    load((char *)((const char *)args), true);

    detect_mode_on();
    NEXT_COMMAND
  };
  
  // *****beware! the old dir will not be in the search path any more

  IFCOMMAND("appendDir=")
  {
    if(chdir(((const char *)args)) == 0 )
      load("index.spk",false);

    detect_mode_on();
    NEXT_COMMAND
  };

  IFCOMMAND("appendFile=")
  {
    load((char *)((const char *)args),false);

    detect_mode_on();
    NEXT_COMMAND
  };

  IFCOMMAND("@")
  {
    detect_mode_off();
    system((const char *)args);

    detect_mode_on();
    NEXT_COMMAND 
  };
 
  // ***** it,A4(Bs just one of a few special commands now, the single ,A4(Bf,A4(B in off IS NO TYPO
  // ***** *Daniel* but we use it with two 'f's now!

  IFCOMMAND("detectModeOff")
  {
    detect_mode_off();
    NEXT_COMMAND
  };

  // ***** execute it only in the Background if there aren,A4(Bt any special commands behind it

  cerr << "C: " << command << endl;

  if ((end <= 0) && (command.right(1) != "&") )
    command.append(" &");     
  system(command);
  detect_mode_on();
};



/* -------------------------------------------------------------------------------- */

bool SpeakerModel::check_references()
{
  for (uint i = 0 ; i < ref_list->count() ; i++)
  {
    if (ref_list->at(i) == 0)
    {
      char text[256];
      sprintf(text, "Reference number %d is empty!", i+1);
      QMessageBox::information( this, "Oops", text );
      return false;
    }
    if (ref_list->at(i)->count() < 2)
    {
      QString *text = new QString(ref_list->at(i)->get_name().copy());
      text->prepend("\"");
      text->append("\"\nneeds at least two utterances!!");
      QMessageBox::information( this, "Oops", *text );
      delete text;
      return false;
    }
  }
  if (ref_list->count() == 0)
  {
    QMessageBox::information( this, "Oops", "No references !!" );
    return false;
  }
  
  return true;
}


void SpeakerModel::new_reference()
{
  // ***** edit a new reference

  if (!ref_list->contains(NULL))
  {
    ref_listbox->insertItem("untitled");
    ref_listbox->setCurrentItem(ref_listbox->count()-1);
    ref_list->append(NULL);

    changed = true;
  }
}

void SpeakerModel::edit_reference()
{
  // ***** edit actual list box entry if one selected

  if (-1 != ref_listbox->currentItem())
  {
    if (do_detect)
      detect_mode_off();

    if (NULL == ref_list->at(ref_listbox->currentItem())) 
    {
      // ***** new reference
      
      Reference *r = new Reference();

      ref_editor = new ReferenceEditor(r, preprocessing, buffer, this, "ref_editor");

      if (ref_editor->exec())
      {
	uint pos = ref_listbox->currentItem();
	
	ref_list->remove(pos);
	ref_list->insert(pos, r);
	ref_listbox->removeItem(pos);
	ref_listbox->insertItem(r->get_name(), pos);
	ref_listbox->setCurrentItem(pos);

	changed = true;
      }
      delete ref_editor;
    }
    else
    {
      // ***** existing reference

      ref_editor = new ReferenceEditor(ref_list->at(ref_listbox->currentItem()), 
				       preprocessing, 
				       buffer,
				       this, 
				       "ref_editor");
      
      if (ref_editor->exec())
      {
	uint pos = ref_listbox->currentItem();
	
	ref_listbox->removeItem(pos);
	ref_listbox->insertItem(ref_list->at(pos)->get_name(), pos);
	ref_listbox->setCurrentItem(pos);

	changed = true;
      }

      delete ref_editor;
    }
  }
}


void SpeakerModel::delete_reference()
{
  // ***** delete actual list box entry if one selected

  if (-1 != ref_listbox->currentItem())
  {
    uint idx = ref_listbox->currentItem();
    ref_listbox->removeItem(idx);
    ref_list->remove(idx);

    changed = true;
  }
}


void SpeakerModel::append_reference(Reference *r)
{
  // ***** add this reference (no copy, just the pointer!!)

  if (r != NULL)
  {
    ref_listbox->insertItem(r->get_name());
    ref_listbox->setCurrentItem(ref_listbox->count()-1);
    ref_list->append(r);

    changed = true;
  }
}


bool SpeakerModel::has_changed()
{
  return changed;
}


bool SpeakerModel::ask_save_changes()
{
  switch( QMessageBox::information( this, "KVoiceControl",
				    "The speakermodel has changed!\n"
				    "Do you want to save it before exiting?",
				    "&Save", "&Don't Save", "&Cancel",
				    0, 2 ) )
  {
  case 0: 
    save();
    return(true);
    break;
  case 1:
    return(true);
    break;
  case 2:
    return(false);
    break;
  }
  
  return(true);
}


void SpeakerModel::show_options()
{
  detect_mode_off();

  if (!options_dlg->isVisible())
  {
    // ***** fill in values

    options_dlg->set_rec_thresh(buffer->get_rec_level_threshold());
    options_dlg->set_acc_sil_frames(buffer->get_acc_sil_frames());

    options_dlg->set_adj_win_width(score->get_adjust_win_width());
    options_dlg->set_reject_thresh(score->get_threshold());
    options_dlg->set_min_dist(score->get_min_distance());

    options_dlg->show();
  }
}


void SpeakerModel::train_references()
{
  detect_mode_off();

  QString refslist_file = fdialog->getOpenFileName(NULL, "*.txt", NULL, "fdialog");
  
  if (refslist_file != NULL)
  {
    TrainDialog *traindialog = new TrainDialog(refslist_file, this, preprocessing, 
					       buffer, this,  "train_dialog");
    //traindialog->exec();
    delete traindialog;
  }
}


void SpeakerModel::calibrate_micro()
{
  detect_mode_off();

  buffer->calibrate_micro();
}


void SpeakerModel::save_config(KConfig *config)
{
  // ***** save configuration
  // ***** not working yet !?

  QString oldgroup = config->group();
  config->setGroup("Sound");
  config->writeEntry("RecLevelThreshold", buffer->get_rec_level_threshold());
  config->writeEntry("AcceptedSilenceFrames", buffer->get_acc_sil_frames());
  config->setGroup("Recognizer");
  config->writeEntry("AdjustmentWindowWidth", score->get_adjust_win_width() );  
  QString s;
  s.sprintf("%f", score->get_threshold());
  config->writeEntry("ScoreThreshold", s );
  s.sprintf("%f", score->get_min_distance());
  config->writeEntry("MinimumScoreDistance", s );
  config->setGroup( oldgroup );
}

