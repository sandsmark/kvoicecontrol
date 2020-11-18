/*********************************************************************************
 *
 * $Id: buffer.cpp,v 1.10 1999/01/02 08:10:50 kiecza Exp $
 *
 *********************************************************************************/

#include "buffer.moc"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>

#include <qobject.h>
#include <qmsgbox.h>
#include <qdialog.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qstring.h>

#include <iostream>

#include <klocale.h>

//# define cpu_to_le32(x) (x)
//# define cpu_to_le16(x) (x)
//# define le32_to_cpu(x) (x)
//# define le16_to_cpu(x) (x)

/* #define _(TEXT) klocale->translate(TEXT) */
#define _(TEXT) TEXT

/* ---------------------------------------------------------------------- */

void xperror(char *msg)
{
  char text[256];

  sprintf(text,"%s: %s", msg, strerror(errno));
  QMessageBox::about(NULL, _("Error"), text);
}

/* ---------------------------------------------------------------------- */

#define BLOCK_SIZE   2048


SoundBuffer::SoundBuffer()
{
  card    = new Soundcard(NULL);
  
  rec_level  = 0;
  stop_level = 3;
  level_distance = 5;
  wait      = 0;
  
  recording = false;
  playing   = false;
  
  get_another_buffer = false;
  postfetch_N        = 1;
  postfetch_count    = 0;
  
  size     = BLOCK_SIZE*1500;
  buffer   = new short[size];
  position      = 0;
  position_play = 0;

  prefetch_N     = 2;
  prefetch_pos   = 0;
  prefetch       = new short[BLOCK_SIZE*prefetch_N];

  accept_low_N = 4;
  accept_low_count = 0;

  replay = false;

  do_calibrate = false;
  calibrate_what = 0;

  connect(card, SIGNAL(senddata(void*)),    this, SLOT(new_data(void*)));
  connect(card, SIGNAL(receivedata(void*)), this, SLOT(post_data(void*)));
}


SoundBuffer::~SoundBuffer()
{
  delete prefetch;
  delete buffer;

  delete card;
}


void* SoundBuffer::read_audio(int len)
{
  void* ptr;
  if (position_play+(int)(len/sizeof(short)) > position)
    return NULL;
  ptr = buffer + position_play;
  position_play += len/sizeof(short);
  return ptr;
}


int SoundBuffer::write_audio(int len, void *data)
{
  if (position+(int)(len/sizeof(short)) < size)
  {
    memcpy(buffer+position, data, len);
    position += len/sizeof(short);
    return 0;
  }
  else
    return -1;
}


int SoundBuffer::prefetch_audio(int len, void *data)
{
  memcpy((prefetch+prefetch_pos*(len)), data, len);
  prefetch_pos = (prefetch_pos+1)%prefetch_N;
  
  return 0;
}


