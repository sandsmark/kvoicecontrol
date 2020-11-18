/*********************************************************************************
 *
 * $Id: options.cpp,v 1.2 1999/01/02 08:10:58 kiecza Exp $
 *
 *********************************************************************************/

/**********************************************************************

	--- Qt Architect generated file ---

	File: options.cpp
	Last generated: Sun Apr 26 22:39:42 1998

 *********************************************************************/

#include "options.moc"
#include "optionsdata.moc"

#define Inherited OptionsData

Options::Options
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name )
{
	setCaption( "Options" );
}


Options::~Options()
{
}

void Options::set_rec_thresh(int threshold) 
{
  QString s;
  s.sprintf("%d", threshold);
  rec_thresh->setText(s);
};

void Options::set_acc_sil_frames(int frames)
{
  QString s;
  s.sprintf("%d", frames);
  acc_sil_frames->setText(s);
}

void Options::set_adj_win_width(int width)
{
  QString s;
  s.sprintf("%d", width);
  adj_win_width->setText(s);
}

void Options::set_reject_thresh(float threshold)
{
  QString s;
  s.sprintf("%f", threshold);
  rej_thresh->setText(s);
}

void Options::set_min_dist(float min)
{
  QString s;
  s.sprintf("%f", min);
  min_dist->setText(s);
}

void Options::my_accept()
{
  // ***** emit signals to change data
  
  rec_level_thresh_changed(QString(rec_thresh->text()).toInt());
  acc_sil_frames_changed(QString(acc_sil_frames->text()).toInt());

  adj_win_width_changed(QString(adj_win_width->text()).toInt());
  reject_thresh_changed(QString(rej_thresh->text()).toFloat());
  min_dist_changed(QString(min_dist->text()).toFloat());

  accept(); 
}
