// Demo
#include <mainwindow.h>

// QCodeEditor
#include <QCodeEditor>
#include <QLuaCompleter>
#include <QSyntaxStyle>
#include <QLuaHighlighter>

// Qt
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QProcess>
#include <QShortcut>
#include <QPushButton>


MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    m_setupLayout(nullptr),
    m_codeSampleCombobox(nullptr),
    m_highlighterCombobox(nullptr),
    m_completerCombobox(nullptr),
    m_styleCombobox(nullptr),
    m_readOnlyCheckBox(nullptr),
    m_wordWrapCheckBox(nullptr),
    m_parenthesesEnabledCheckbox(nullptr),
    m_tabReplaceEnabledCheckbox(nullptr),
    m_tabReplaceNumberSpinbox(nullptr),
    m_autoIndentationCheckbox(nullptr),
    m_completers(),
    m_highlighters(),
    m_styles()
{
    resize(1000, 700);

    settings = new QSettings("QT", "CodeEditor");
//    settings->setValue("engine_path", "C:/Users/shoco/source/repos/DrawingLib/Debug/DrawingLib.exe");


    initData();
    createWidgets();
    setupWidgets();
    performConnections();

    initActions();

    addTabShortcuts();



    updateTabs();
}


void MainWindow::dragEnterEvent(QDragEnterEvent *event){
    if (event->mimeData()->hasUrls()){
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event){
    for (QUrl &file : event->mimeData()->urls()){
        justOpenFile(file.path().remove(0,1));
    }
}

void MainWindow::addTabShortcuts(){
    // close tab shortcut
    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+W"), this);
    connect(shortcut, &QShortcut::activated, tabWidget, [&](){
        tabWidget->tabCloseRequested(tabWidget->currentIndex());
    });

    // choose tab by ctrl+1, etc
    for (int i = 1; i <= 9; i++){
        QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+" + QString::number(i)), this);
        connect(shortcut, &QShortcut::activated, tabWidget, [&, i](){
            if (i <= tabWidget->count()) {
                tabWidget->setCurrentIndex(i-1);
            }
        });
    }

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    auto reply = QMessageBox::question(this, "Save", "Do you want to save changes?",
                          QMessageBox::Save, QMessageBox::Discard, QMessageBox::Cancel);

    if (reply == QMessageBox::Save){
        saveFile();

    } else if (reply == QMessageBox::Discard){

    } else if (reply == QMessageBox::Cancel){
        event->ignore();
    }
}



void MainWindow::initActions(){
    newAction = new QAction("New file", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

    saveAction = new QAction("Save file", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);

    saveAsAction = new QAction("Save As file", this);
    saveAsAction->setShortcut(QKeySequence("Shift+Ctrl+S"));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAsFile);

    loadAction = new QAction("Load file", this);
    loadAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(loadAction, &QAction::triggered, this, &MainWindow::openFile);

    runAction = new QAction("Run code", this);
    runAction->setShortcut(QKeySequence("Ctrl+R"));
    connect(runAction, &QAction::triggered, this, &MainWindow::runCode);

    exitAction = new QAction("Exit", this);
    connect(exitAction, &QAction::triggered, this, &QApplication::closeAllWindows, Qt::QueuedConnection);

    settingsAction = new QAction("Open settings", this);
    settingsAction->setShortcut(QKeySequence("Ctrl+O"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettings);


    showOutput = new QAction("Show output window", this);
    showOutput->setShortcut(QKeySequence("Ctrl+`"));
    connect(showOutput, &QAction::triggered, this, &MainWindow::showOutputWindow);


    fullscreenAction = new QAction("Fullscreen", this);
    fullscreenAction->setShortcut(QKeySequence("F11"));
    connect(fullscreenAction, &QAction::triggered, this, [this](){
        if (this->isFullScreen()) {
            this->showNormal();
        } else {
            this->showFullScreen();
        }
    });


    fileMenu = menuBar()->addMenu("&File");
    fileMenu->addActions({ newAction, loadAction, saveAction, saveAsAction});
    fileMenu->addSeparator();
    fileMenu->addActions({ runAction});
    fileMenu->addSeparator();
    fileMenu->addActions({ settingsAction, showOutput, fullscreenAction});
    fileMenu->addSeparator();
    fileMenu->addActions({exitAction});
}


void MainWindow::newFile(){
    addNewTab("New tab");
}

void MainWindow::justOpenFile(QString filePath){

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::critical(this, "Error", "File reading error! File: " + filePath);
        return;
    }

    QFileInfo fileInfo(file);
    QString fileContent = file.readAll();

    addNewTab(fileInfo.fileName());
    getCurrentCodeEditor()->setPlainText(fileContent);
    getCurrentCodeEditor()->setFilePath(filePath);
}

void MainWindow::openFile(){

    QString filePath = QFileDialog::getOpenFileName(this, "Open text file");
    if (filePath.length() == 0) return;

    justOpenFile(filePath);

}

void MainWindow::saveFile(){
    if (getCurrentCodeEditor() == nullptr) return;
    auto currentFilePath = getCurrentCodeEditor()->getFilePath();
    if (currentFilePath.length() == 0) {
        saveAsFile();
        return;
    }

    QFile file(currentFilePath);
    if (!file.open(QIODevice::WriteOnly)){
        QMessageBox::critical(this, "Error", "File writing error! File: " + currentFilePath);
    }
    QFileInfo fileInfo(file);
    tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.fileName());
    getCurrentCodeEditor()->setFilePath(currentFilePath);

    QString fileContent = getCurrentCodeEditor()->toPlainText();
    QTextStream textStream(&file);
    textStream.setCodec("UTF-8");
    textStream << fileContent;
}

