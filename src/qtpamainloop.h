#pragma once

#include <qapplication.h>
#include <qtimer.h>
#include <qsocketnotifier.h>
#include <qdatetime.h>
#include <iostream>
#include <pulse/mainloop-api.h>
#include <sys/time.h>
#include <assert.h>
#include <stdio.h>

class TimerWrapper : public QTimer
{
    Q_OBJECT

public:
    pa_mainloop_api *a;
    void *userdata;

    bool aborted;
    pa_defer_event_cb_t deferCallback;
    pa_time_event_cb_t timeCallback;
    pa_time_event_destroy_cb_t timerDestructor;

    pa_defer_event *deferEvent;
    pa_defer_event_destroy_cb_t deferDestructor;
    const struct timeval *tv;
public slots:
    void onDeferTimeout();
    void onTimeTimeout();
    void onKill();
public:
    TimerWrapper();

    ~TimerWrapper();
};

class SocketNotifierWrapper : public QObject
{
    Q_OBJECT
public:
    SocketNotifierWrapper();
    ~SocketNotifierWrapper();
public slots:
    void onRead(int fd);
    void onWrite(int fd);
    void onError(int fd);
    void onKill();

public:
    QSocketNotifier *readNotifier;
    QSocketNotifier *writeNotifier;
    QSocketNotifier *errorNotifier;

    pa_io_event_destroy_cb_t destructor;

    void *userdata;
    pa_mainloop_api *a;
    pa_io_event_cb_t cb;
    bool aborted;
private:
    SocketNotifierWrapper(const SocketNotifierWrapper &o);
    const SocketNotifierWrapper &operator=(const SocketNotifierWrapper &);
};

struct QtPaMainLoop {
    pa_mainloop_api pa_vtable;

    QtPaMainLoop()
    {
        pa_vtable.userdata = this;

        pa_vtable.io_new = newIoEvent;
        pa_vtable.io_enable = setIoEnabled;
        pa_vtable.io_free = ioDestroy;
        pa_vtable.io_set_destroy = setIoDestructor;
        pa_vtable.time_new = newTimer;
        pa_vtable.time_restart = restartTimer;
        pa_vtable.time_free = freeTimer;
        pa_vtable.time_set_destroy = timerSetDestructor;
        pa_vtable.time_set_destroy = timerSetDestructor;

        pa_vtable.defer_new = newDefer;
        pa_vtable.defer_enable = setDeferEnabled;
        pa_vtable.defer_free = freeDefer;
        pa_vtable.defer_set_destroy = deferSetDestructor;

        pa_vtable.quit = quit;
    }

    static int msecsUntilTimeval(const struct timeval *tv)
    {
        timeval now;
        gettimeofday(&now, NULL);
        timeval diff;
        timersub(tv, &now, &diff);

        time_t target = diff.tv_sec * 1000;

        if (diff.tv_usec) {
            target += diff.tv_usec / 1000;
        }

        return target;

        //return QDateTime::currentDateTime().msecsTo(QDateTime::fromMSecsSinceEpoch(target));
    }

    static pa_time_event *newTimer(pa_mainloop_api *a, const struct timeval *tv, pa_time_event_cb_t callback, void *userdata)
    {
        TimerWrapper *timer = new TimerWrapper;
        timer->userdata = userdata;
        timer->timeCallback = callback;
        timer->a = a;
        timer->tv = tv;

        pa_time_event *timerEvent = reinterpret_cast<pa_time_event *>(timer);

        QObject::connect(timer, SIGNAL(timeout()), timer, SLOT(onTimeTimeout()));

        int duration = msecsUntilTimeval(tv);

        if (duration < 0) {
            std::cerr << "Invalid timer target, sec:" << tv->tv_sec << "usec" << tv->tv_usec << std::endl;
            duration = 0;
        }

        timer->start(duration);

        return timerEvent;
    }

    static void restartTimer(pa_time_event *e, const timeval *tv)
    {
        TimerWrapper *timer = reinterpret_cast<TimerWrapper *>(e);
        int duration = msecsUntilTimeval(tv);

        if (duration < 0) {
            std::cerr << "Invalid restart timer target, sec:" << tv->tv_sec << "usec" << tv->tv_usec << std::endl;
        }

        timer->start(duration);
    }

    static void freeTimer(pa_time_event *e)
    {
        TimerWrapper *timer = reinterpret_cast<TimerWrapper *>(e);
        timer->aborted = true;
        QTimer::singleShot(0, timer, SLOT(onKill()));
    }

