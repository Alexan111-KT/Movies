#include "addmoviedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QIcon>
#include <QEvent>
#include <QIntValidator>
#include <QLabel>
#include <QFileDialog>
#include <QPixmap>
#include <QFont>
#include <QSlider>
#include <QMainWindow>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QTextEdit>
#include <QRegExpValidator>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QDate>

AddMovieDialog::AddMovieDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Добавить фильм");
    QHBoxLayout *mainRow = new QHBoxLayout;
    QVBoxLayout *fieldsLayout = new QVBoxLayout;

    titleEdit = new QLineEdit;
    titleEdit->setPlaceholderText("Название");
    QFont bigFont;
    bigFont.setPointSize(18);
    titleEdit->setFont(bigFont);
    titleEdit->setMinimumHeight(40);
    titleEdit->setStyleSheet("QLineEdit { background: white; } QLineEdit:focus { border: 2px solid #03a9f4; background: #e0f7fa; }");
    fieldsLayout->addWidget(titleEdit);

    yearEdit = new QLineEdit;
    yearEdit->setPlaceholderText("Год");
    yearEdit->setValidator(new QIntValidator(0, 9999, this));
    yearEdit->setFont(bigFont);
    yearEdit->setMinimumHeight(40);
    yearEdit->setStyleSheet("QLineEdit { background: white; } QLineEdit:focus { border: 2px solid #03a9f4; background: #e0f7fa; }");
    fieldsLayout->addWidget(yearEdit);

    connect(yearEdit, &QLineEdit::editingFinished, this, [this]() {
        bool ok = false;
        int year = yearEdit->text().toInt(&ok);
        int currentYear = QDate::currentDate().year();
        if (!yearEdit->text().isEmpty() && (!ok || year > currentYear)) {
            yearEdit->clear();
            QMessageBox::warning(this, "Ошибка", QString("Год выпуска не может быть больше %1").arg(currentYear));
        }
    });

    genreEdit = new QLineEdit;
    genreEdit->setPlaceholderText("Жанр");
    genreEdit->setFont(bigFont);
    genreEdit->setMinimumHeight(40);
    genreEdit->setValidator(new QRegExpValidator(QRegExp("[A-Za-zА-Яа-яЁё\s]+"), this));
    genreEdit->setStyleSheet("QLineEdit { background: white; } QLineEdit:focus { border: 2px solid #03a9f4; background: #e0f7fa; }");
    fieldsLayout->addWidget(genreEdit);

    reviewEdit = new QTextEdit;
    reviewEdit->setPlaceholderText("Общее впечатление");
    reviewEdit->setFixedHeight(60);
    reviewEdit->setFont(bigFont);
    reviewEdit->setStyleSheet("QTextEdit { background: white; } QTextEdit:focus { border: 2px solid #03a9f4; background: #e0f7fa; }");
    fieldsLayout->addWidget(reviewEdit);

    QLabel *ratingLabel = new QLabel("Рейтинг:");
    ratingLabel->setFont(bigFont);
    fieldsLayout->addWidget(ratingLabel);
    QHBoxLayout *numbersLayout = new QHBoxLayout;
    for (int i = 0; i < 10; ++i) {
        ratingLabels[i] = new QLabel(QString::number(i + 1));
        ratingLabels[i]->setFont(bigFont);
        ratingLabels[i]->setAlignment(Qt::AlignCenter);
        ratingLabels[i]->setFixedSize(32, 32);
        ratingLabels[i]->setCursor(Qt::PointingHandCursor);
        ratingLabels[i]->setStyleSheet("color: #555;");
        ratingLabels[i]->setMinimumHeight(36);
        ratingLabels[i]->setMinimumWidth(36);
        numbersLayout->addWidget(ratingLabels[i]);
        ratingLabels[i]->installEventFilter(this);
    }
    fieldsLayout->addLayout(numbersLayout);
    // Spacer для выравнивания расстояния между рейтингом и кнопками
    fieldsLayout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    addButton = new QPushButton("Добавить фильм");
    cancelButton = new QPushButton("Отмена");
    addButton->setFont(bigFont);
    addButton->setMinimumHeight(40);
    cancelButton->setFont(bigFont);
    cancelButton->setMinimumHeight(40);
    addButton->setStyleSheet("QPushButton { background-color: #43a047; color: white; font-weight: bold; } QPushButton:hover { background-color: #388e3c; }");
    cancelButton->setStyleSheet("QPushButton { background-color: #d32f2f; color: white; font-weight: bold; } QPushButton:hover { background-color: #b71c1c; }");
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(cancelButton);
    fieldsLayout->addLayout(buttonLayout);

    mainRow->addLayout(fieldsLayout);

    // Блок справа для обложки
    QVBoxLayout *coverLayout = new QVBoxLayout;
    coverLabel = new QLabel("Нет обложки");
    coverLabel->setMinimumHeight(270);
    coverLabel->setMinimumWidth(180);
    coverLabel->setFixedSize(180, 270);
    coverLabel->setAlignment(Qt::AlignCenter);
    coverLabel->setStyleSheet("border: 2px solid #03a9f4; background: #eee;");
    QFont coverLabelFont = bigFont;
    coverLabelFont.setPointSize(12);
    coverLabel->setFont(coverLabelFont);
    chooseCoverButton = new QPushButton("Выбрать обложку");
    chooseCoverButton->setFont(bigFont);
    chooseCoverButton->setMinimumHeight(40);
    chooseCoverButton->setStyleSheet("QPushButton { border: 2px solid #03a9f4; background: white; color: #03a9f4; font-weight: bold; } QPushButton:hover { background: #e0f7fa; }");
    // Делаем QLabel обложки кликабельным
    coverLabel->setCursor(Qt::PointingHandCursor);
    coverLabel->installEventFilter(this);
    // Центрируем кнопку 'Выбрать обложку' под обложкой
    QHBoxLayout *chooseCoverButtonLayout = new QHBoxLayout;
    chooseCoverButtonLayout->addStretch();
    chooseCoverButtonLayout->addWidget(chooseCoverButton);
    chooseCoverButtonLayout->addStretch();
    // Центрируем обложку и кнопку по вертикали
    coverLayout->addStretch();
    coverLayout->addWidget(coverLabel, 0, Qt::AlignHCenter);
    coverLayout->addLayout(chooseCoverButtonLayout);
    coverLayout->addStretch();
    mainRow->addLayout(coverLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(mainRow);

    currentRating = 0;
    updateRatingLabels(0);

    connect(addButton, &QPushButton::clicked, this, &AddMovieDialog::onAddClicked);
    connect(cancelButton, &QPushButton::clicked, this, &AddMovieDialog::onCancelClicked);
    connect(chooseCoverButton, &QPushButton::clicked, this, &AddMovieDialog::onChooseCoverClicked);

    setMinimumSize(700, 400);
    this->setStyleSheet("background-color: orange;");
}

