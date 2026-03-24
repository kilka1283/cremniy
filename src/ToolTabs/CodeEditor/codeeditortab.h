#ifndef CODEEDITORTAB_H
#define CODEEDITORTAB_H

#include "QCodeEditor/include/QCodeEditor.hpp"
#include "core/ToolTab.h"
#include <QWidget>
#include <qfileinfo.h>
#include <qlabel.h>

class CodeEditorTab : public ToolTab
{
    Q_OBJECT

private:

    /**
     * @brief Виджет редактора кода
    */
    QCodeEditor* m_codeEditorWidget;

    /**
     * @brief Главный виджет страницы "Binary File Detected"
    */
    QWidget* m_overlayWidget;

    /**
     * @brief Флаг принудительной установки данных
     *
     * Используется при нажатии пользователем на кнопку "Open Anyway" на странице "Binary File Detected"
    */
    bool forceSetData = false;
    
    /**
     * @brief Флаг для предотвращения рекурсии при обновлении выделения
    */
    bool m_updatingSelection = false;

public:
    explicit CodeEditorTab(FileDataBuffer* buffer, QWidget *parent = nullptr);

    QString toolName() const override { return "Code"; };
    QIcon toolIcon() const override { return QIcon(":/icons/code.png"); };

signals:

    /**
     * @brief Переключить на вкладку "Hex View"
     *
     * Используется при нажатии на кнопку "Open in HexView" на странице "Binary File Detected"
    */
    void switchHexViewTab();

protected slots:
    // Обработчик изменения выделения из буфера
    void onSelectionChanged(qint64 pos, qint64 length) override;

public slots:

    // From Parrent Class: ToolTab
    void setFile(QString filepath) override;
    void setTabData() override;
    void saveTabData() override;

};

#endif // CODEEDITORTAB_H
