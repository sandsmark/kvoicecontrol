/*********************************************************************************
 *
 * $Id: kvoicecontrol.cpp,v 1.17 1999/01/02 21:45:19 kiecza Exp $
 *
 *********************************************************************************/

#include "qtpamainloop.h"
#include <qkeycode.h>
#include <qaccel.h>
#include <kiconloader.h>
#include <klocale.h>
#include <qmsgbox.h>
#include <kconfig.h>
#include <qrect.h>
#include <qfile.h>
#include <qdir.h>

#include "docking.h"

#include "kvoicecontrol.moc"

#include <iostream>
#include <stdlib.h>
#include <unistd.h>

#define _(TEXT) klocale->translate(TEXT)

KApplication  *globalKapp;
KIconLoader   *globalKIL;
KVoiceControl *kvoicecontrol;
DockWidget    *dock_widget;
QString       lockFile;

using std::cerr;

bool createLockFile(QFile *file)
{
  if (file->open( IO_WriteOnly ))
  {
    QTextStream t( file );
    t << getpid() << endl;
    file->close();               
    return(true);
  }
  else
  {
    // ***** could not write PID file!

    return(false);
  }
}


int main(int argc, char **argv)
{
  if (argc != 1 && argc != 2)
  {
    cerr << "Usage: " << *argv[0] << " [<speakerfile.spk>]" << endl;
    exit(1);
  }

  // ***** create the lock file direcotry and check whether another instance
  //       of KVoiceControl is already running

  lockFile = globalKapp->localkdedir().data(); 
  lockFile += "/share/apps/kvoicecontrol";
  QDir dir(lockFile.data());
  if ( !dir.exists() )
    if ( ! dir.mkdir(lockFile.data(), TRUE) )
    {
       cerr << "Could not create the directory " << lockFile << endl;
       exit(1);
    }
  lockFile += "/kvoicecontrol.pid";
  QFile *file = new QFile(lockFile);
  if (file->open( IO_ReadOnly ))
  {
    QTextStream t( file );
    __pid_t s;
    t >> s;
    file->close();               

    if (getpgid(s) != -1)
    {
      // ***** tell the user about another instance of KVoiceControl already running
 
      cerr << "Another instance of KVoiceControl is already running!" << endl;
      cerr << "Its process ID is " << s << endl;
      exit(1);
    }
    else
    {
      // ***** update PID file!

      cerr << "Removing stale lock file " << lockFile << endl;
      file->remove();
      
      if (!createLockFile(file))
      {
	cerr << "Failed to create file " << lockFile << "!" << endl;
	exit(1);
      }
    }
  }
  else 
  {
    // ***** create PID file

    if (!createLockFile(file))
    {
      cerr << "Failed to create file " << lockFile << "!" << endl;
      exit(1);
    }
  }
  delete file;

  globalKapp = new KApplication( argc, argv, "kvoicecontrol");
  globalKIL  = globalKapp->getIconLoader();
  dock_widget = new DockWidget("dock_kvoicecontrol");
  dock_widget->dock();

  if (argc == 0)
    kvoicecontrol = new KVoiceControl(0);
  else
    kvoicecontrol = new KVoiceControl(argv[1]);
  return globalKapp->exec();
}


/* *******************************************************************************
**
** Constructor
**
******************************************************************************* */

KVoiceControl::KVoiceControl(char *speakerfile)
{
  speakermodel = new SpeakerModel(globalKapp->getConfig(), this, "spkm");

  globalKapp->setMainWidget(speakermodel);
  create_menu();
    
  connect(speakermodel, SIGNAL(new_title(QString)), this, SLOT(set_title(QString)));
  connect(speakermodel, SIGNAL(detect_mode_changed(bool)), this, SLOT(set_detect(bool)));
  //connect(speakermodel, SIGNAL(detect_mode_changed(bool)), dock_widget, SLOT(set_detect(bool)));

  speakermodel->reset();
  uint minY = main_menu->height();
  speakermodel->setGeometry(5, minY+5, 300, 200);
  
  resize(310,210+2*minY);
  //show();
  setMinimumSize( 310, 210+2*minY );
  setMaximumSize( 310, 210+2*minY );

  if (speakerfile != NULL)
  {
    speakermodel->load(speakerfile, true);
    speakermodel->detect_mode_on();
  }
  else
    show();
}


