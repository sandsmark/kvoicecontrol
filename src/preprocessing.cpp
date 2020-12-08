/*********************************************************************************
 *
 * $Id: preprocessing.cpp,v 1.2 1999/01/02 08:11:01 kiecza Exp $
 *
 *********************************************************************************/

#include "preprocessing.moc"

#include <math.h>

#include "utterance.h"

/* realfftf.c */
extern int *bit_reversed;
extern "C" void initialize_FFT(int fftlen);
extern "C" void end_FFT();
extern "C" void real_FFT(float *buffer);


Preprocessing::Preprocessing()
{
    rate = 16000;
    fft_size = 256;

    initialize_FFT(fft_size);

    hamming_size = fft_size;
    float tmp = 2.0 * M_PI / (hamming_size - 1);
    hamming_window = new float[hamming_size];

    for (int i = 0; i < hamming_size; i++) {
        hamming_window[i] = 0.54 - 0.46 * cos(tmp * i);
    }

    filter_banks = new int[17];
    filter_banks[0] = 0;
    filter_banks[1] = 2;
    filter_banks[2] = 6;
    filter_banks[3] = 10;
    filter_banks[4] = 14;
    filter_banks[5] = 18;
    filter_banks[6] = 22;
    filter_banks[7] = 26;
    filter_banks[8] = 30;
    filter_banks[9] = 35;
    filter_banks[10] = 41;
    filter_banks[11] = 48;
    filter_banks[12] = 57;
    filter_banks[13] = 68;
    filter_banks[14] = 81;
    filter_banks[15] = 97;
    filter_banks[16] = 116;

    // ***** Offset = 10ms (160 Frames)
    offset = 160;
}


Preprocessing::~Preprocessing()
{
    end_FFT();

    for (int i = 0; i < hamming_size; i++) {
        delete hamming_window;
    }

    delete filter_banks;
}


Utterance *Preprocessing::preprocess_utterance(short *wave, int size)
{
    Utterance *utt = new Utterance();

    float buffer[fft_size];

    // ***** process wave
    // ***** FFT, powerspec, melscale, meansub

    int frames_N = (size - fft_size) / offset;

    // ***** init melscale buffer ...

    float **mel_scales;
    mel_scales = new float * [frames_N];

    for (int i = 0; i < frames_N; i++) {
        mel_scales[i] = new float[16];
    }

    // ***** calculate Spectrum/Melscale

    for (int frame = 0; frame < frames_N; frame++) {
        // ***** fourier transformation, power spectrum

        for (int i = 0; i < fft_size; i++) {
            buffer[i] = ((float)wave[frame * offset + i]) * hamming_window[i];
        }

        real_FFT(buffer);

        int power_spec_N = fft_size >> 1;
        float *power_spec = new float[power_spec_N];

        for (int j = 0 ; j < power_spec_N ; j++)
            power_spec[j] = buffer[bit_reversed[j]] * buffer[bit_reversed[j]] +
                            buffer[bit_reversed[j] + 1] * buffer[bit_reversed[j] + 1];

        // ***** melscale reduction

        for (int melI = 0; melI < 16; melI++) {
            int from = (int)filter_banks[melI  ];
            int to   = (int)filter_banks[melI + 1];

            if (from == 0) {
                mel_scales[frame][melI] = power_spec[0];
            } else {
                mel_scales[frame][melI] = power_spec[from] / 2.0;
            }

            mel_scales[frame][melI] += power_spec[to] / 2.0;

            for (int i = from + 1; i <= to - 1; i++) {
                mel_scales[frame][melI] += power_spec[i];
            }

            mel_scales[frame][melI] = log(mel_scales[frame][melI] + 1.0) / M_LN2;
        }
    }

    // ***** calculate mean vector

    float *mean = new float[16];

    for (int i = 0; i < 16; i++) {
        mean[i] = 0;

        for (int l = 0; l < frames_N; l++) {
            mean[i] += mel_scales[l][i];
        }

        mean[i] /= frames_N;
    }

    // ***** subtract mean vector

    for (int l = 0; l < frames_N; l++) {
        float val = 0;

        for (int i = 0; i < 16; i++) {
            mel_scales[l][i] -= mean[i];

            val += mel_scales[l][i] * mel_scales[l][i];
        }
    }

    utt->set_data(mel_scales, frames_N);

    delete[] mean;

    return utt;
}
