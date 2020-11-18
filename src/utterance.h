/*********************************************************************************
 *
 * $Id: utterance.h,v 1.2 1999/01/02 08:14:09 kiecza Exp $
 *
 *********************************************************************************/

#ifndef UTTERANCE_H
#define UTTERANCE_H

#include <qdstream.h> 
#include <qobject.h>


class Utterance : public QObject
{
  Q_OBJECT

public:

  Utterance();
  Utterance(float **, int);
  ~Utterance();

  void set_data(float **, int);
  void set_data(int, int, float);
  float **get_data();
  float get_data(int, int) const;

  float *operator[](int) const;

  void set_size(int);
  const int get_size() const { return (size);};
  
private:

  float **data;
  int size;
};


QDataStream& operator<<(QDataStream& , const Utterance *);
QDataStream& operator>>(QDataStream& , Utterance *);

#endif