/* *******************************************************************************
**
** Destructor
**
******************************************************************************* */

KVoiceControl::~KVoiceControl()
{
  delete speakermodel;

  delete file_menu;
  delete opt_menu;
  delete help_menu;
  delete main_menu;
}


/* *******************************************************************************
**
** about
**
******************************************************************************* */

void KVoiceControl::about()
{
  speakermodel->detect_mode_off();

  QString version = "kvoicecontrol ";
  version += VERSION;
  version += "\n\n(c) 1998 Daniel Kiecza <daniel@kiecza.de>";
  QMessageBox::about(this, "About...", version);
}


/* *******************************************************************************
**
** create_menu
**
******************************************************************************* */

void KVoiceControl::create_menu()
{
  file_menu = new QPopupMenu;
  file_menu->insertItem( _("&New"), speakermodel, SLOT(reset()));
  file_menu->insertSeparator();
  file_menu->insertItem( _("&Load"), speakermodel, SLOT(load()));
  file_menu->insertItem( _("&Save"), speakermodel, SLOT(save()));
  file_menu->insertItem( _("Save &as..."), speakermodel, SLOT(save_as()));
  file_menu->insertSeparator();
  file_menu->insertItem( _("&Import ..."), speakermodel, SLOT(import()));
  file_menu->insertSeparator();
  file_menu->insertItem( _("&Quit"), this, SLOT(quit()), CTRL+Key_Q);
  
  rec_menu = new QPopupMenu;
  detect_ID = rec_menu->insertItem( _("&Recognition Mode"), speakermodel, SLOT(toggle_detect_mode()), CTRL+ALT+Key_R);
  rec_menu->setCheckable( TRUE );
  
  connect(dock_widget, SIGNAL(toggle_detect_mode()), speakermodel, SLOT(toggle_detect_mode()));

  QAccel *b = new QAccel(this);
  b->connectItem(b->insertItem(CTRL+ALT+Key_R), speakermodel, SLOT(toggle_detect_mode()));

  opt_menu = new QPopupMenu;
  opt_menu->insertItem( _("&Options ..."), speakermodel, SLOT(show_options()));
  opt_menu->insertSeparator();
  opt_menu->insertItem( _("&Train From File ..."), speakermodel, SLOT(train_references()));
  opt_menu->insertSeparator();
  opt_menu->insertItem( _("&Calibrate Micro ..."), speakermodel, SLOT(calibrate_micro()));

  help_menu = new QPopupMenu;
  help_menu->insertItem( _("&Help"), this, SLOT(help()),Key_F1);
  help_menu->insertSeparator();
  help_menu->insertItem( _("&About..."), this, SLOT(about()));
  
  main_menu = new KMenuBar(this, "main menu");
  main_menu->insertItem( _("&File"), file_menu);
  main_menu->insertItem( _("&Action"), rec_menu);
  main_menu->insertItem( _("&Options"), opt_menu);
  main_menu->insertSeparator();
  main_menu->insertItem( _("&Help"), help_menu);
  
  setMenu(main_menu);
}

  
/* *******************************************************************************
**
** help
**
******************************************************************************* */

void KVoiceControl::help()
{
  globalKapp->invokeHTMLHelp("", "");
}


/* *******************************************************************************
**
** quit
**
******************************************************************************* */

void KVoiceControl::quit()
{
  speakermodel->detect_mode_off();

  if (speakermodel->has_changed())
  {
    if (speakermodel->ask_save_changes())
    {
      speakermodel->save_config(globalKapp->getConfig());
      QFile::remove(lockFile.data());
      exit(0);
    }
  }
  else 
  {
    speakermodel->save_config(globalKapp->getConfig());
    QFile::remove(lockFile.data());
    exit(0);
  }
}


/* *******************************************************************************
**
** set_detect
**
******************************************************************************* */

void KVoiceControl::set_detect(bool do_detect)
{
  rec_menu->setItemChecked( detect_ID, do_detect );
}


/* *******************************************************************************
**
** set_title
**
******************************************************************************* */

void KVoiceControl::set_title(QString title)
{
  setCaption(title);
}



void KVoiceControl::closeEvent(QCloseEvent *)
{
  dock_widget->toggle_window_state();
  //e->ignore();
}


