/*********************************************************************************
 *
 * $Id: realfftf.h,v 1.2 1999/01/02 08:14:46 kiecza Exp $
 *
 *********************************************************************************/

typedef float fft_type;

extern int *bit_reversed;

void initialize_FFT(int);
void end_FFT(void);
void real_FFT(fft_type *);

