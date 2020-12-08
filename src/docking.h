/*********************************************************************************
 *
 * $Id: docking.h,v 1.2 1999/01/02 08:15:04 kiecza Exp $
 *
 *********************************************************************************/

#ifndef _DOCKING_H_
#define _DOCKING_H_

#include <stdio.h>
#include <qapp.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qpopmenu.h>
#include <qpoint.h>
#include <qcolor.h>


class DockWidget : public QWidget
{
    Q_OBJECT

public:
    DockWidget(const char *name = 0);
    ~DockWidget();

protected:
    void paintEvent(QPaintEvent *e);

private slots:
    void mousePressEvent(QMouseEvent *e);

public slots:
    void dock();
    void undock();

    void toggle_led_record()
    {
        record_state = !record_state;
        repaint();
    };
    void led_record_on()
    {
        record_state = true;
        repaint();
    };
    void led_record_off()
    {
        record_state = false;
        repaint();
    };

    void toggle_led_recognition()
    {
        if (recognition_state == 1) {
            recognition_state = 0;
        } else {
            recognition_state = 1;
        }

        repaint();
    };
    void led_recognition_on()
    {
        recognition_state = 1;
        repaint();
    };
    void led_recognition_off()
    {
        recognition_state = 0;
        repaint();
    };
    void led_recognition_success()
    {
        recognition_state = 2;
        repaint();
    };

    void toggle_window_state();

signals:
    void toggle_detect_mode();

public:
    const bool isDocked();

private:
    bool docked;
    int pos_x;
    int pos_y;
    QPopupMenu *popup_m;

    bool record_state;
    int recognition_state;
};

#endif

