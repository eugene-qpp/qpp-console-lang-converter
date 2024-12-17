#include "appwindow.hpp"
#include <set>
#include <sstream>
#include <chrono>
#include <QApplication>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QSettings>
#include <QClipboard>

#include "rapidcsv.h"
#include "convert.hpp"

AppWindow::AppWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("QPP Console Lang Converter");
    setFixedSize(640, 360);
    columnNameIndex = 1;
    rowNameIndex = 1;
    shouldReplaceBreakLines = true;
    QString lastSerial;

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
    QVariant lastSerialVariant = settings->value("lastSerial");
    if (!lastSerialVariant.isNull())
    {
        lastSerial = lastSerialVariant.toString();
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

    QLabel *serialLabel = new QLabel("Serial:", this);
    serialLabel->setGeometry(20, 260, 60, 40);
    serialTextEdit = new QTextEdit(lastSerial, this);
    serialTextEdit->setReadOnly(true);
    serialTextEdit->setGeometry(80, 265, 200, 30);

    QPushButton *copyToClipboardPushButton = new QPushButton("Copy", this);
    copyToClipboardPushButton->setGeometry(290, 260, 60, 40);

    convertButton = new QPushButton("Convert", this);
    convertButton->setDisabled(translationFilenameString == nullptr);
    convertButton->setGeometry(420, 260, 200, 60);

    connect(chooseFileButton, &QPushButton::clicked, this, &AppWindow::onChooseTranslationButtonClicked);
    connect(columnNameIndexSpinBox, &QSpinBox::valueChanged, this, &AppWindow::onColumnNameIndexChanged);
    connect(rowNameIndexSpinBox, &QSpinBox::valueChanged, this, &AppWindow::onRowNameIndexChanged);
    connect(copyToClipboardPushButton, &QPushButton::clicked, this, &AppWindow::onCopyToClipboardButtonClicked);
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

void AppWindow::onCopyToClipboardButtonClicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(serialTextEdit->toPlainText());
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

    // Generate new timestamp.
    int64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::string timestampStr = std::to_string(timestamp);

    try
    {
        QString oldTimestamp = serialTextEdit->toPlainText();
        if (oldTimestamp.size() > 0)
        {
            // Remove old translation files.
            for (auto &&langName : langNames)
            {
                QString filename = outputBaseFolder + "/" + langName.c_str() + "/common" + "-" + oldTimestamp + ".json";
                QFile::remove(filename);
            }
        }

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
            const QString filename = outputFolder + "/" + "common-" + timestampStr.c_str() + ".json";

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
        serialTextEdit->setText(timestampStr.c_str());
        settings->setValue("lastSerial", timestampStr.c_str());

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