void AddMovieDialog::onAddClicked()
{
    if (titleEdit->text().trimmed().isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Ошибка");
        msgBox.setText("Введите название фильма.");
        msgBox.setStyleSheet("QLabel, QMessageBox QLabel { color: #03a9f4; font-size: 18px; background: #e0f7fa; } QMessageBox { background: #e0f7fa; } QPushButton { background: #e0f7fa; color: #03a9f4; font-weight: bold; min-width: 80px; border: none; } QPushButton:hover { background: #b3e5fc; }");
        QPixmap blueIcon(64, 64);
        blueIcon.fill(QColor("#e0f7fa"));
        QPainter painter(&blueIcon);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(QColor("#03a9f4"), 6));
        painter.setBrush(QBrush(QColor("#e0f7fa")));
        painter.drawEllipse(8, 8, 48, 48);
        painter.setPen(QPen(QColor("#03a9f4"), 4));
        painter.drawLine(32, 20, 32, 40);
        painter.drawEllipse(30, 46, 4, 4);
        msgBox.setIconPixmap(blueIcon);
        msgBox.exec();
        return;
    }
    bool ok = false;
    int year = yearEdit->text().toInt(&ok);
    int currentYear = QDate::currentDate().year();
    if (!yearEdit->text().isEmpty() && (!ok || year > currentYear)) {
        QMessageBox::warning(this, "Ошибка", QString("Год выпуска не может быть больше %1").arg(currentYear));
        return;
    }
    QString reviewText;
    if (reviewEdit) {
        reviewText = reviewEdit->toPlainText();
    }
    emit movieAdded(titleEdit->text(), yearEdit->text(), genreEdit->text(), currentRating, coverPath, reviewText);
    accept();
}

void AddMovieDialog::onCancelClicked()
{
    reject();
}

void AddMovieDialog::onChooseCoverClicked()
{
    QString file = QFileDialog::getOpenFileName(this, "Выберите обложку", QString(), "Изображения (*.png *.jpg *.jpeg *.bmp)");
    if (!file.isEmpty()) {
        coverPixmap.load(file);
        coverLabel->setPixmap(coverPixmap.scaled(180, 270, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        coverPath = file;
    }
}

bool AddMovieDialog::eventFilter(QObject *obj, QEvent *event)
{
    for (int i = 0; i < 10; ++i) {
        if (obj == ratingLabels[i]) {
            if (event->type() == QEvent::MouseButtonPress) {
                currentRating = i + 1;
                updateRatingLabels(currentRating);
            } else if (event->type() == QEvent::Enter) {
                updateRatingLabels(i + 1);
            } else if (event->type() == QEvent::Leave) {
                updateRatingLabels(currentRating);
            }
        }
    }
    if (obj == coverLabel && event->type() == QEvent::MouseButtonPress) {
        onChooseCoverClicked();
        return true;
    }
    return QDialog::eventFilter(obj, event);
}

void AddMovieDialog::updateRatingLabels(int rating)
{
    for (int i = 0; i < 10; ++i) {
        QString color = "#888"; // по умолчанию серый
        if (rating > 0 && i + 1 == rating) {
            if (i + 1 >= 1 && i + 1 <= 4) color = "#d32f2f"; // 1-4 очень красные
            else if (i + 1 == 10) color = "#388e3c"; // очень зеленый
            else if (i + 1 == 5 || i + 1 == 6) color = "#888"; // серый
            else if (i + 1 < 10) color = "#4caf50"; // зеленый
        }
        QString style = QString("color: %1;").arg(color);
        if (rating > 0 && i + 1 == rating) style += " font-weight: bold; text-decoration: underline;";
        ratingLabels[i]->setStyleSheet(style);
    }
}

// Для корректной работы звезд добавьте папку icons в ресурсный файл .qrc с иконками star_empty.png и star_filled.png
// Пример структуры:
// <RCC>
//  <qresource prefix="/icons">
//   <file>icons/star_empty.png</file>
//   <file>icons/star_filled.png</file>
//  </qresource>
// </RCC> 