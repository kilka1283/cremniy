#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <qobject.h>
class FileManager
{
public:

    // Глобальный класс необходимый для работы с файлами
    static FileManager& instance() {
        static FileManager inst;
        return inst;
    }


    // Action methods
    void saveFile(QByteArray* data);
    QByteArray* openFile();


};

#endif // FILEMANAGER_H
