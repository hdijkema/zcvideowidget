#ifndef ZCVIDEOWIDGETSRTPARSER_H_
#define ZCVIDEOWIDGETSRTPARSER_H_

#include "zcvideowidget_global.h"
#include <QObject>
#include <QFile>

class zcVideoWidgetSrtParserData;

class ZCVIDEOWIDGET_EXPORT zcVideoWidgetSrtParser : public QObject
{
    Q_OBJECT;
private:
    zcVideoWidgetSrtParserData *D;
public:
    zcVideoWidgetSrtParser(QObject *parent = nullptr);
   ~zcVideoWidgetSrtParser();

public:
    bool setSrtFile(const QFile &f);
    void clear();

public:
    int size() const;
    const QTime   &from(int i) const;
    const QTime   &until(int i) const;
    const QString &subtitle(int i) const;
    int fromMs(int i) const;
    int untilMs(int i) const;

private:
    bool parse();
};

#endif
