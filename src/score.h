/*********************************************************************************
 *
 * $Id: score.h,v 1.5 1999/01/02 08:14:20 kiecza Exp $
 *
 *********************************************************************************/

#ifndef SCORE_H
#define SCORE_H

#include <qobject.h>
#include <qlist.h>
#include <qstring.h>

#include "utterance.h"
#include "reference.h"

#define MAX2(a,b) ((a>b)?(a):(b))
#define MIN2(a,b) ((a<b)?(a):(b))
#define MIN3(a,b,c) ( (a<b)?(MIN2(a,c)):(MIN2(b,c)) )

class Score : public QObject
{
  Q_OBJECT

public:

  Score();
  ~Score();

  QString *score(const Utterance *, QList<Reference> *);
  QString *branchNbound_score(const Utterance *, QList<Reference> *);

  float score(const Utterance *, const Utterance *);
  int get_adjust_win_width() {return adjustment_width;};
  float get_threshold() {return threshold;};
  float get_min_distance() {return min_distance;};

public slots:

  void set_adjust_width(int val) {adjustment_width =val;};
  void set_threshold(float val) {threshold =val;};
  void set_min_distance(float val) {min_distance = val;};


private:

  float euklid_distance(const float *, const float *);

  float **DTW_matrix;
  int adjustment_width;
  float threshold;
  float min_distance;
};


class BBQueueItem
{
public:
  BBQueueItem( float _s, uint _i, uint _p ) { sc=_s; idx=_i; p=_p; }
  float  score() const { return(sc); }
  uint  plane_index() const { return(idx); }
  uint pos()   const { return(p); }

  void set_score(float _s)      { sc=_s; }
  void set_plane_index(uint _i) { idx=_i; }
  void set_pos(uint _p)         { p=_p; }

  void step_right() { p++; }
  
private:
  float sc;
  uint idx;
  uint p;
};


#endif

