/*********************************************************************************
 *
 * $Id: soundcard.cpp,v 1.4 1999/04/21 22:03:02 kiecza Exp $
 *
 *********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include "soundcard.moc"

#ifndef OSS_GETVERSION
#define OSS_GETVERSION       _IOR('M',118,int)
#endif



/* ---------------------------------------------------------------------- */

Soundcard::Soundcard(char *dev)
{
  if (dev)
    strcpy(devname,dev);
  else
    strcpy(devname,"/dev/dsp");
  
  driver_name[0] = '\0';

  get_capabilities();
  channels = 1;
  rate = 16000;
  afmt = AFMT_S16_LE;
  fd = -1;
  stat = STATUS_CLOSED;
}


Soundcard::~Soundcard()
{
  /* nothing */
}


int Soundcard::start_record()
{
  switch (stat) 
  {
  case STATUS_CLOSED:
    if (!init_done)
      get_capabilities();
    if (!init_done)
      return -1;
    return open_dev(TRUE);
  case STATUS_RECORD:
    return 0;
  case STATUS_PLAYBACK:
    close_dev();
    return open_dev(TRUE);
  }
  return -1;
}


int Soundcard::start_playback()
{
  switch (stat)
  {
  case STATUS_CLOSED:
    if (!init_done)
      get_capabilities();
    if (!init_done)
      return -1;
    return open_dev(FALSE);
  case STATUS_RECORD:
    close_dev();
    return open_dev(FALSE);
  case STATUS_PLAYBACK:
    return 0;
  }
  return -1;
}


int Soundcard::stop()
{
  if (stat != STATUS_CLOSED)
    close_dev();
  return 0;
}

/* ---------------------------------------------------------------------- */

void Soundcard::get_capabilities()
{
  int i,dsp;
  int try_afmt;
  int try_channels;
  
  afmt = 0;
  if (-1 != (dsp = open(devname, O_RDONLY)))
  {
    ioctl(dsp, SNDCTL_DSP_SETFMT,  &afmt);     /* current */
    ioctl(dsp, SNDCTL_DSP_GETFMTS, &afmt_hw);  /* hardware cap */
    afmt_sw = 0;
    
    for (i = 0; i < 16 /* XXX */; i++)
    {
      try_afmt = (1<<i);
      if (-1 == ioctl(dsp, SNDCTL_DSP_SETFMT, &try_afmt))
	continue;
      if (try_afmt != (1<<i))
	continue;
      afmt_sw |= try_afmt;
    }
    
    try_channels = 2;
    if (-1 != ioctl(dsp, SNDCTL_DSP_CHANNELS, &try_channels) && 2 == try_channels)
      channels_hw = 2;
    else
      channels_hw = 1;
    
    /* version check */
    if (-1 == ioctl(dsp,OSS_GETVERSION,&i))
    {
      strcpy(driver_name,"OSS (version unknown)");
    }
    else
    {
      sprintf(driver_name,"OSS %d.%d.%d%c", (i>>16) & 0xff,(i>>8) & 0xff,(i>>4) & 0xf,(i&0xf)+'a');
    }
    
    close(dsp);
    init_done = 1;
  }
  else
  {
    init_done = 0;
  }
}


int Soundcard::has_channels()
{
  if (!init_done)
    return -1;
  return channels_hw;
}


int Soundcard::has_format(int f)
{
  if (!init_done)
    return -1;
  switch (f)
  {
  case FMT_8BIT:
    return (afmt_hw & AFMT_U8) ? 1 : 0;
    break;
  case FMT_16BIT:
    return (afmt_hw & AFMT_S16_LE) ? 1 : 0;
    break;
  case FMT_MULAW:
  case FMT_ALAW:
  default:
    return 0;
  }
}


char *Soundcard::driver()
{
  return driver_name;
}


