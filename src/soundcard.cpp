/*********************************************************************************
 *
 * $Id: soundcard.cpp,v 1.4 1999/04/21 22:03:02 kiecza Exp $
 *
 *********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <qtimer.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <pulse/pulseaudio.h>

#include "soundcard.moc"

#ifndef OSS_GETVERSION
#define OSS_GETVERSION       _IOR('M',118,int)
#endif



extern pa_context *pulse_context;
/* ---------------------------------------------------------------------- */

Soundcard::Soundcard(char *dev)
{
    puts("Creating soundcard");

    if (dev) {
        strcpy(devname, dev);
    } else {
        strcpy(devname, "/dev/dsp");
    }

    driver_name[0] = '\0';

    get_capabilities();
    channels = 1;
    rate = 16000;
    afmt = AFMT_S16_LE;
    fd = -1;
    stat = STATUS_CLOSED;

    stream = NULL;
    blocksize = 2048;
}


Soundcard::~Soundcard()
{
    /* nothing */
}


int Soundcard::start_record()
{
    puts("Start record!");

    switch (stat) {
    case STATUS_CLOSED:
        puts("is closed, reopening");

        if (!init_done) {
            get_capabilities();
        }

        if (!init_done) {
            return -1;
        }

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
    puts("Start playback!");

    switch (stat) {
    case STATUS_CLOSED:
        if (!init_done) {
            get_capabilities();
        }

        if (!init_done) {
            return -1;
        }

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
    if (stat != STATUS_CLOSED) {
        puts("Closing");
        close_dev();
    }

    return 0;
}

/* ---------------------------------------------------------------------- */

void Soundcard::get_capabilities()
{
    init_done = true;
    return;

    int i, dsp;
    int try_afmt;
    int try_channels;

    afmt = 0;

    if (-1 != (dsp = open(devname, O_RDONLY))) {
        ioctl(dsp, SNDCTL_DSP_SETFMT,  &afmt);     /* current */
        ioctl(dsp, SNDCTL_DSP_GETFMTS, &afmt_hw);  /* hardware cap */
        afmt_sw = 0;

        for (i = 0; i < 16 /* XXX */; i++) {
            try_afmt = (1 << i);

            if (-1 == ioctl(dsp, SNDCTL_DSP_SETFMT, &try_afmt)) {
                continue;
            }

            if (try_afmt != (1 << i)) {
                continue;
            }

            afmt_sw |= try_afmt;
        }

        try_channels = 2;

        if (-1 != ioctl(dsp, SNDCTL_DSP_CHANNELS, &try_channels) && 2 == try_channels) {
            channels_hw = 2;
        } else {
            channels_hw = 1;
        }

        /* version check */
        if (-1 == ioctl(dsp, OSS_GETVERSION, &i)) {
            strcpy(driver_name, "OSS (version unknown)");
        } else {
            sprintf(driver_name, "OSS %d.%d.%d%c", (i >> 16) & 0xff, (i >> 8) & 0xff, (i >> 4) & 0xf, (i & 0xf) + 'a');
        }

        close(dsp);
        init_done = 1;
    } else {
        init_done = 0;
    }
}


int Soundcard::has_channels()
{
    if (!init_done) {
        return -1;
    }

    return channels_hw;
}


int Soundcard::has_format(int f)
{
    if (!init_done) {
        return -1;
    }

    switch (f) {
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

void Soundcard::pulse_read_cb(pa_stream *stream, size_t length, void *userdata)
{
    Soundcard *that = reinterpret_cast<Soundcard *>(userdata);
    const void *data;

    if (that->stat != STATUS_RECORD) {
        printf("Not recording, is: %d\n", that->stat);
        pa_stream_drop(stream);
        return;
    }

    if (int(pa_stream_readable_size(stream)) < that->blocksize) {
        printf("not enough bytes %lu bytes\n", pa_stream_readable_size(stream));
        return;
    }

    if (pa_stream_peek(stream, &data, &length) < 0) {
        fprintf(stderr, "pa_stream_peek() failed: %s\n", pa_strerror(pa_context_errno(pulse_context)));
        exit(1);
        return;
    }

    // todo: loop and read
    assert(length < 65536);
    assert(length > 0);
    memcpy(that->buffer, data, length);
    pa_stream_drop(stream);
    that->blocksize = length;

    QTimer::singleShot(0, that, SIGNAL(senddata()));
}

void Soundcard::pulse_write_cb(pa_stream *stream, size_t length, void *userdata)
{
    puts("Writing");
    Soundcard *that = reinterpret_cast<Soundcard *>(userdata);

//    that->blocksize = length;
    if (that->stat != STATUS_PLAYBACK) {
        puts("Not playing back");
//        pa_stream_disconnect(stream);
        return;
    }

    ssize_t paLength = length;
    while (paLength > 0 && that->stat == STATUS_PLAYBACK) {
        emit that->receivedata((void *)that->buffer);

        char *buffer = that->buffer;

        int bytesLeft = length;//that->blocksize;
        printf("length: %lu\n", length);

        while (paLength > 0 && bytesLeft > 0 && that->stat == STATUS_PLAYBACK) {
            size_t bytesToWrite = that->blocksize;

            if (pa_stream_begin_write(stream, (void **)&buffer, &bytesToWrite) < 0) {
                puts("failed to begin write");
                return;
            }

            if (bytesToWrite == 0) {
                puts("no bytes to write?");
                break;
            }

            if (ssize_t(bytesToWrite) > bytesLeft) {
                bytesToWrite = bytesLeft;
            }
            assert(int(bytesToWrite) <= bytesLeft);
            bytesLeft -= bytesToWrite;
            paLength -= bytesToWrite;
            pa_stream_write(stream, (void *)buffer, bytesToWrite, NULL, 0LL,
                            PA_SEEK_RELATIVE);
            buffer += bytesToWrite;
            printf("Wrote %lu bytes, now at %p\n", bytesToWrite, buffer);
        }
    }

    puts("Written");
}

int Soundcard::open_dev(int record)
{
    stat = record ? STATUS_RECORD : STATUS_PLAYBACK;
    puts("Opening dev");
    printf("Open dev!");

    pa_sample_spec sampleSpec;
    sampleSpec.format = PA_SAMPLE_S16LE;
    sampleSpec.channels = 1;
    sampleSpec.rate = 16000;
    blocksize = 2048;

    pa_buffer_attr attributes;
    bzero(&attributes, sizeof(attributes));
    attributes.maxlength = sizeof(buffer);
    attributes.tlength = -1;
    attributes.prebuf = -1;
    attributes.minreq = -1;
    attributes.fragsize = -1;

//    attributes.tlength = blocksize;
//    attributes.prebuf = blocksize;
//    attributes.minreq = blocksize;
//    attributes.fragsize = blocksize;
//    printf("%d\n", blocksize);
    stream = pa_stream_new(pulse_context, record ? "Record" : "Play", &sampleSpec, NULL);

    if (!stream) {
        puts(" !!!!!!!! FAiled to connect stream");
        return 1;
    }

//    blocksize = attributes.fragsize;

    const char *dev = NULL;
    pa_stream_flags_t flags = PA_STREAM_NOFLAGS;

    if (record) {
        puts("Opening record");
        pa_stream_set_read_callback(stream, pulse_read_cb, this);

        if (pa_stream_connect_record(stream, dev, &attributes, flags) < 0) {
            puts("Failed to connect stream");
            pa_stream_unref(stream);
            return 1;
        }
    } else {
        puts("Opening write");
        pa_stream_set_write_callback(stream, pulse_write_cb, this);

        pa_stream *sync_stream = NULL;
        pa_cvolume *volume = NULL;

        if (pa_stream_connect_playback(stream, dev, &attributes, flags, volume, sync_stream) < 0) {
            puts("Failed to connect stream");
            pa_stream_unref(stream);
            return 1;
        }

    }

//    printf("Reading %d bytes, now at %p\n", blocksize, buffer);
//    printf("Reading %u bytes, now at %p\n", attributes.tlength, buffer);
//    printf("Reading %u bytes, now at %p\n", attributes.prebuf, buffer);
//    printf("Reading %u bytes, now at %p\n", attributes.minreq, buffer);
    printf("fragsize %u bytes, now at %p\n", attributes.fragsize, buffer);


    return 0;

    struct SOUNDPARAMS p;
    //int frag;

    if (-1 == (fd = open(devname, record ? O_RDONLY : O_WRONLY))) {
        goto err;
    }

    fcntl(fd, F_SETFD, FD_CLOEXEC);

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

    if (-1 == ioctl(fd, SNDCTL_DSP_SETFMT,      &afmt)) {
        goto err;
    }

    if (-1 == ioctl(fd, SNDCTL_DSP_CHANNELS,    &channels)) {
        goto err;
    }

    if (-1 == ioctl(fd, SNDCTL_DSP_SPEED,       &rate)) {
        goto err;
    }

    if (-1 == ioctl(fd, SNDCTL_DSP_GETBLKSIZE,  &blocksize)) {
        goto err;
    }

    latency = blocksize * 1000 / channels / rate;

    if (afmt == AFMT_U16_BE || afmt == AFMT_S16_BE || afmt == AFMT_U16_LE || afmt == AFMT_S16_LE) {
        latency = latency / 2;
    }

    telmi = new QSocketNotifier(fd, record ? QSocketNotifier::Read : QSocketNotifier::Write);
    QObject::connect(telmi, SIGNAL(activated(int)), this, SLOT(sounddata(int)));

    stat = record ? STATUS_RECORD : STATUS_PLAYBACK;

    //fprintf(stderr,"%s (format=%d, %s, rate=%d, blocksize=%d, latency=%d ms)\n",
    //	  record ? "recording" : "playback", afmt, (channels == 2) ? "stereo" : "mono", rate, blocksize, latency);

    p.channels  = channels;
    p.rate      = rate;
    p.blocksize = blocksize;
    p.latency   = latency;

    switch (afmt) {
    case AFMT_U8:
        p.format = FMT_8BIT;
        break;

    case AFMT_S16_LE:
        p.format = FMT_16BIT;
        break;

    default:
        fprintf(stderr, "oops(open): unsupported sound format\n");
        exit(1);
    }

    emit newparams(&p);

    if (record) {
        trigger = ~PCM_ENABLE_INPUT;
        ioctl(fd, SNDCTL_DSP_SETTRIGGER, &trigger);
        trigger = PCM_ENABLE_INPUT;
        ioctl(fd, SNDCTL_DSP_SETTRIGGER, &trigger);
    }

    return 0;

err:

    if (-1 != fd) {
        close(fd);
    }

    stat = STATUS_CLOSED;
    fd = -1;
    return -1;
}


void Soundcard::close_dev()
{
    if (!stream) {
        if (stat != STATUS_CLOSED) {
            puts("!!!! wasn't set as closed");
            stat = STATUS_CLOSED;
        }

        puts("already closed!");
        return;
    }

    puts("========== Closing dev ===========");
    pa_stream_disconnect(stream);
    pa_stream_unref(stream);
    stream = NULL;
    stat = STATUS_CLOSED;
    return;
    close(fd);
    fd = -1;
    stat = STATUS_CLOSED;

    delete telmi;
    return;
}


void Soundcard::setparams(struct SOUNDPARAMS *p)
{
    puts("Setting params");
    rate     = p->rate;
    channels = p->channels;

    switch (p->format) {
    case FMT_8BIT:
        afmt = AFMT_U8;
        break;

    case FMT_16BIT:
        afmt = AFMT_S16_LE;
        break;

    default:
        fprintf(stderr, "oops(set): unsupported sound format\n");
        exit(1);
    }

    switch (stat) {
    case STATUS_RECORD:
        close_dev();
        open_dev(TRUE);
        break;

    case STATUS_PLAYBACK:
        close_dev();
        open_dev(FALSE);
        break;

    case STATUS_CLOSED:
        if (!init_done) {
            get_capabilities();
        }

        if (!init_done) {
            return;
        }

        if (0 == open_dev(TRUE)) {
            close_dev();
        }

        break;
    }
}


void Soundcard::sounddata(int)
{
    return;

    switch (stat) {
    case STATUS_RECORD:
        read(fd, buffer, blocksize);
#if 0

        if (select_bug) {
            trigger = ~PCM_ENABLE_INPUT;
            ioctl(fd, SNDCTL_DSP_SETTRIGGER, &trigger);
            trigger = PCM_ENABLE_INPUT;
            ioctl(fd, SNDCTL_DSP_SETTRIGGER, &trigger);
        }

#endif
//    emit senddata((void*)buffer);
        break;

    case STATUS_PLAYBACK:
        emit receivedata((void *)buffer);
        write(fd, buffer, blocksize);
        //emit senddata((void*)buffer); /* fft :-) */
        break;
    }
}

