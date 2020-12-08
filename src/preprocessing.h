/*********************************************************************************
 *
 * $Id: preprocessing.h,v 1.2 1999/01/02 08:14:51 kiecza Exp $
 *
 *********************************************************************************/

#ifndef KPREPROCESSING_H
#define KPREPROCESSING_H

#include <qobject.h>

class Utterance;


class Preprocessing : public QObject
{
    Q_OBJECT

public:

    Preprocessing();
    ~Preprocessing();
    Utterance *preprocess_utterance(short *wave, int size);

private:

    int    rate;
    int   *filter_banks;
    int    fft_size;     // ***** Size of short-time window
    float *hamming_window;
    int    hamming_size; // ***** Hamming window width = 16ms ! (256 Frames)
    int    offset;       // ***** Offset = 10ms (160 Frames)
};

#endif

