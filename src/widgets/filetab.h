#ifndef FILETAB_H
#define FILETAB_H

#include "filestabwidget.h"
#include "tooltabwidget.h"
#include <QWidget>

class FileTab : public QWidget
{
    Q_OBJECT

private:
    ToolTabWidget *m_tooltabWidget;
    FilesTabWidget* m_filesTabWidget;

public:
    explicit FileTab(FilesTabWidget *ftparent, QString path, QWidget* parrent = nullptr);
    QString filePath;
    void saveFile();
    void openFile(int index = -1, int excluded_index = -1);
public slots:
    void giveData(int index);
};

#endif // FILETAB_H