int Soundcard::open_dev(int record)
{
  struct SOUNDPARAMS p;
  //int frag;
  
  if (-1 == (fd = open(devname,record ? O_RDONLY : O_WRONLY)))
    goto err;
  fcntl(fd,F_SETFD,FD_CLOEXEC);
  
  /* try to get ~50 ms latency
  blocksize = 50*channels*rate/1000;
  if (afmt == AFMT_U16_BE || afmt == AFMT_S16_BE || afmt == AFMT_U16_LE || afmt == AFMT_S16_LE)
    blocksize *= 2;
  for (frag = 0; blocksize != 1; frag++)
    blocksize >>= 1;

  fprintf(stderr,"asking for %d byte blocksize\n",1 << frag);
  
  frag |= 0x7fff0000;
  if (-1 == ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &frag))
    perror("ioctl SNDCTL_DSP_SETFRAGMENT");
  */
  
  if (-1 == ioctl(fd, SNDCTL_DSP_SETFMT,      &afmt))
    goto err;
  if (-1 == ioctl(fd, SNDCTL_DSP_CHANNELS,    &channels))
    goto err;
  if (-1 == ioctl(fd, SNDCTL_DSP_SPEED,       &rate))
    goto err;
  if (-1 == ioctl(fd, SNDCTL_DSP_GETBLKSIZE,  &blocksize))
    goto err;
  
  latency = blocksize*1000/channels/rate;
  if (afmt == AFMT_U16_BE || afmt == AFMT_S16_BE || afmt == AFMT_U16_LE || afmt == AFMT_S16_LE)
    latency = latency/2;
  
  telmi = new QSocketNotifier(fd, record ? QSocketNotifier::Read : QSocketNotifier::Write);
  QObject::connect(telmi,SIGNAL(activated(int)), this, SLOT(sounddata(int)));
  
  stat = record ? STATUS_RECORD : STATUS_PLAYBACK;
  
  //fprintf(stderr,"%s (format=%d, %s, rate=%d, blocksize=%d, latency=%d ms)\n",
  //	  record ? "recording" : "playback", afmt, (channels == 2) ? "stereo" : "mono", rate, blocksize, latency);

  p.channels  = channels;
  p.rate      = rate;
  p.blocksize = blocksize;
  p.latency   = latency;

  switch (afmt)
  {
  case AFMT_U8:
    p.format = FMT_8BIT;
    break;
  case AFMT_S16_LE:
    p.format = FMT_16BIT;
    break;
  default:
    fprintf(stderr,"oops(open): unsupported sound format\n");
    exit(1);
  }

  emit newparams(&p);
  
  if (record)
  {
    trigger = ~PCM_ENABLE_INPUT;
    ioctl(fd,SNDCTL_DSP_SETTRIGGER,&trigger);
    trigger = PCM_ENABLE_INPUT;
    ioctl(fd,SNDCTL_DSP_SETTRIGGER,&trigger);
  }
  return 0;
  
 err:
  if (-1 != fd)
    close(fd);
  stat = STATUS_CLOSED;
  fd = -1;
  return -1;
}


void Soundcard::close_dev()
{
  close(fd);
  fd = -1;
  stat = STATUS_CLOSED;
  
  delete telmi;
  return;
}


void Soundcard::setparams(struct SOUNDPARAMS *p)
{
  rate     = p->rate;
  channels = p->channels;
  switch (p->format) 
  {
  case FMT_8BIT:   afmt = AFMT_U8;      break;
  case FMT_16BIT:  afmt = AFMT_S16_LE;  break;
  default: fprintf(stderr,"oops(set): unsupported sound format\n"); exit(1);
  }
  
  switch (stat) 
  {
  case STATUS_RECORD:    
    close_dev();
    open_dev(TRUE);
    break;
  case STATUS_PLAYBACK:
    close_dev();
    open_dev(FALSE);
    break;
  case STATUS_CLOSED:
    if (!init_done)
      get_capabilities();
    if (!init_done)
      return;
    if (0 == open_dev(TRUE))
      close_dev();
    break;
  }
}


void Soundcard::sounddata(int)
{
  switch (stat)
  {
  case STATUS_RECORD:
    read(fd,buffer,blocksize);
#if 0
    if (select_bug) 
    {
      trigger = ~PCM_ENABLE_INPUT;
      ioctl(fd,SNDCTL_DSP_SETTRIGGER,&trigger);
      trigger = PCM_ENABLE_INPUT;
      ioctl(fd,SNDCTL_DSP_SETTRIGGER,&trigger);
    }
#endif
    emit senddata((void*)buffer);
    break;
  case STATUS_PLAYBACK:
    emit receivedata((void*)buffer);
    write(fd,buffer,blocksize);
    //emit senddata((void*)buffer); /* fft :-) */
    break;
  }
}

