/*********************************************************************************
 *
 * $Id: reference.h,v 1.2 1999/01/02 08:14:43 kiecza Exp $
 *
 *********************************************************************************/

#ifndef REFERENCE_H
#define REFERENCE_H

#include <qobject.h>
#include <qlist.h>
#include <qdstream.h>

#include "utterance.h"

class Reference : public QObject
{
    Q_OBJECT

public:

    Reference();
    ~Reference();

    void set_command(const QString *);
    void set_command(QString);
    void set_command(char *);

    void set_name(const QString *);
    void set_name(QString);
    void set_name(char *);

    const QString get_command() const
    {
        return (*cmd_line);
    };
    const QString get_name()    const
    {
        return (*text);
    };

    const QString *get_sample_name(uint idx) const;

    void reset_sample_names();
    void append_sample_name(const char *);
    bool append_sample_utterance(Utterance *);
    bool remove_sample_utterance(uint);

    const uint count() const;
    const Utterance *at(uint idx) const;

private:

    QString *text;
    QString *cmd_line;
    QList<Utterance> *samples;
    QList<QString> *samples_name;
};


QDataStream &operator<<(QDataStream &, const Reference &);
QDataStream &operator>>(QDataStream &, Reference *);

#endif

