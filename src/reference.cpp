/*********************************************************************************
 *
 * $Id: reference.cpp,v 1.2 1999/01/02 08:11:06 kiecza Exp $
 *
 *********************************************************************************/

#include "reference.moc"


Reference::Reference()
{
    samples = new QList<Utterance>();
    samples->setAutoDelete(true);
    samples_name = new QList<QString>();
    samples_name->setAutoDelete(true);
    text = new QString("untitled");
    cmd_line = new QString("");
}


Reference::~Reference()
{
    delete samples;
    delete samples_name;
    delete text;
    delete cmd_line;
}


void Reference::set_command(const QString *_cmd_line)
{
    delete cmd_line;
    cmd_line = new QString(*_cmd_line);
}


void Reference::set_command(QString _cmd_line)
{
    delete cmd_line;
    cmd_line = new QString(_cmd_line);
}


void Reference::set_command(char *_cmd_line)
{
    delete cmd_line;
    cmd_line = new QString(_cmd_line);
}


void Reference::set_name(const QString *_text)
{
    delete text;
    text = new QString(*_text);
}


void Reference::set_name(QString _text)
{
    delete text;
    text = new QString(_text);
}


void Reference::set_name(char *_text)
{
    delete text;
    text = new QString(_text);
}


void Reference::reset_sample_names()
{
    delete samples_name;
    samples_name = new QList<QString>();
    samples_name->setAutoDelete(true);
}


const QString *Reference::get_sample_name(uint idx) const
{
    return (samples_name->at(idx));
}


void Reference::append_sample_name(const char *name)
{
    samples_name->append(new QString(name));
}


bool Reference::append_sample_utterance(Utterance *sampleUtterance)
{
    samples->append(sampleUtterance);
    return true;
}


bool Reference::remove_sample_utterance(uint index)
{
    samples->remove(index);
    return true;
}


const uint Reference::count() const
{
    return samples->count();
}


const Utterance *Reference::at(uint idx) const
{
    return samples->at(idx);
}


// ********************************************************************************
// ********************************************************************************
// ********************************************************************************


QDataStream &operator<<(QDataStream &out, const Reference &r)
{
    out << r.get_name();
    out << r.get_command();
    out << r.count();

    for (uint i = 0; i < r.count(); i++) {
        out << *(r.get_sample_name(i));
        out << r.at(i);
    }

    return out;
}


QDataStream &operator>>(QDataStream &in, Reference *r)
{
    char *s;
    in >> s;
    r->set_name(s);
    delete s;
    in >> s;
    r->set_command(s);
    delete s;

    uint count;
    in >> count;

    for (uint i = 0; i < count; i++) {
        char *name;
        in >> name;
        r->append_sample_name(name);
        delete name;

        Utterance *s = new Utterance();
        in >> s;
        r->append_sample_utterance(s);
    }

    return in;
}

