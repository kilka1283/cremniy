#include "FileDataBuffer.h"

FileDataBuffer::FileDataBuffer(QObject* parent)
    : QObject(parent)
{
}

QByteArray FileDataBuffer::data() const
{
    QMutexLocker locker(&m_mutex);
    return m_data;
}

void FileDataBuffer::setData(const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);
    m_data = data;
    m_originalHash = qHash(data, 0);
    locker.unlock();
    emit dataChanged();
}

void FileDataBuffer::setByte(qint64 pos, char byte)
{
    QMutexLocker locker(&m_mutex);
    if (pos < 0 || pos >= m_data.size())
        return;

    m_data[pos] = byte;
    locker.unlock();
    emit byteChanged(pos);
}

char FileDataBuffer::getByte(qint64 pos) const
{
    QMutexLocker locker(&m_mutex);
    if (pos < 0 || pos >= m_data.size())
        return 0;

    return m_data[pos];
}

void FileDataBuffer::setBytes(qint64 pos, const QByteArray& bytes)
{
    QMutexLocker locker(&m_mutex);
    if (pos < 0 || pos >= m_data.size())
        return;

    qint64 length = qMin(bytes.size(), m_data.size() - pos);
    for (qint64 i = 0; i < length; ++i) {
        m_data[pos + i] = bytes[i];
    }
    locker.unlock();
    emit bytesChanged(pos, length);
}

qint64 FileDataBuffer::size() const
{
    QMutexLocker locker(&m_mutex);
    return m_data.size();
}

void FileDataBuffer::setSelection(qint64 pos, qint64 length)
{
    QMutexLocker locker(&m_mutex);
    if (m_selectionPos == pos && m_selectionLength == length)
        return;

    m_selectionPos = pos;
    m_selectionLength = length;
    locker.unlock();
    emit selectionChanged(pos, length);
}

void FileDataBuffer::getSelection(qint64& pos, qint64& length) const
{
    QMutexLocker locker(&m_mutex);
    pos = m_selectionPos;
    length = m_selectionLength;
}

uint FileDataBuffer::originalHash() const
{
    QMutexLocker locker(&m_mutex);
    return m_originalHash;
}

uint FileDataBuffer::currentHash() const
{
    QMutexLocker locker(&m_mutex);
    return qHash(m_data, 0);
}

bool FileDataBuffer::isModified() const
{
    QMutexLocker locker(&m_mutex);
    return qHash(m_data, 0) != m_originalHash;
}

void FileDataBuffer::resetModified()
{
    QMutexLocker locker(&m_mutex);
    m_originalHash = qHash(m_data, 0);
}