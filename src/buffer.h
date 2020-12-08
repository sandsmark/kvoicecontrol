/*********************************************************************************
 *
 * $Id: buffer.h,v 1.8 1999/01/02 08:15:10 kiecza Exp $
 *
 *********************************************************************************/

#ifndef BUFFER_H
#define BUFFER_H

#include <qobject.h>

#include "soundcard.h"

class QDialog;
class QLabel;

/* ---------------------------------------------------------------------- */
/*  excerpted from KRecord ...                                            */
/* ---------------------------------------------------------------------- */

class SoundBuffer : public QObject
{
    Q_OBJECT

private:

    short *buffer;

    int  position;
    int  position_play;
    int  size;

    Soundcard    *card;

    int          rec_level, stop_level, wait, level_distance;

    bool         recording, playing;

    void         *prefetch;
    int          prefetch_N;
    int          prefetch_pos;

    int accept_low_N;
    int accept_low_count;

    bool         get_another_buffer;
    int          postfetch_N;
    int          postfetch_count;

    bool replay;

    bool do_calibrate;
    int calibrate_what; // ***** -1=init, 0=nothing, 1=start_level, 2=stop_level
    int calibrate_count;
    int cal_stop_level;
    int cal_start_level_sum;
    int cal_start_level_min;
    int cal_start_level_count;
    int cal_start_level;

    QDialog *stop_level_dlgbox;
    QLabel  *stop_level_msg;

    bool record();
    void stop();

public:

    SoundBuffer();
    ~SoundBuffer();

    bool play();

    void *read_audio(int len);
    int   write_audio(int len, void *data);
    int   prefetch_audio(int len, void *data);

    bool is_detect_mode();
    bool detect_speech(bool);
    void set_stop_level(int l);

    void do_replay(bool);
    short *get_data();
    int get_size();

    int get_acc_sil_frames()
    {
        return accept_low_N;
    };
    int get_rec_level_threshold()
    {
        return rec_level;
    };

signals:

    void recording_active();
    void end_detected();

public slots:

    void new_data();
    void post_data(void *data);

    void set_accept_silence(int val)
    {
        accept_low_N = val;
    };
    void set_rec_level_threshold(int val)
    {
        rec_level = val;
    };

    void calibrate_micro();
    void cal_stop_level_done();
};

#endif
