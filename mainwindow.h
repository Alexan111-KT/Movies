#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QDialog>
#include <QCloseEvent>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QMouseEvent>
#include "coverlabel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTableWidget *tableWidget;
    void saveMoviesToFile();
    bool exportMoviesToFile(const QString &filePath);
    void loadMoviesFromFile();
    QString lastImportedFilePath = "";
    void saveLastFilePath();
    void loadLastFilePath();
protected:
    void closeEvent(QCloseEvent *event) override;
    // Здесь можно добавить методы для работы с таблицей фильмов

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void onMovieAdded(const QString &title, const QString &year, const QString &genre, int rating, const QString &coverPath, const QString &review);
    void onTableRowSelected();
    void onSaveEditClicked();

private:
    CoverLabel *coverLabel;
    QLineEdit *editTitle;
    QLineEdit *editYear;
    QLineEdit *editGenre;
    QTextEdit *editReview;
    QPushButton *saveEditButton;
    int editingRow = -1;
    QVector<QPushButton*> ratingButtons;
    int editRatingValue = 0;
    void ensureAllItems();
};

class AddMovieDialog;

#endif // MAINWINDOW_H
