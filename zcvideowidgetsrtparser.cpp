#include "zcvideowidgetsrtparser.h"

#include <QVector>
#include <QTime>
#include <QString>
#include <QRegularExpression>

class zcVideoWidgetSrtParserData
{
public:
    QVector<QTime>      _from_times;
    QVector<QTime>      _until_times;
    QVector<QString>    _subs;
    QString             _error_string;
    QString             _srt;
};


zcVideoWidgetSrtParser::zcVideoWidgetSrtParser(QObject *parent)
    : QObject(parent)
{
    D = new zcVideoWidgetSrtParserData();
}

zcVideoWidgetSrtParser::~zcVideoWidgetSrtParser()
{
    delete D;
}

void zcVideoWidgetSrtParser::clear()
{
    D->_from_times.clear();
    D->_until_times.clear();
    D->_subs.clear();
}

bool zcVideoWidgetSrtParser::setSrtFile(const QFile &_f)
{
    QFile f(_f.fileName());

    clear();

    if (f.open(QIODevice::ReadOnly)) {
        D->_srt = QString::fromUtf8(f.readAll());
        return parse();
    } else {
        D->_error_string = tr("Cannot open Srt file %1").arg(f.fileName());
        return false;
    }
}

int zcVideoWidgetSrtParser::size() const
{
    return D->_subs.size();
}

const QTime &zcVideoWidgetSrtParser::from(int i) const
{
    return D->_from_times[i];
}

const QTime &zcVideoWidgetSrtParser::until(int i) const
{
    return D->_until_times[i];
}

const QString &zcVideoWidgetSrtParser::subtitle(int i) const
{
    return D->_subs[i];
}

int zcVideoWidgetSrtParser::fromMs(int i) const
{
    const QTime &tm = from(i);
    return QTime(0, 0).msecsTo(tm);
}

int zcVideoWidgetSrtParser::untilMs(int i) const
{
    const QTime &tm = until(i);
    return QTime(0, 0).msecsTo(tm);
}

#define EXPECT_NUM  1
#define EXPECT_TIME 2
#define EXPECT_SUB  3

bool zcVideoWidgetSrtParser::parse()
{
    D->_srt.replace("\r", "");
    QStringList lines = D->_srt.split("\n");

    auto matchToTime = [](QRegularExpressionMatch &m) {
        int hour = m.captured(1).toInt();
        int min = m.captured(2).toInt();
        int sec = m.captured(3).toInt();
        QString s = m.captured(5);
        while (s.length() < 3) { s += "0"; }
        int milli = s.toInt();
        QTime tm(hour, min, sec, milli);
        return tm;
    };

    QRegularExpression num("^\\s*([0-9]+)\\s*$");
    QRegularExpression time("([0-9]+)[:]([0-9]+)[:]([0-9]+)([,]([0-9]+)){0,1}");

    QString sub;
    QTime from, until;

    int state = EXPECT_NUM;

    int i, N;
    for(i = 0, N = lines.size(); i < N; i++) {
        QString line = lines[i].trimmed();
        if (state == EXPECT_NUM) {
            QRegularExpressionMatch m = num.match(line);
            if (m.hasMatch()) {
                state = EXPECT_TIME;
                sub = "";
            }
        } else if (state == EXPECT_TIME) {
            QRegularExpressionMatch m = time.match(line);
            if (m.hasMatch()) {
                from = matchToTime(m);
                until = from;
                line = line.mid(m.capturedEnd());
                m = time.match(line);
                if (m.hasMatch()) {
                    until = matchToTime(m);
                }
                state = EXPECT_SUB;
            }
        } else if (state == EXPECT_SUB) {
            if (line.length() > 0) {
                sub += "\n" + line;
            } else {
                sub = sub.trimmed();
                D->_from_times.append(from);
                D->_until_times.append(until);
                D->_subs.append(sub);
                state = EXPECT_NUM;
            }
        }
    }

    return  true;
}
