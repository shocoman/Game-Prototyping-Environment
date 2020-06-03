#ifndef MAINWINDOW_H
#define MAINWINDOW_H


// Qt
#include <QMainWindow> // Required for inheritance
#include <QString>
#include <QVector>
#include <QPair>
#include <QTabWidget>
#include <QTextBrowser>
#include <QSplitter>
#include <QProcess>
#include <QScrollBar>
#include <QLineEdit>
#include <QDropEvent>
#include <QMimeData>
#include <QApplication>

#include "settingswindow.h"


class QVBoxLayout;
class QSyntaxStyle;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QCompleter;
class QStyleSyntaxHighlighter;
class QCodeEditor;


/**
 * @brief Class, that describes demo main window.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    /**
     * @brief Constructor.
     * @param parent Pointer to parent widget.
     */
    explicit MainWindow(QWidget* parent=nullptr);


protected:



private:
    QSettings *settings;
    SettingsWindow *settingsWindow;
    QPair<QString, QSyntaxStyle*> loadStyle(QString path);

    QString loadCode(QString path);

    void initActions();
    void newFile();
    void openFile();
    void justOpenFile(QString);

    void saveFile();
    void saveAsFile();
    void runCode();
    void quitProgram();
    void openSettings();
    void updateTabs();
    void showOutputWindow();

    void initData();
    void addNewTab(QString name);
    void createWidgets();

    void setupWidgets();

    void performConnections();

    QCodeEditor* getCurrentCodeEditor();

    QVBoxLayout* m_setupLayout;

    QComboBox* m_codeSampleCombobox;
    QComboBox* m_highlighterCombobox;
    QComboBox* m_completerCombobox;
    QComboBox* m_styleCombobox;

    QCheckBox* m_readOnlyCheckBox;
    QCheckBox* m_wordWrapCheckBox;
    QCheckBox* m_parenthesesEnabledCheckbox;
    QCheckBox* m_tabReplaceEnabledCheckbox;
    QSpinBox*  m_tabReplaceNumberSpinbox;
    QCheckBox* m_autoIndentationCheckbox;


    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);


    QVector<QPair<QString, QString>> m_codeSamples;
    QVector<QPair<QString, QCompleter*>> m_completers;
    QVector<QPair<QString, QStyleSyntaxHighlighter*>> m_highlighters;
    QVector<QPair<QString, QSyntaxStyle*>> m_styles;


    QMenu *fileMenu;
    QAction *newAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *loadAction;
    QAction *runAction;
    QAction *exitAction;
    QAction *settingsAction;
    QAction *fullscreenAction;
    QAction *showOutput;



    QTextEdit *outputTextEdit;
    QLineEdit *inputLineEdit;
    QSplitter *splitter;
    QProcess *engineProcess;

    QTabWidget *tabWidget;
    QVector<QCodeEditor*> codeEditors;
    void addTabShortcuts();

    void closeEvent(QCloseEvent*) override;
};


#endif // MAINWINDOW_H
