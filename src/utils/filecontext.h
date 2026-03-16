#ifndef FILECONTEXT_H
#define FILECONTEXT_H

#include <cstdint>
#include <qobject.h>
class FileContext
{
public:
    // Класс который хранит информацию об открытом файле. Для каждого ToolTab отдельно (codeEditor, hexView и т.д)
    FileContext();

private:
    // путь к файлу (ссылка на FileTab->m_filePath)
    QString &m_filePath;
    // количество загруженных (текущих отображаемых) байто
    uint64_t m_bytesCount;
    // начало в файле (номер байта)
    uint64_t m_startOffset;
    // конец в файле (номер байта)
    uint64_t m_endOffset;
};

#endif // FILECONTEXT_H
