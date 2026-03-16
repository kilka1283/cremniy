#ifndef HEXVIEWTAB_H
#define HEXVIEWTAB_H

#include "QHexView/qhexview.h"
#include "tooltab.h"
#include <QWidget>
#include <qfileinfo.h>

class HexViewTab : public QWidget, public ToolTab
{
    Q_OBJECT

private:
    QHexView* m_hexViewWidget;
    QWidget* createPage();

public:
    explicit HexViewTab(QWidget *parent, QString path);

    void saveToFile(QString path) override {
        QFile f(path);
        if (!f.open(QFile::WriteOnly)) return;
        f.write(m_hexViewWidget->getBData());
        f.close();
    };

    void setTabData(QByteArray &data) override {
        m_hexViewWidget->setBData(data);
    };

signals:
    void modifyData(bool modified);
};

#endif // HEXVIEWTAB_H
