/********************************************************************************
** Form generated from reading UI file 'settingswindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGSWINDOW_H
#define UI_SETTINGSWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_SettingsWindow
{
public:
    QGridLayout *gridLayout;
    QFormLayout *formLayout;
    QLabel *pathLabel;
    QLabel *Themelabel;
    QComboBox *themeBox;
    QHBoxLayout *horizontalLayout;
    QLineEdit *pathEdit;
    QPushButton *pathOpen;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *SettingsWindow)
    {
        if (SettingsWindow->objectName().isEmpty())
            SettingsWindow->setObjectName(QString::fromUtf8("SettingsWindow"));
        SettingsWindow->resize(670, 304);
        gridLayout = new QGridLayout(SettingsWindow);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        pathLabel = new QLabel(SettingsWindow);
        pathLabel->setObjectName(QString::fromUtf8("pathLabel"));

        formLayout->setWidget(1, QFormLayout::LabelRole, pathLabel);

        Themelabel = new QLabel(SettingsWindow);
        Themelabel->setObjectName(QString::fromUtf8("Themelabel"));

        formLayout->setWidget(2, QFormLayout::LabelRole, Themelabel);

        themeBox = new QComboBox(SettingsWindow);
        themeBox->addItem(QString());
        themeBox->addItem(QString());
        themeBox->setObjectName(QString::fromUtf8("themeBox"));

        formLayout->setWidget(2, QFormLayout::FieldRole, themeBox);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pathEdit = new QLineEdit(SettingsWindow);
        pathEdit->setObjectName(QString::fromUtf8("pathEdit"));
        pathEdit->setMinimumSize(QSize(100, 0));
        pathEdit->setBaseSize(QSize(400, 0));

        horizontalLayout->addWidget(pathEdit);

        pathOpen = new QPushButton(SettingsWindow);
        pathOpen->setObjectName(QString::fromUtf8("pathOpen"));

        horizontalLayout->addWidget(pathOpen);


        formLayout->setLayout(1, QFormLayout::FieldRole, horizontalLayout);


        gridLayout->addLayout(formLayout, 0, 0, 1, 1);

        buttonBox = new QDialogButtonBox(SettingsWindow);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 1, 0, 1, 1);


        retranslateUi(SettingsWindow);
        QObject::connect(buttonBox, SIGNAL(accepted()), SettingsWindow, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), SettingsWindow, SLOT(reject()));

        QMetaObject::connectSlotsByName(SettingsWindow);
    } // setupUi

    void retranslateUi(QDialog *SettingsWindow)
    {
        SettingsWindow->setWindowTitle(QCoreApplication::translate("SettingsWindow", "Options Dialog", nullptr));
        pathLabel->setText(QCoreApplication::translate("SettingsWindow", "Game Engine path: ", nullptr));
        Themelabel->setText(QCoreApplication::translate("SettingsWindow", "Theme:", nullptr));
        themeBox->setItemText(0, QCoreApplication::translate("SettingsWindow", "Dark", nullptr));
        themeBox->setItemText(1, QCoreApplication::translate("SettingsWindow", "Light", nullptr));

        pathOpen->setText(QCoreApplication::translate("SettingsWindow", "Open", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SettingsWindow: public Ui_SettingsWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGSWINDOW_H
