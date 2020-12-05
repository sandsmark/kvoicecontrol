#include "qtpamainloop.h"
#include "kvoicecontrol.h"
#include <pulse/pulseaudio.h>

#include <unistd.h>

pa_context *pulse_context;
extern pa_mainloop_api *pulse_api;
extern KVoiceControl *kvoicecontrol;

void TimerWrapper::onDeferTimeout()
{
    if (!deferCallback) {
        return;
    }
    if (aborted) {
        return;
    }
    deferCallback(a, reinterpret_cast<pa_defer_event *>(this), userdata);
}

void TimerWrapper::onTimeTimeout()
{
    if (!timeCallback) {
        return;
    }
    if (aborted) {
        return;
    }
    timeCallback(a, reinterpret_cast<pa_time_event *>(this), tv, userdata);
}

TimerWrapper::TimerWrapper() :
    QTimer(qApp),
    a(NULL),
    userdata(NULL),
    aborted(false),
    deferCallback(NULL),
    timeCallback(NULL),
    timerDestructor(NULL),
    deferDestructor(NULL),
    tv(NULL)
{
}

TimerWrapper::~TimerWrapper() {

}

void TimerWrapper::onKill()
{
    if (deferDestructor) {
        deferDestructor(a, reinterpret_cast<pa_defer_event *>(this), userdata);
    }
    if (timerDestructor) {
        timerDestructor(a, reinterpret_cast<pa_time_event *>(this), userdata);
    }

    delete this;
}

SocketNotifierWrapper::SocketNotifierWrapper() :
    readNotifier(NULL),
    writeNotifier(NULL),
    errorNotifier(NULL),
    destructor(NULL),
    userdata(NULL),
    a(NULL),
    cb(NULL),
    aborted(false)
{}

SocketNotifierWrapper::~SocketNotifierWrapper()
{
    delete readNotifier;
    delete writeNotifier;
    delete errorNotifier;

    if (destructor) {
        destructor(a, reinterpret_cast<pa_io_event *>(this), userdata);
    }
}

void SocketNotifierWrapper::onRead(int fd)
{
    if (aborted) {
        return;
    }
//    printf("read: %d\n", fd);
    cb(a, reinterpret_cast<pa_io_event*>(this), fd, PA_IO_EVENT_INPUT, userdata);
}

void SocketNotifierWrapper::onWrite(int fd)
{
    if (aborted) {
        return;
    }
    cb(a, reinterpret_cast<pa_io_event*>(this), fd, PA_IO_EVENT_OUTPUT, userdata);
}

void SocketNotifierWrapper::onError(int fd)
{
    if (aborted) {
        return;
    }
    cb(a, reinterpret_cast<pa_io_event*>(this), fd, PA_IO_EVENT_ERROR, userdata);
}

void SocketNotifierWrapper::onKill()
{
    if (destructor) {
        destructor(a, reinterpret_cast<pa_io_event *>(this), userdata);
    }

    delete this;
}

SocketNotifierWrapper::SocketNotifierWrapper(const SocketNotifierWrapper &o) {
    *this = o;
}

const SocketNotifierWrapper &SocketNotifierWrapper::operator=(const SocketNotifierWrapper &) {
    assert(false);
    return *this;
}

static void context_state_callback(pa_context *c, void *userdata)
{
    (void)userdata;// todo: use it?
    pa_context_state_t state = pa_context_get_state(c);
    switch (state) {
    case PA_CONTEXT_UNCONNECTED:
        std::cout << "unconnceted" << std::endl;
        break;
    case PA_CONTEXT_CONNECTING:
        std::cout << "connecting..." << std::endl;
        break;
    case PA_CONTEXT_AUTHORIZING:
        std::cout << "authorizing..." << std::endl;
        break;
    case PA_CONTEXT_SETTING_NAME:
        std::cout << "setting name..." << std::endl;
        break;

    case PA_CONTEXT_READY:
        std::cout << "context ready" << std::endl;
        break;

    case PA_CONTEXT_FAILED:
        std::cerr << "context failed" << std::endl;
        break;

    case PA_CONTEXT_TERMINATED:
        std::cerr << "context terminated!" << std::endl;
        break;
    default:
        std::cerr << "Unknown state: " << state << std::endl;
        return;
    }

}

bool connectToPulse()
{
    if (pulse_context) {
        std::cerr << "Already connected!" << std::endl;
        return false;
    }
    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "KVoiceControl");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "com.iskrembilen.kvoicecontrol");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "3.0");

    pulse_context = pa_context_new_with_proplist(pulse_api, NULL, proplist);
    assert(pulse_context);

    pa_proplist_free(proplist);

    pa_context_set_state_callback(pulse_context, context_state_callback, kvoicecontrol);
    if (pa_context_connect(pulse_context, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
        if (pa_context_errno(pulse_context) == PA_ERR_INVALID) {
            std::cerr << "invalid  pulse context" << std::endl;
        } else {
            std::cerr << "error connecting context" << std::endl;
        }
        return false;
    }
    std::cout << "Connected to pulseaudio" << std::endl;
    return true;
}


#include "qtpamainloop.moc"
