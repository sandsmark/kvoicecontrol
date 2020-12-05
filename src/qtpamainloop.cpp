#include "qtpamainloop.h"

void TimerWrapper::onDeferTimeout() {
    deferCallback(a, reinterpret_cast<pa_defer_event *>(this), userdata);
    if (singleshot) {
        stop();

        //idk
        delete this;
    }
}

void TimerWrapper::onTimeTimeout() {
    timeCallback(a, reinterpret_cast<pa_time_event *>(this), tv, userdata);
    if (singleshot) {
        stop();

        //idk
        delete this;
    }
}

TimerWrapper::TimerWrapper() :
    QTimer(qApp),
    a(NULL),
    userdata(NULL),
    singleshot(false),
    deferCallback(NULL),
    timeCallback(NULL),
    timerDestructor(NULL),
    deferDestructor(NULL),
    tv(NULL)
{
}

TimerWrapper::~TimerWrapper() {
    if (deferDestructor) {
        deferDestructor(a, reinterpret_cast<pa_defer_event *>(this), userdata);
    }
    if (timerDestructor) {
        timerDestructor(a, reinterpret_cast<pa_time_event *>(this), userdata);
    }

}

SocketNotifierWrapper::SocketNotifierWrapper() :
    readNotifier(NULL),
    writeNotifier(NULL),
    errorNotifier(NULL),
    destructor(NULL),
    userdata(NULL),
    a(NULL)
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

void SocketNotifierWrapper::onRead(int fd) {
    cb(a, reinterpret_cast<pa_io_event*>(this), fd, PA_IO_EVENT_INPUT, userdata);
}

void SocketNotifierWrapper::onWrite(int fd) {
    cb(a, reinterpret_cast<pa_io_event*>(this), fd, PA_IO_EVENT_OUTPUT, userdata);
}

void SocketNotifierWrapper::onError(int fd) {
    cb(a, reinterpret_cast<pa_io_event*>(this), fd, PA_IO_EVENT_ERROR, userdata);
}

SocketNotifierWrapper::SocketNotifierWrapper(const SocketNotifierWrapper &o) {
    *this = o;
}

const SocketNotifierWrapper &SocketNotifierWrapper::operator=(const SocketNotifierWrapper &) {
    assert(false);
    return *this;
}

#include "qtpamainloop.moc"
