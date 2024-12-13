#include "appwindow.hpp"
#include <set>
#include <sstream>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QSettings>

#include "rapidcsv.h"
#include "convert.hpp"

AppWindow::AppWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("QPP Console Lang Converter");
    setFixedSize(640, 360);
    columnNameIndex = 1;
    rowNameIndex = 1;
    shouldReplaceBreakLines = true;

    settings = std::make_unique<QSettings>("settings.ini", QSettings::IniFormat);
    QVariant columnNameIndexVariant = settings->value("columnNameIndex");
    if (!columnNameIndexVariant.isNull())
    {
        columnNameIndex = columnNameIndexVariant.toInt();
    }
    QVariant rowNameIndexVariant = settings->value("rowNameIndex");
    if (!rowNameIndexVariant.isNull())
    {
        rowNameIndex = rowNameIndexVariant.toInt();
    }
    QVariant shouldReplaceBreakLinesVariant = settings->value("shouldReplaceBreakLines");
    if (!shouldReplaceBreakLinesVariant.isNull())
    {
        shouldReplaceBreakLines = shouldReplaceBreakLinesVariant.toBool();
    }
    QVariant translationFilenameVariant = settings->value("translationFilename");
    if (!translationFilenameVariant.isNull())
    {
        translationFilenameString = translationFilenameVariant.toString();
    }

    QLabel *filenameLabel = new QLabel("Translation file:", this);
    filenameLabel->setGeometry(20, 20, 80, 40);

    filenameTextEdit = new QTextEdit(this);
    filenameTextEdit->setGeometry(120, 20, 360, 40);
    filenameTextEdit->setReadOnly(true);
    filenameTextEdit->setText(translationFilenameString);

    QPushButton *chooseFileButton = new QPushButton("Choose", this);
    chooseFileButton->setGeometry(490, 20, 60, 40);

    QLabel *columnNameIndexLabel = new QLabel("Column name index:", this);
    columnNameIndexLabel->setGeometry(20, 80, 120, 40);

    QSpinBox *columnNameIndexSpinBox = new QSpinBox(this);
    columnNameIndexSpinBox->setGeometry(140, 80, 60, 40);
    columnNameIndexSpinBox->setValue(columnNameIndex);

    QLabel *rowNameIndexLabel = new QLabel("Row name index:", this);
    rowNameIndexLabel->setGeometry(20, 140, 120, 40);

    QSpinBox *rowNameIndexSpinBox = new QSpinBox(this);
    rowNameIndexSpinBox->setGeometry(140, 140, 60, 40);
    rowNameIndexSpinBox->setValue(rowNameIndex);

    QCheckBox *shouldReplaceBreakLinesCheckBox = new QCheckBox("Should replace break lines", this);
    shouldReplaceBreakLinesCheckBox->setGeometry(20, 200, 160, 40);
    shouldReplaceBreakLinesCheckBox->setChecked(shouldReplaceBreakLines);

    convertButton = new QPushButton("Convert", this);
    convertButton->setDisabled(translationFilenameString == nullptr);
    convertButton->setGeometry(210, 280, 200, 60);

    connect(chooseFileButton, &QPushButton::clicked, this, &AppWindow::onChooseTranslationButtonClicked);
    connect(columnNameIndexSpinBox, &QSpinBox::valueChanged, this, &AppWindow::onColumnNameIndexChanged);
    connect(rowNameIndexSpinBox, &QSpinBox::valueChanged, this, &AppWindow::onRowNameIndexChanged);
    connect(convertButton, &QPushButton::clicked, this, &AppWindow::onConvertButtonClicked);
}

void AppWindow::onChooseTranslationButtonClicked()
{
    translationFilenameString = QFileDialog::getOpenFileName(this, "Choose translation file", "", "CSV Files (*.csv)");
    if (translationFilenameString != nullptr)
    {
        filenameTextEdit->setText(translationFilenameString);
        convertButton->setDisabled(false);
        settings->setValue("translationFilename", translationFilenameString);
    }
}

void AppWindow::onColumnNameIndexChanged(int value)
{
    columnNameIndex = value;
    settings->setValue("columnNameIndex", columnNameIndex);
}

void AppWindow::onRowNameIndexChanged(int value)
{
    rowNameIndex = value;
    settings->setValue("rowNameIndex", rowNameIndex);
}

void AppWindow::onShouldReplaceBreakLinesChecked(bool checked)
{
    shouldReplaceBreakLines = checked;
    settings->setValue("shouldReplaceBreakLines", shouldReplaceBreakLines);
}

void AppWindow::onConvertButtonClicked()
{
    const QString outputBaseFolder = "locales";
    const std::string langNames[] = {"en", "zh"};
    const char *error = nullptr;
    std::stringstream duplicatedKeysMessageStream;

    QFile f(translationFilenameString);
    if (!f.exists())
    {
        QMessageBox::warning(this, "Convert", "The file does not exists.\n" + translationFilenameString, QMessageBox::StandardButton::Ok);
        return;
    }

    try
    {
        rapidcsv::Document doc = readCvs(translationFilenameString.toStdString(), columnNameIndex, rowNameIndex);
        for (auto &&langName : langNames)
        {
            // Create output directory if not exists.
            const QString outputFolder = outputBaseFolder + "/" + langName.c_str();
            QDir dir;
            if (!dir.exists(outputFolder))
            {
                dir.mkpath(outputFolder);
            }
            const QString filename = outputFolder + "/" + "common.json";

            const std::set<std::string> duplicatedKeys = writeJson(doc, langName, filename.toStdString(), shouldReplaceBreakLines);

            if (duplicatedKeys.size() > 0)
            {
                duplicatedKeysMessageStream << langName << ":\n";
                for (auto &&key : duplicatedKeys)
                {
                    duplicatedKeysMessageStream << "  " << key << "\n";
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        error = e.what();
    }

    if (error)
    {
        QMessageBox::critical(this, "Convert Error", error, QMessageBox::StandardButton::Ok);
    }
    else
    {
        QString duplicatedKeysMessage{duplicatedKeysMessageStream.str().c_str()};
        QString message;
        if (duplicatedKeysMessage.size() > 0)
        {
            message = "Succcess with duplicated keys:\n" + duplicatedKeysMessage;
        }
        else
        {
            message = "Success";
        }
        QMessageBox::information(this, "Convert Result", message, QMessageBox::StandardButton::Ok);
    }
}