void SoundBuffer::new_data(void *data)
{
  int   i,max;
  short *s;

  if (do_calibrate)
  {
    switch (calibrate_what)
    {
    case 0: // ***** nothing
      break;
    case 1: // ***** stop level
      {
	int cal_val;
	for (i = ((BLOCK_SIZE*sizeof(short))>>1)-1, s=(short*)data, cal_val = 0; i; i--)
	  if (s[i] > cal_val)
	    cal_val = s[i];
	cal_val = cal_val * 100 / 32768;
	
	QString s;
	s.sprintf("%d", cal_val);
	stop_level_msg->setText( s );
      }
      break;
    case 2: // ***** start level
      calibrate_count--;
      if (calibrate_count <= 0)
      {
	// finish calibrating start level

	if (cal_start_level_count > 0)
	  cal_start_level = (2*(cal_start_level_sum/cal_start_level_count)+1*cal_start_level_min)/3;
	else
	  cal_start_level = 0;

	//fprintf(stderr, "start_level: %d\n", cal_start_level);
		
	detect_speech(false);
	calibrate_micro();
      }
      else
      {
	int cal_max;
	
	for (i = ((BLOCK_SIZE*sizeof(short))>>1)-1, s=(short*)data, cal_max = 0; i; i--)
	  if (s[i] > cal_max)
	    cal_max = s[i];
	
	cal_max = cal_max * 100 / 32768;

	//fprintf(stderr, "start_level: Max %d\n", cal_max);
	
	if (cal_max > stop_level)
	{
	  cal_start_level_sum += cal_max;
	  cal_start_level_count++;
	  
	  if (cal_max < cal_start_level_min)
	    cal_start_level_min = cal_max;
	}
      }
    }
  }
  else
  {
    if (wait && !playing) 
    {
      // ***** prefetch buffers
      
      prefetch_audio(BLOCK_SIZE*sizeof(short),data);
      
      for (i = ((BLOCK_SIZE*sizeof(short))>>1)-1, s=(short*)data, max = 0; i; i--)
	if (s[i] > max)
	  max = s[i];
      max = max * 100 / 32768;
      
      //fprintf(stderr, "MAX: %d\n", max);
      
      if (max >= rec_level) 
      {
	wait = 0;
	//fprintf(stderr, "recording...");
	recording = true;
      } 
      else
	return;
    }
    else
      if (recording) 
      {
	//cerr  << "activ" << endl;
	emit recording_active();
	
	if (get_another_buffer)
	{
	  if (postfetch_count > 0)
	  {
	    postfetch_count--;
	    write_audio(BLOCK_SIZE*sizeof(short),data);
	  }
	  else
	  {
	    get_another_buffer = false;
	    //fprintf(stderr, "done\n");
	    stop();
	  }
	}
	else
	{
	  for (i = ((BLOCK_SIZE*sizeof(short))>>1)-1, s=(short*)data, max = 0; i; i--)
	    if (s[i] > max)
	      max = s[i];
	  max = max * 100 / 32768;
	  
	  //fprintf(stderr, "MAX: %d\n", max);
	  
	  if (accept_low_count >= accept_low_N)
	  {
	    get_another_buffer = true;
	    accept_low_count=0;
	  }
	  else
	  {
	    if (max <= stop_level)
	      accept_low_count++;
	    else
	      accept_low_count = 0;
	    
	    // ***** finish before buffer space exceeds
	    
	    if (size - position < (int)(BLOCK_SIZE*sizeof(short)*(1+postfetch_N+prefetch_N)))
	    {
	      get_another_buffer = true;
	      printf("underrun near! -> stop\n");
	    }
	    
	    write_audio(BLOCK_SIZE*sizeof(short),data);
	  }
	}
      }
  }
}


 
void SoundBuffer::post_data(void *data)
{
  void *ptr;
  
  if (NULL == (ptr = read_audio(card->get_blocksize()))) 
  {
    memset(data,0,card->get_blocksize());
    stop();
    return;
  }
  memcpy(data,ptr,card->get_blocksize());
}


void SoundBuffer::set_stop_level(int l)
{
  stop_level = l;
}


bool SoundBuffer::record()
{
  if (-1 == card->start_record()) 
  {
    xperror(_("can't open soundcard"));
    return false;
  }

  position           = 0;
  postfetch_count    = postfetch_N;
  get_another_buffer = false;
  prefetch_pos       = 0;
  wait               = 1;
   
  return true;
}

 
bool SoundBuffer::play()
{
  position_play=0;
  if (-1 == card->start_playback()) 
  {
    xperror(_("can't open soundcard"));
    return false;
  }
  playing=true;
  return true;
}


