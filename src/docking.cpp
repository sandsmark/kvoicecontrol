/*********************************************************************************
 *
 * $Id: docking.cpp,v 1.3 1999/01/02 08:10:54 kiecza Exp $
 *
 *********************************************************************************/

#include <qtooltip.h>
#include <kwm.h>
#include <kapp.h>
#include <qpainter.h>

#include "docking.h"

#include "kvoicecontrol.h"

extern KVoiceControl *kvoicecontrol;

DockWidget::DockWidget(const char *name) : QWidget(0, name, 0)
{
    docked = false;

    pos_x = pos_y = 0;

    record_state = false;
    recognition_state = 0;
}


DockWidget::~DockWidget()
{
    // nothing
}


void DockWidget::dock()
{
    if (!docked) {
        // prepare panel to accept this widget
        KWM::setDockWindow(this->winId());

        // that's all the space there is
        this->setFixedSize(24, 24);

        // finally dock the widget
        this->show();
        docked = true;
    }
}


void DockWidget::undock()
{
    if (docked) {
        // the widget's window has to be destroyed in order
        // to undock from the panel. Simply using hide() is
        // not enough.
        this->destroy(true, true);

        // recreate window for further dockings
        this->create(0, true, false);

        docked = false;
    }
}


const bool DockWidget::isDocked()
{
    return docked;
}


void DockWidget::paintEvent(QPaintEvent *)
{
    QPainter p;

    p.begin(this);

    if (record_state) {
        p.setBrush(yellow);
    } else {
        p.setBrush(lightGray);
    }

    p.drawRect(5, 4, 15, 6);

    if (recognition_state == 2) {
        p.setBrush(green);
    } else if (recognition_state == 1) {
        p.setBrush(red);
    } else {
        p.setBrush(lightGray);
    }

    p.drawRect(5, 14, 15, 6);

    p.end();

}


void DockWidget::mousePressEvent(QMouseEvent *e)
{

    // open/close connect-window on right mouse button

    if (e->button() == LeftButton) {
        toggle_window_state();
    }

    // open popup menu on left mouse button

    if (e->button() == RightButton) {
        emit toggle_detect_mode();
    }
}


void DockWidget::toggle_window_state()
{
    // restore/hide connect-window
    if (kvoicecontrol != 0L) {
        if (kvoicecontrol->isVisible()) {
            QPoint point = kvoicecontrol->mapToGlobal(QPoint(0, 0));
            pos_x = point.x();
            pos_y = point.y();
            kvoicecontrol->hide();
        } else {
            kvoicecontrol->setGeometry(
                pos_x,
                pos_y,
                kvoicecontrol->width(),
                kvoicecontrol->height());
            kvoicecontrol->show();
        }
    }
}


#include "docking.moc"