void MainWindow::saveAsFile(){
    if (getCurrentCodeEditor() == nullptr) return;
    QString filePath = QFileDialog::getSaveFileName(this, "Save file");
    if (filePath.length() == 0) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)){
        QMessageBox::critical(this, "Error", "File writing error! File: " + filePath);
    }

    QFileInfo fileInfo(file);
    tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.fileName());
    QString fileContent = getCurrentCodeEditor()->toPlainText();
    getCurrentCodeEditor()->setFilePath(filePath);
    QTextStream textStream(&file);
    textStream.setCodec("UTF-8");
    textStream << fileContent;
}


void MainWindow::runCode(){
    // cant find active editor
    if (getCurrentCodeEditor() == nullptr) return;
    saveFile();
    auto currentFilePath = getCurrentCodeEditor()->getFilePath();
    if (currentFilePath.length() == 0) return;


    QString exePath = settings->value("engine_path").toString();
    if (exePath.length() == 0) return;



    if (engineProcess->state() == QProcess::ProcessState::NotRunning) {
        outputTextEdit->clear();
        engineProcess->start(exePath, {currentFilePath});
    } else {
        engineProcess->close();
    }

}

void MainWindow::quitProgram(){
    exit(0);
}

void MainWindow::openSettings()
{
    settingsWindow = new SettingsWindow(this, settings);
    auto result = settingsWindow->exec();

    if (result == QDialog::Accepted)
    {
        updateTabs();
    }
}

void MainWindow::updateTabs()
{
    QFile file(":/stylesheets.qss");
    file.open(QIODevice::ReadOnly);
    QString text = QLatin1String(file.readAll());
    if (settings->value("theme") == "Light") {
        this->setStyleSheet("QCodeEditor {background-color: white;}");
    } else if (settings->value("theme") == "Dark") {
        this->setStyleSheet(text);
    }

    auto defaultStyle = QSyntaxStyle::defaultStyle();
    auto darkStyle = loadStyle(":/styles/drakula.xml").second;
    for (auto &editor : codeEditors) {
        if (settings->value("theme") == "Light") {
            editor->setSyntaxStyle(defaultStyle);
        } else if (settings->value("theme") == "Dark") {
            editor->setSyntaxStyle(darkStyle);
        }
    }

}

void MainWindow::showOutputWindow()
{
    bool isOutputHidden = splitter->sizes()[1] == 0;
    if (isOutputHidden){
        auto lastSize = settings->value("splitterSize").toByteArray();
        splitter->restoreState(lastSize);
    } else {
        settings->setValue("splitterSize", splitter->saveState());
        splitter->setSizes({999,0});
    }
}




void MainWindow::initData()
{
    m_codeSamples = {
        {"LUA",  loadCode(":/code_samples/lua.lua")},
    };

    m_completers = {
        {"None", nullptr},
        {"LUA", new QLuaCompleter(this)},
    };

    m_highlighters = {
        {"LUA",  new QLuaHighlighter},
    };

    m_styles = {
        {"Default", QSyntaxStyle::defaultStyle()},
        loadStyle(":/styles/drakula.xml") // Darcula
    };
}