void SoundBuffer::stop()
{
  if (recording) 
  {
    recording = false;    
    card->stop();
    wait = 0;
    
    // ***** put prefetch und buffer together

    short *tmpbuffer = new short[size];
    memcpy((tmpbuffer+prefetch_N*BLOCK_SIZE), buffer, position*sizeof(short));
    memset(tmpbuffer, 0, prefetch_N*BLOCK_SIZE*sizeof(short));
    for (int i=prefetch_pos, j=0; i<prefetch_pos+prefetch_N; i++, j++)
    {
      void *to   = (tmpbuffer+j*BLOCK_SIZE);
      void *from = (prefetch+(i%prefetch_N)*BLOCK_SIZE*sizeof(short));
      memcpy(to, from, BLOCK_SIZE*sizeof(short));
    }

    delete buffer;
    buffer = tmpbuffer;
    position += prefetch_N*BLOCK_SIZE;


    if (replay)
      play();
    else
      emit end_detected();
  }
  else if (playing) 
  {
    playing = false;
    emit end_detected();
  }
}


bool SoundBuffer::detect_speech(bool detect)
{
  if (detect)
  {
    if (!recording)
    {
      return(record());
    }
    else
      return true;
  }
  else
  {
    card->stop();
    playing   = false;
    recording = false;
    wait = 0;
    return true;
  }
}


bool SoundBuffer::is_detect_mode()
{
  if (wait==1||recording)
    return true;
  else
    return false;
}


void SoundBuffer::do_replay(bool _r)
{
  replay = _r;
}


short *SoundBuffer::get_data()
{
  return buffer;
}


int SoundBuffer::get_size()
{
  return position;
}


void SoundBuffer::calibrate_micro()
{
  do_calibrate = true;
  
  switch (calibrate_what)
  {
  case 0: // ***** prepare for stop level calibration
    QMessageBox::about(NULL, _("Calibration"), "Calibrating silence now.\nAdjust MICROPHONE IN level (use kmix)\nso that the silence level displayed in the next dialog\nis stable at zero!\nPush button to continue!");  
    calibrate_what = 1;
   
    stop_level_dlgbox = new QDialog(0, _("Calibration"), FALSE);

    QLabel *stop_level_msg_lbl;
    stop_level_msg_lbl = new QLabel(stop_level_dlgbox);
    //stop_level_msg_lbl->setFrameStyle( QFrame::Panel);
    stop_level_msg_lbl->setAlignment( AlignVCenter | AlignRight );
    stop_level_msg_lbl->setText( "Level:" );
    stop_level_msg_lbl->setGeometry( 10,10, 50,20 );

    stop_level_msg = new QLabel(stop_level_dlgbox);
    stop_level_msg->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    stop_level_msg->setAlignment( AlignVCenter | AlignRight );
    stop_level_msg->setText( "" );
    stop_level_msg->setGeometry( 70,10, 22,20 );

    QPushButton *ok;
    ok = new QPushButton( "Ok", stop_level_dlgbox );
    ok->setGeometry( 20,40, 80,20 );
    connect( ok, SIGNAL(clicked()), SLOT(cal_stop_level_done()) );

    stop_level_dlgbox->show();

    detect_speech(true);
    break;
  case 1:  // ***** prepare for start level calibration
    QMessageBox::about(NULL, _("Calibration"), "Calibrating speech signal now.\nPush button and talk to your micro\nuntil the next dialog appears!");
    cal_start_level_sum = 0;
    cal_start_level_min = 1000;
    cal_start_level_count = 0;
    calibrate_count = 50;
    calibrate_what = 2;
    detect_speech(true);
    break;
  case 2:  // ***** done
    do_calibrate = false;
    calibrate_what = 0;
    if (cal_start_level >= stop_level+level_distance)
    {
      rec_level  = cal_start_level;
   
      QMessageBox::about(NULL, _("Calibration"), "Calibration done!");
    }
    else
    {
      QMessageBox::about(NULL, _("Calibration"), "Calibration failed! Push OK to restart!");
      do_calibrate = true;
      calibrate_micro();
    }
    break;
  }

}


void SoundBuffer::cal_stop_level_done()
{
  // finish calibrating stop level
	
  detect_speech(false);
  delete stop_level_dlgbox;
  calibrate_micro();
}
