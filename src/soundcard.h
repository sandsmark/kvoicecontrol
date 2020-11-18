/*********************************************************************************
 *
 * $Id: soundcard.h,v 1.3 1999/04/21 22:02:56 kiecza Exp $
 *
 *********************************************************************************/

#ifndef OSS_H
#define OSS_H

#include <qobject.h>
#include <qsocknot.h>

/* ---------------------------------------------------------------------- */
/*  taken from KRecord ...                                                */
/* ---------------------------------------------------------------------- */

#define STATUS_CLOSED    0
#define STATUS_RECORD    1
#define STATUS_PLAYBACK  2

#define FMT_UNDEFINED  0
#define FMT_8BIT       1          /* unsigned */
#define FMT_16BIT      2          /* signed - native byte order */
#define FMT_MULAW      4          /* NOT SUPPORTED (yet) */
#define FMT_ALAW       8          /* NOT SUPPORTED (yet) */

#define FMT_MAX        2

struct SOUNDPARAMS {
    int format;
    int channels;
    int rate;
    int blocksize;
    int latency;
};


class Soundcard : public QObject
{
    Q_OBJECT

private:

    /* sound card capabilities */
    char devname[32];
    int  init_done;
    int  afmt_hw;
    int  afmt_sw;
    int  channels_hw;

    int  trigger;
    char driver_name[64];

    /* current settings */
    int  afmt;
    int  channels;
    int  rate;
    int  blocksize;
    int  latency;

    /* file handle, reference count */
    int  fd, stat;
    char buffer[65536];
    QSocketNotifier *telmi;
    
    /* internal functions */
    void get_capabilities();
    int  open_dev(int record);
    void close_dev();

public:

    Soundcard(char *dev);
    ~Soundcard();
    char *driver();
    void setparams(struct SOUNDPARAMS *params);
    int  get_blocksize() { return blocksize;};
    int  start_record();
    int  start_playback();
    int  stop();

    int  has_channels();      /* # of channels (1=mono,2=stereo) */
    int  has_format(int f);   /* check format availibity         */ 

public slots:

    void sounddata(int);
    
signals:

    void senddata(void *data);
    /* !!! only one should be connected to receivedata !!! */
    void receivedata(void *data);
    void newparams(struct SOUNDPARAMS *params);
};

#endif

