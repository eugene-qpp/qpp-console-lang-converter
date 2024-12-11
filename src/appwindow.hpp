#ifndef APP_WINDOW_HPP
#define APP_WINDOW_HPP

#include <QWidget>
#include <QSettings>
#include <QSharedPointer>

class QTextEdit;
class QString;
class QPushButton;

class AppWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AppWindow(QWidget *parent = nullptr);

private slots:
    void onChooseTranslationButtonClicked();
    void onColumnNameIndexChanged(int);
    void onRowNameIndexChanged(int);
    void onShouldReplaceBreakLinesChecked(bool);
    void onConvertButtonClicked();

private:
    QSharedPointer<QSettings> settings;
    QTextEdit *filenameTextEdit;
    QString translationFilenameString;
    QPushButton *convertButton;
    int32_t columnNameIndex;
    int32_t rowNameIndex;
    bool shouldReplaceBreakLines;
};

#endif // APP_WINDOW_HPP
