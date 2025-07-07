#ifndef ADDMOVIEDIALOG_H
#define ADDMOVIEDIALOG_H

#include <QDialog>
#include <QPixmap>
#include <QLabel>
#include <QSlider>

class QLineEdit;
class QPushButton;
class QTextEdit;

class AddMovieDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddMovieDialog(QWidget *parent = nullptr);

signals:
    void movieAdded(const QString &title, const QString &year, const QString &genre, int rating, const QString &coverPath, const QString &review);

private slots:
    void onAddClicked();
    void onCancelClicked();
    void onChooseCoverClicked();

private:
    QLineEdit *titleEdit;
    QLineEdit *yearEdit;
    QLineEdit *genreEdit;
    QTextEdit *reviewEdit;
    QLabel *ratingLabels[10];
    int currentRating = 0;
    QLabel *coverLabel;
    QPushButton *chooseCoverButton;
    QPixmap coverPixmap;
    QString coverPath;
    QPushButton *addButton;
    QPushButton *cancelButton;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void updateRatingLabels(int rating);
};

#endif // ADDMOVIEDIALOG_H 