QString MainWindow::loadCode(QString path)
{
    QFile fl(path);

    if (!fl.open(QIODevice::ReadOnly))
    {
        return QString();
    }

    return fl.readAll();
}

QPair<QString, QSyntaxStyle*> MainWindow::loadStyle(QString path)
{
    QFile fl(path);
    fl.open(QIODevice::ReadOnly);

    auto style = new QSyntaxStyle(this);
    style->load(fl.readAll());

    return {style->name(), style};
}

void MainWindow::createWidgets()
{
    // Layout
    auto container = new QWidget(this);    container->setContentsMargins(0,0,0,0);
    setCentralWidget(container);

    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, [&](int index){
        codeEditors[index]->deleteLater();
        codeEditors.remove(index);
    });



    auto outputWidget = new QWidget(container);

    outputTextEdit = new QTextEdit(outputWidget);
    inputLineEdit = new QLineEdit(outputWidget);

    auto outputInputTextLayout = new QVBoxLayout(outputWidget);

    auto outputTextLayout = new QHBoxLayout();
    auto inputLineLayout = new QHBoxLayout();
    outputTextLayout->addWidget(new QLabel("Output: ", outputWidget), 0, Qt::AlignmentFlag::AlignTop);
    outputTextLayout->addWidget(outputTextEdit);

    inputLineLayout->addWidget(new QLabel("  Input:  ", outputWidget));
    inputLineLayout->addWidget(inputLineEdit);

    outputInputTextLayout->setSpacing(1);
    outputInputTextLayout->addLayout(outputTextLayout);
    outputInputTextLayout->addLayout(inputLineLayout);


    splitter = new QSplitter(Qt::Orientation::Vertical, container);
    auto vBox = new QVBoxLayout(container);
    vBox->addWidget(splitter);


    splitter->addWidget(tabWidget);
    splitter->addWidget(outputWidget);
    splitter->setHandleWidth(0);

    vBox->setContentsMargins(0,0,0,0);
    splitter->setContentsMargins(0,0,0,0);
    outputInputTextLayout->setContentsMargins(4,2,4,4);
    inputLineLayout->setContentsMargins(0,0,0,0);
    outputTextLayout->setContentsMargins(0,0,0,0);
    outputWidget->setContentsMargins(0,0,0,0);
    tabWidget->setContentsMargins(0,0,0,0);
    container->setContentsMargins(0,0,0,0);
}


void MainWindow::addNewTab(QString name){
    auto newCodeTab = new QCodeEditor(this);
    codeEditors.push_back(newCodeTab);
    auto index = tabWidget->addTab(newCodeTab, name);
    tabWidget->setCurrentIndex(index);

    updateTabs();
}


void MainWindow::setupWidgets()
{
    setWindowTitle("MyCodeEditor");
    engineProcess = new QProcess(this);
    engineProcess->setProcessChannelMode(QProcess::MergedChannels);
    connect(engineProcess, &QProcess::readyReadStandardOutput, this,
            [&](){
        QString readBA = engineProcess->readAllStandardOutput();

        outputTextEdit->append(readBA);
        outputTextEdit->verticalScrollBar()->setValue(outputTextEdit->verticalScrollBar()->maximum());
    });

    connect(engineProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            [&](int exitCode, QProcess::ExitStatus status){
        QString msg = "Process finished. Exit code: " + QString::number(exitCode);
        outputTextEdit->append(msg);
        outputTextEdit->verticalScrollBar()->setValue(outputTextEdit->verticalScrollBar()->maximum());
    });

    outputTextEdit->setFontPointSize(15);

    addNewTab("New Tab");

    // collapse (hide) second widget (output text edit)
    settings->setValue("splitterSize", splitter->saveState());
    splitter->setSizes({500, 100});

//    inputLineEdit
    connect(inputLineEdit, &QLineEdit::returnPressed, this,
            [&](){
        engineProcess->write((inputLineEdit->text() + "\n").toUtf8());
    });
    setAcceptDrops(true);



}



void MainWindow::performConnections()
{

}


QCodeEditor *MainWindow::getCurrentCodeEditor()
{
    auto currentTab = tabWidget->currentIndex();
    if (currentTab == -1) return nullptr;
    return codeEditors[currentTab];
}
