/*********************************************************************************
 *
 * $Id: options.h,v 1.2 1999/01/02 08:14:55 kiecza Exp $
 *
 *********************************************************************************/

/**********************************************************************

	--- Qt Architect generated file ---

	File: options.h
	Last generated: Sun Apr 26 22:39:42 1998

 *********************************************************************/

#ifndef Options_included
#define Options_included

#include "optionsdata.h"

class Options : public OptionsData
{
    Q_OBJECT

public:

    Options
    (
        QWidget *parent = NULL,
        const char *name = NULL
    );

    virtual ~Options();

    void set_rec_thresh(int threshold);
    void set_acc_sil_frames(int frames);

    void set_adj_win_width(int width);
    void set_reject_thresh(float threshold);
    void set_min_dist(float min);

signals:

    void rec_level_thresh_changed(int threshold);
    void acc_sil_frames_changed(int frames);

    void adj_win_width_changed(int width);
    void reject_thresh_changed(float threshold);
    void min_dist_changed(float min);


protected slots:

    virtual void my_accept();
};
#endif // Options_included
