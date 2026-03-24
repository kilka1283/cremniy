#include "codeeditortab.h"
#include "QCodeEditor.hpp"
#include <qboxlayout.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qstackedlayout.h>
#include "filemanager.h"
#include "globalwidgetsmanager.h"
#include "utils.h"

#include "core/ToolTabFactory.h"

static bool registered = [](){
    ToolTabFactory::instance().registerTab("1", [](FileDataBuffer* buffer){
        return new CodeEditorTab(buffer);
    });
    return true;
}();

CodeEditorTab::CodeEditorTab(FileDataBuffer* buffer, QWidget *parent)
    : ToolTab{buffer, parent}
{

    // - - Create "Code Editor" Page - -

    m_codeEditorWidget = new QCodeEditor(this);

    QTextOption opt = m_codeEditorWidget->document()->defaultTextOption();
    opt.setTabStopDistance(20);
    m_codeEditorWidget->document()->setDefaultTextOption(opt);

    m_codeEditorWidget->document()->markContentsDirty(0, m_codeEditorWidget->document()->characterCount());
    m_codeEditorWidget->viewport()->update();

    // - - Create "Binary File Detected" Page - -

    m_overlayWidget = new QWidget(this);

    auto overlayLayout = new QVBoxLayout(m_overlayWidget);
    overlayLayout->setAlignment(Qt::AlignCenter);

    QLabel* title = new QLabel("Binary file detected");
    title->setStyleSheet("color: white; font-size: 20px;");
    title->setAlignment(Qt::AlignCenter);
    title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    overlayLayout->addWidget(title);
    overlayLayout->addSpacing(15);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setAlignment(Qt::AlignCenter);

    QPushButton* anywayOpenBtn = new QPushButton("Open anyway");

    btnLayout->addWidget(anywayOpenBtn);
    overlayLayout->addLayout(btnLayout);

    // - - Create and Init Stacked Layout Widget - -

    auto stack = new QStackedLayout;
    stack->setStackingMode(QStackedLayout::StackAll);
    stack->addWidget(m_codeEditorWidget);
    stack->addWidget(m_overlayWidget);

    m_overlayWidget->hide();

    this->setLayout(stack);

    // - - Connects - -

    // Trigger: Menu Bar: View->wordWrap - setWordWrapMode
    connect(GlobalWidgetsManager::instance().get_IDEWindow_menuBar_view_wordWrap(),
            &QAction::changed,
            this, [this]{
                if (GlobalWidgetsManager::instance().get_IDEWindow_menuBar_view_wordWrap()->isChecked())
                    m_codeEditorWidget->setWordWrapMode(QTextOption::WordWrap);
                else
                    m_codeEditorWidget->setWordWrapMode(QTextOption::NoWrap);
            });

    // Clicked: Open File Anyway Button
    connect(anywayOpenBtn, &QPushButton::clicked, this, [this]{
        forceSetData = true;
        this->setTabData();
    });

    // ContentsChanged: if new hash == old hash: dataEqual, else: signal modifyData
    connect(m_codeEditorWidget->document(),
            &QTextDocument::contentsChanged,
            this,
            [this](){
                // Проверяем только если документ действительно изменен
                if (!m_codeEditorWidget->document()->isModified()) return;
                
                QByteArray data = m_codeEditorWidget->getBData();
                uint newDataHash = qHash(data, 0);
                if (m_dataBuffer->originalHash() == newDataHash) {
                    setModifyIndicator(false);
                    emit dataEqual();
                }
                else{
                    if (!m_codeEditorWidget->m_ignoreModification) {
                        setModifyIndicator(true);
                        emit modifyData();
                    }
                }
            });

    // SelectionChanged: уведомляем буфер о выделении
    connect(m_codeEditorWidget, &QPlainTextEdit::selectionChanged,
            this, [this](){
                if (m_updatingSelection) return; // Предотвращаем рекурсию
                
                // Устанавливаем флаг перед отправкой сигнала
                m_updatingSelection = true;
                
                QTextCursor cursor = m_codeEditorWidget->textCursor();
                if (cursor.hasSelection()) {
                    int charStart = cursor.selectionStart();
                    int charEnd = cursor.selectionEnd();
                    
                    // Преобразуем позицию символа в позицию байта
                    QString text = m_codeEditorWidget->toPlainText();
                    QByteArray utf8Data = text.toUtf8();
                    
                    // Получаем байтовую позицию начала выделения
                    QString beforeSelection = text.left(charStart);
                    qint64 byteStart = beforeSelection.toUtf8().size();
                    
                    // Получаем длину выделения в байтах
                    QString selectedText = text.mid(charStart, charEnd - charStart);
                    qint64 byteLength = selectedText.toUtf8().size();
                    
                    // Уведомляем буфер о выделении
                    m_dataBuffer->setSelection(byteStart, byteLength);
                }
                
                m_updatingSelection = false;
            });

}

// - - override functions - -

// - public slots -

void CodeEditorTab::setFile(QString filepath){
    m_fileContext = new FileContext(filepath);
    QFileInfo fileInfo(filepath);
    QString ext = (fileInfo.suffix()).toLower();
    m_codeEditorWidget->setFileExt(ext);
}

void CodeEditorTab::setTabData(){

    qDebug() << "CodeEditorTab: setTabData";

    QByteArray data = m_dataBuffer->data();

    if (isBinary(data) && !forceSetData){
        m_codeEditorWidget->hide();
        m_overlayWidget->show();
    }
    else{
        m_codeEditorWidget->show();
        m_overlayWidget->hide();
        m_codeEditorWidget->setBData(data);
        forceSetData = false;
    }

    setModifyIndicator(false);
    emit dataEqual();
}

void CodeEditorTab::onSelectionChanged(qint64 pos, qint64 length)
{
    if (m_updatingSelection) return; // Предотвращаем рекурсию
    
    // Не обрабатываем, если выделение пришло от нас же
    m_updatingSelection = true;
    
    // Преобразуем байтовую позицию в символьную позицию
    QByteArray data = m_dataBuffer->data();
    QString text = QString::fromUtf8(data);
    
    // Получаем символьную позицию из байтовой
    QByteArray beforeSelection = data.left(pos);
    QString beforeText = QString::fromUtf8(beforeSelection);
    int charStart = beforeText.length();
    
    // Получаем длину выделения в символах
    QByteArray selectedBytes = data.mid(pos, length);
    QString selectedText = QString::fromUtf8(selectedBytes);
    int charLength = selectedText.length();
    
    // Выделяем соответствующий текст в редакторе
    QTextCursor cursor = m_codeEditorWidget->textCursor();
    
    cursor.setPosition(charStart);
    cursor.setPosition(charStart + charLength, QTextCursor::KeepAnchor);
    m_codeEditorWidget->setTextCursor(cursor);
    
    m_updatingSelection = false;
}

void CodeEditorTab::saveTabData() {
    qDebug() << "CodeEditorTab: saveTabData";

    QByteArray data = m_codeEditorWidget->getBData();
    
    if (!m_dataBuffer->isModified()) return;

    // Обновляем общий буфер
    m_dataBuffer->setData(data);

    // Сохраняем в файл
    FileManager::saveFile(m_fileContext, &data);

    m_codeEditorWidget->document()->setModified(false);

    setModifyIndicator(false);
    emit dataEqual();
    emit refreshDataAllTabsSignal();
}