    static void timerSetDestructor(pa_time_event *e, pa_time_event_destroy_cb_t destructor)
    {
        TimerWrapper *timer = reinterpret_cast<TimerWrapper *>(e);
        timer->timerDestructor = destructor;
    }

    static pa_io_event *newIoEvent(pa_mainloop_api *a, int fd, pa_io_event_flags_t events, pa_io_event_cb_t cb, void *userdata)
    {
        SocketNotifierWrapper *wrapper = new SocketNotifierWrapper;
        wrapper->userdata = userdata;
        wrapper->a = a;
        wrapper->cb = cb;

        pa_io_event *eventObject = reinterpret_cast<pa_io_event *>(wrapper);

        wrapper->readNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, qApp);
        QObject::connect(wrapper->readNotifier, SIGNAL(activated(int)), wrapper, SLOT(onRead(int)));

        wrapper->writeNotifier = new QSocketNotifier(fd, QSocketNotifier::Write, qApp);
        QObject::connect(wrapper->writeNotifier, SIGNAL(activated(int)), wrapper, SLOT(onWrite(int)));

        wrapper->errorNotifier = new QSocketNotifier(fd, QSocketNotifier::Exception, qApp);
        QObject::connect(wrapper->errorNotifier, SIGNAL(activated(int)), wrapper, SLOT(onError(int)));

        wrapper->readNotifier->setEnabled(events & PA_IO_EVENT_INPUT);
        wrapper->writeNotifier->setEnabled(events & PA_IO_EVENT_OUTPUT);
        wrapper->errorNotifier->setEnabled(events & PA_IO_EVENT_ERROR || events & PA_IO_EVENT_HANGUP);

        return eventObject;
    }

    static void setIoEnabled(pa_io_event *e, pa_io_event_flags_t events)
    {
//        puts("IO SET ENABLED");
        SocketNotifierWrapper *wrapper = reinterpret_cast<SocketNotifierWrapper *>(e);

        if (events & PA_IO_EVENT_HANGUP) {
            puts("HANGUP");
        }

        wrapper->readNotifier->setEnabled(events & PA_IO_EVENT_INPUT);
        wrapper->writeNotifier->setEnabled(events & PA_IO_EVENT_OUTPUT);
        wrapper->errorNotifier->setEnabled(events & PA_IO_EVENT_ERROR || events & PA_IO_EVENT_HANGUP);
    }

    static void ioDestroy(pa_io_event *e)
    {
//        puts("IO DESTROY");
        SocketNotifierWrapper *wrapper = reinterpret_cast<SocketNotifierWrapper *>(e);
        wrapper->aborted = true;

        wrapper->readNotifier->setEnabled(false);
        wrapper->writeNotifier->setEnabled(false);
        wrapper->errorNotifier->setEnabled(false);
        QTimer::singleShot(0, wrapper, SLOT(onKill()));
    }

    static void setIoDestructor(pa_io_event *e, pa_io_event_destroy_cb_t cb)
    {
        SocketNotifierWrapper *wrapper = reinterpret_cast<SocketNotifierWrapper *>(e);
        wrapper->destructor = cb;
    }
    static pa_defer_event *newDefer(pa_mainloop_api *a, pa_defer_event_cb_t callback, void *userdata)
    {
        TimerWrapper *timer = new TimerWrapper();
        timer->a = a;
        timer->deferCallback = callback;
        timer->userdata = userdata;

        pa_defer_event *eventObject = reinterpret_cast<pa_defer_event *>(timer);

        QTimer::singleShot(0, timer, SLOT(onDeferTimeout()));

        return eventObject;
    }

    static void setDeferEnabled(pa_defer_event *event, int enabled)
    {
        TimerWrapper *timer = reinterpret_cast<TimerWrapper *>(event);

        if (!enabled) {
            timer->aborted = true;
        } else {
            timer->aborted = false;
            QTimer::singleShot(0, timer, SLOT(onDeferTimeout()));
        }
    }

    static void freeDefer(pa_defer_event *event)
    {
        TimerWrapper *timer = reinterpret_cast<TimerWrapper *>(event);
        timer->aborted = true;
        QTimer::singleShot(0, timer, SLOT(onKill()));
    }

    static void deferSetDestructor(pa_defer_event *e, pa_defer_event_destroy_cb_t destructor)
    {
        TimerWrapper *timer = reinterpret_cast<TimerWrapper *>(e);
        timer->deferDestructor = destructor;
    }

    static void quit(pa_mainloop_api *a, int retval)
    {
        (void)a;
        std::cout << "Quit requested" << std::endl;

        qApp->exit(retval);
    }
};

bool connectToPulse();
