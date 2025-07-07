#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "addmoviedialog.h"
#include <QFile>
#include <QTextStream>
#include <QStyledItemDelegate>
#include <QIntValidator>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QFormLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QMessageBox>
#include <QEvent>
#include <QSizePolicy>
#include <QLabel>
#include <QFont>
#include <QRegExpValidator>
#include <QRegExp>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QPixmap>
#include <QPalette>
#include <QBrush>
#include <QApplication>
#include <QHeaderView>
#include <QComboBox>
#include <QDir>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QTimer>
#include <QSettings>
#include <QDate>

// Делегат для числового ввода в колонке 'Рейтинг'
class RatingDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        if (index.column() == 3) {
            QLineEdit *editor = new QLineEdit(parent);
            editor->setValidator(new QIntValidator(1, 10, editor));
            return editor;
        }
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
};

// Делегат для жанра (только буквы и пробелы)
class GenreDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        if (index.column() == 2) {
            QLineEdit *editor = new QLineEdit(parent);
            editor->setValidator(new QRegExpValidator(QRegExp("[A-Za-zА-Яа-яЁё\\s]+"), editor));
            return editor;
        }
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
};

// Делегат для года (только числа)
class YearDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        if (index.column() == 1) {
            QLineEdit *editor = new QLineEdit(parent);
            editor->setValidator(new QIntValidator(0, 9999, editor));
            return editor;
        }
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
};

// Кастомный QTableWidgetItem для числовых столбцов
class NumericTableWidgetItem : public QTableWidgetItem {
public:
    using QTableWidgetItem::QTableWidgetItem;
    bool operator<(const QTableWidgetItem &other) const override {
        bool ok1, ok2;
        int v1 = text().toInt(&ok1);
        int v2 = other.text().toInt(&ok2);
        if (ok1 && ok2)
            return v1 < v2;
        return QTableWidgetItem::operator<(other);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Кино");
    loadLastFilePath();
    if (lastImportedFilePath.isEmpty()) {
        lastImportedFilePath = QDir::currentPath() + "/film.csv";
    }
    QWidget *central = new QWidget(this);
    central->setStyleSheet("background-color: #e0f7fa;"); // Light blue background
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    // Слева — обложка (поднимаю выше)
    QVBoxLayout *leftLayout = new QVBoxLayout;
    coverLabel = new CoverLabel("Нет обложки");
    coverLabel->setFixedSize(300, 450);
    coverLabel->setAlignment(Qt::AlignCenter);
    coverLabel->setStyleSheet("border: 2px solid orange; background: #eee; border-radius: 8px;");
    leftLayout->addWidget(coverLabel);
    leftLayout->addStretch();
    mainLayout->addLayout(leftLayout);
    // Центр — таблица и панель редактирования
    QVBoxLayout *centerLayout = new QVBoxLayout;
    mainLayout->addLayout(centerLayout);
    // --- Панель поиска ---
    QHBoxLayout *searchLayout = new QHBoxLayout;
    QLabel *searchLabel = new QLabel("Поиск:");
    QFont searchFont = searchLabel->font();
    searchFont.setPointSize(22);
    searchLabel->setFont(searchFont);
    QComboBox *searchColumnCombo = new QComboBox;
    searchColumnCombo->addItem("Название", 0);
    searchColumnCombo->addItem("Год выпуска", 1);
    searchColumnCombo->addItem("Жанр", 2);
    searchColumnCombo->addItem("Рейтинг", 3);
    searchColumnCombo->setFont(searchFont);
    searchColumnCombo->setMinimumHeight(40);
    QLineEdit *searchEdit = new QLineEdit;
    searchEdit->setFont(searchFont);
    searchEdit->setMinimumHeight(40);
    searchEdit->setStyleSheet("QLineEdit { border: 2px solid orange; border-radius: 4px; background: white; }");
    QPushButton *searchButton = new QPushButton("Поиск");
    searchButton->setFont(searchFont);
    searchButton->setMinimumHeight(40);
    searchButton->setStyleSheet("QPushButton { border: 2px solid orange; border-radius: 4px; background: #fff3e0; color: #333; font-weight: bold; } QPushButton:hover { background: #ffe0b2; }");
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchColumnCombo);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);
    centerLayout->insertLayout(0, searchLayout);
    tableWidget = new QTableWidget(this);
    tableWidget->setStyleSheet(
        "QTableWidget { border: 2px solid orange; border-radius: 8px; }"
        "QTableWidget::item { border: 2px solid orange; }"
        "QTableWidget::item:selected { background: #fff3e0; color: #222; border: 2px solid orange; }"
        "QTableWidget::item:focus { background: #fffbe6; border: 2px solid orange; }"
        "QTableCornerButton::section { background: #ffe0b2; border: 2px solid orange; }"
    );
    centerLayout->addWidget(tableWidget);
    // Панель редактирования в рамке
    QGroupBox *editGroup = new QGroupBox("Редактирование фильма");
    QFont groupTitleFont = editGroup->font();
    groupTitleFont.setPointSize(22);
    editGroup->setFont(groupTitleFont);
    editGroup->setStyleSheet("QGroupBox { border: 2px solid orange; border-radius: 8px; margin-top: 1ex; } QGroupBox::title { color: orange; }");
    QVBoxLayout *editGroupLayout = new QVBoxLayout(editGroup);
    editGroupLayout->setContentsMargins(0, 30, 0, 0);
    QFormLayout *editLayout = new QFormLayout;
    editLayout->setContentsMargins(30, 0, 0, 0);
    editGroupLayout->addLayout(editLayout);
    QFont editFont;
    editFont.setPointSize(18);
    editTitle = new QLineEdit; editTitle->setFont(editFont);
    editTitle->setStyleSheet("QLineEdit { border: 2px solid orange; background: white; }");
    QLabel *titleLabel = new QLabel("Название:"); titleLabel->setFont(editFont);
    editLayout->addRow(titleLabel, editTitle);
    editYear = new QLineEdit; editYear->setFont(editFont);
    editYear->setStyleSheet("QLineEdit { border: 2px solid orange; background: white; }");
    QLabel *yearLabel = new QLabel("Год выпуска:"); yearLabel->setFont(editFont);
    editYear->setValidator(new QIntValidator(0, 9999, this));
    editLayout->addRow(yearLabel, editYear);
    editGenre = new QLineEdit; editGenre->setFont(editFont);
    editGenre->setStyleSheet("QLineEdit { border: 2px solid orange; background: white; }");
    QLabel *genreLabel = new QLabel("Жанр:"); genreLabel->setFont(editFont);
    QRegExp genreRx("[A-Za-zА-Яа-яЁё\\s]+$");
    editGenre->setValidator(new QRegExpValidator(genreRx, this));
    editLayout->addRow(genreLabel, editGenre);
    QLabel *ratingLabel = new QLabel("Рейтинг:"); ratingLabel->setFont(editFont);
    QHBoxLayout *ratingNumbersLayout = new QHBoxLayout;
    for (int i = 1; i <= 10; ++i) {
        QPushButton *numBtn = new QPushButton(QString::number(i));
        numBtn->setCheckable(true);
        numBtn->setMinimumWidth(28);
        numBtn->setMaximumWidth(32);
        numBtn->setMinimumHeight(28);
        QFont numFont; numFont.setPointSize(12);
        numBtn->setFont(numFont);
        QString color;
        if (i >= 1 && i <= 4) color = "#d32f2f";
        else if (i == 5 || i == 6) color = "#888";
        else color = "#388e3c";
        numBtn->setStyleSheet(
            QString(
                "QPushButton { border: none; background: none; color: %1; }"
                "QPushButton:hover { color: %1; font-weight: bold; text-decoration: underline; background: none; }"
                "QPushButton:checked { color: %1; font-weight: bold; text-decoration: underline; background: none; }"
            ).arg(color)
        );
        ratingNumbersLayout->addWidget(numBtn);
        this->ratingButtons.append(numBtn);
        QObject::connect(numBtn, &QPushButton::clicked, this, [this, i]() {
            this->editRatingValue = i;
            for (int j = 0; j < this->ratingButtons.size(); ++j) {
                QPushButton* btn = this->ratingButtons[j];
                bool checked = (j == i-1);
                btn->setChecked(checked);
                QString color;
                int idx = j+1;
                if (idx >= 1 && idx <= 4) color = "#d32f2f";
                else if (idx == 5 || idx == 6) color = "#888";
                else color = "#388e3c";
                if (checked) {
                    btn->setStyleSheet(
                        QString(
                            "QPushButton { border: none; background: none; color: %1; font-weight: bold; text-decoration: underline; }"
                            "QPushButton:hover { color: %1; font-weight: bold; text-decoration: underline; background: none; }"
                            "QPushButton:checked { color: %1; font-weight: bold; text-decoration: underline; background: none; }"
                        ).arg(color)
                    );
                } else {
                    btn->setStyleSheet(
                        QString(
                            "QPushButton { border: none; background: none; color: %1; }"
                            "QPushButton:hover { color: %1; font-weight: bold; text-decoration: underline; background: none; }"
                            "QPushButton:checked { color: %1; background: none; }"
                        ).arg(color)
                    );
                }
            }
        });
    }
    QWidget *ratingNumbersWidget = new QWidget;
    ratingNumbersWidget->setLayout(ratingNumbersLayout);
    editLayout->addRow(ratingLabel, ratingNumbersWidget);
    editReview = new QTextEdit; editReview->setFont(editFont);
    editReview->setStyleSheet("QTextEdit { border: 2px solid orange; background: white; }");
    QLabel *reviewLabel = new QLabel("Отзыв:"); reviewLabel->setFont(editFont);
    editReview->setPlaceholderText("Общее впечатление"); editReview->setFixedHeight(150); editLayout->addRow(reviewLabel, editReview);
    saveEditButton = new QPushButton("Сохранить изменения"); 
    saveEditButton->setStyleSheet("QPushButton { background: #43a047; color: #fff; font-weight: bold; }");
    editLayout->addRow(saveEditButton);
    centerLayout->addWidget(editGroup);
    // Кнопки: 'Добавить фильм' справа от таблицы, 'Удалить фильм' ниже неё
    QVBoxLayout *rightButtonLayout = new QVBoxLayout;
    QPushButton *addButton = new QPushButton("Добавить фильм");
    QPushButton *removeButton = new QPushButton("Удалить фильм");
    rightButtonLayout->addWidget(addButton, 0, Qt::AlignTop);
    rightButtonLayout->addSpacing(20);
    rightButtonLayout->addWidget(removeButton, 0, Qt::AlignTop);
    rightButtonLayout->addStretch();
    // --- Кнопки для работы с файлами ---
    QPushButton *findFileButton = new QPushButton("Найти файл");
    QPushButton *exportButton = new QPushButton("Экспорт");
    QPushButton *importButton = new QPushButton("Импорт");
    findFileButton->setFont(searchFont);
    exportButton->setFont(searchFont);
    importButton->setFont(searchFont);
    findFileButton->setMinimumHeight(40);
    exportButton->setMinimumHeight(40);
    importButton->setMinimumHeight(40);
    findFileButton->setStyleSheet("QPushButton { border: 2px solid orange; border-radius: 4px; background: #fff3e0; color: #333; font-weight: bold; } QPushButton:hover { background: #ffe0b2; }");
    exportButton->setStyleSheet("QPushButton { border: 2px solid orange; border-radius: 4px; background: #fff3e0; color: #333; font-weight: bold; } QPushButton:hover { background: #ffe0b2; }");
    importButton->setStyleSheet("QPushButton { border: 2px solid orange; border-radius: 4px; background: #fff3e0; color: #333; font-weight: bold; } QPushButton:hover { background: #ffe0b2; }");
    rightButtonLayout->addSpacing(30);
    rightButtonLayout->addWidget(findFileButton);
    rightButtonLayout->addWidget(exportButton);
    rightButtonLayout->addWidget(importButton);
    mainLayout->addLayout(rightButtonLayout);
    setCentralWidget(central);
    tableWidget->setColumnCount(6);
    QStringList headers;
    headers << "Название" << "Год выпуска" << "Жанр" << "Рейтинг" << "Обложка" << "Описание";
    tableWidget->setHorizontalHeaderLabels(headers);
    tableWidget->setColumnHidden(4, true);
    tableWidget->setColumnHidden(5, true);
    tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->setItemDelegate(new RatingDelegate(this));
    tableWidget->setItemDelegateForColumn(2, new GenreDelegate(this));
    tableWidget->setItemDelegateForColumn(1, new YearDelegate(this));
    connect(tableWidget, &QTableWidget::itemChanged, this, [this](QTableWidgetItem* item) {
        if (item->column() == 1) {
            bool ok;
            int val = item->text().toInt(&ok);
            int currentYear = QDate::currentDate().year();
            if (!ok || val > currentYear) {
                if (!item->text().isEmpty()) {
                    item->setText("");
                    QMessageBox::warning(this, "Ошибка", QString("Год выпуска не может быть больше %1").arg(currentYear));
                }
            }
        }
    });
    loadMoviesFromFile();
    connect(tableWidget, &QTableWidget::itemSelectionChanged, this, &MainWindow::onTableRowSelected);
    connect(saveEditButton, &QPushButton::clicked, this, &MainWindow::onSaveEditClicked);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::on_addButton_clicked);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::on_removeButton_clicked);
    // --- Сигналы ---
    connect(coverLabel, &CoverLabel::clicked, this, [this]() {
        if (tableWidget->currentRow() < 0) {
            QMessageBox::warning(this, "Ошибка", "Сначала выберите фильм в таблице!");
            return;
        }
        QString file = QFileDialog::getOpenFileName(this, "Выберите обложку", QString(), "Изображения (*.png *.jpg *.jpeg *.bmp)");
        if (!file.isEmpty()) {
            QPixmap pix(file);
            coverLabel->setPixmap(pix.scaled(300, 450, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            coverLabel->setText("");
            int row = tableWidget->currentRow();
            if (row >= 0) {
                if (!tableWidget->item(row, 4))
                    tableWidget->setItem(row, 4, new QTableWidgetItem(file));
                else
                    tableWidget->item(row, 4)->setText(file);
            }
        }
    });
    // Сразу делаем поля и обложку неактивными
    editTitle->setEnabled(false);
    editYear->setEnabled(false);
    editGenre->setEnabled(false);
    editReview->setEnabled(false);
    saveEditButton->setEnabled(false);
    coverLabel->setEnabled(false);
    for (QPushButton* btn : this->ratingButtons) btn->setEnabled(false);
    for (QPushButton* btn : this->ratingButtons) btn->setStyleSheet(
        "QPushButton { border: none; background: none; color: #bbb; }"
        "QPushButton:hover { color: #bbb; font-weight: bold; text-decoration: underline; background: none; }"
        "QPushButton:checked { color: #bbb; font-weight: bold; text-decoration: underline; background: none; }"
    );
    // Деревянный фон для всего приложения
    QPixmap woodBg("wood.jpg");
    if (woodBg.isNull()) woodBg.load("wood.png");
    if (!woodBg.isNull()) {
        QPalette pal = qApp->palette();
        pal.setBrush(QPalette::Window, QBrush(woodBg));
        qApp->setPalette(pal);
    }
    QFont bigFont;
    bigFont.setPointSize(18);
    this->setFont(bigFont);
    tableWidget->setFont(bigFont);
    QFont headerFont = tableWidget->horizontalHeader()->font();
    headerFont.setPointSize(18);
    headerFont.setBold(true);
    tableWidget->horizontalHeader()->setFont(headerFont);
    tableWidget->verticalHeader()->setFont(headerFont);
    tableWidget->setMinimumHeight(400);
    tableWidget->setMinimumWidth(700);
    addButton->setFont(bigFont);
    removeButton->setFont(bigFont);
    QFont groupBoxFont = editGroup->font();
    groupBoxFont.setBold(false);
    editGroup->setFont(groupBoxFont);
    QFont normalFont = this->font();
    editTitle->setFont(normalFont);
    editYear->setFont(normalFont);
    editGenre->setFont(normalFont);
    editReview->setFont(normalFont);
    saveEditButton->setFont(normalFont);
    saveEditButton->setMinimumHeight(40);
    for (QPushButton* btn : this->ratingButtons) {
        btn->setFont(bigFont);
        btn->setMinimumHeight(36);
        btn->setMinimumWidth(36);
    }
    this->resize(1800, 1200);
    tableWidget->setColumnWidth(0, 300);
    tableWidget->setColumnWidth(2, 200);
    tableWidget->setColumnWidth(1, 250);
    // Новый код для sectionClicked:
    static Qt::SortOrder lastSortOrder = Qt::AscendingOrder;
    static int lastSortColumn = -1;
    connect(tableWidget->horizontalHeader(), &QHeaderView::sectionClicked, this, [=](int logicalIndex) {
        Qt::SortOrder order = Qt::AscendingOrder;
        if (lastSortColumn == logicalIndex) {
            order = (lastSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
        }
        tableWidget->setSortingEnabled(true);
        tableWidget->sortItems(logicalIndex, order);
        lastSortOrder = order;
        lastSortColumn = logicalIndex;
    });
    // --- Реализация поиска ---
    connect(searchButton, &QPushButton::clicked, this, [=]() {
        int col = searchColumnCombo->currentData().toInt();
        QString text = searchEdit->text().trimmed();
        for (int row = 0; row < tableWidget->rowCount(); ++row) {
            QTableWidgetItem *item = tableWidget->item(row, col);
            bool match = item && item->text().contains(text, Qt::CaseInsensitive);
            tableWidget->setRowHidden(row, !match && !text.isEmpty());
        }
    });
    // Сброс фильтра при очистке поля
    connect(searchEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        if (text.isEmpty()) {
            for (int row = 0; row < tableWidget->rowCount(); ++row)
                tableWidget->setRowHidden(row, false);
        }
    });
    addButton->setStyleSheet("QPushButton { background-color: #43a047; color: white; font-weight: bold; } QPushButton:hover { background-color: #388e3c; }");
    removeButton->setStyleSheet("QPushButton { background-color: #d32f2f; color: white; font-weight: bold; } QPushButton:hover { background-color: #b71c1c; }");
    // --- Логика кнопок ---
    connect(findFileButton, &QPushButton::clicked, this, [this]() {
        QInputDialog dialog(this);
        dialog.setWindowTitle("Путь к файлу");
        dialog.setLabelText("Путь к файлу (можно скопировать):");
        dialog.setTextValue(this->lastImportedFilePath);
        dialog.setOkButtonText("OK");
        dialog.setInputMode(QInputDialog::TextInput);
        dialog.setFixedSize(500, 120);
        QDialogButtonBox *box = dialog.findChild<QDialogButtonBox*>();
        if (box) {
            QPushButton *cancelBtn = box->button(QDialogButtonBox::Cancel);
            if (cancelBtn) cancelBtn->hide();
        }
        dialog.exec();
    });
    connect(exportButton, &QPushButton::clicked, this, [this]() {
        QString file = QFileDialog::getSaveFileName(this, "Экспортировать данные", QFileInfo(this->lastImportedFilePath).fileName(), "CSV файлы (*.csv)");
        if (!file.isEmpty()) {
            // Экспортируем данные напрямую в новый файл
            if (exportMoviesToFile(file)) {
                QMessageBox::information(this, "Экспорт", "Файл успешно экспортирован!");
            } else {
                QMessageBox::warning(this, "Ошибка", "Не удалось экспортировать файл.");
            }
        }
    });
    connect(importButton, &QPushButton::clicked, this, [this]() {
        // Сохраняем изменения в текущий файл перед импортом нового
        this->saveMoviesToFile();
        QString file = QFileDialog::getOpenFileName(this, "Импортировать данные", QString(), "CSV файлы (*.csv)");
        if (!file.isEmpty()) {
            // Обновляем путь к текущему файлу
            this->lastImportedFilePath = file;
            // Загружаем данные из нового файла
            loadMoviesFromFile();
            // Обновляем отображение обложки и описания для выбранного фильма
            if (tableWidget->currentRow() >= 0) {
                onTableRowSelected();
            }
            QMessageBox::information(this, "Импорт", "Данные успешно импортированы!");
        }
    });
    tableWidget->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { background: #f5e6d3; color: #000; font-weight: bold; border: 1px solid #bfa77a; padding: 4px; }"
        "QHeaderView::section:hover, QHeaderView::section:focus, QHeaderView::section:checked { background: #fff3e0; color: orange; border: 2px solid orange; }"
    );
    tableWidget->verticalHeader()->setStyleSheet(
        "QHeaderView::section { background: #f5e6d3; color: #222; font-weight: bold; border: 1px solid #bfa77a; padding: 4px; }"
        "QHeaderView::section:hover, QHeaderView::section:focus, QHeaderView::section:checked { background: #fff3e0; color: orange; border: 2px solid orange; }"
    );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_addButton_clicked()
{
    AddMovieDialog dialog(this);
    connect(&dialog, &AddMovieDialog::movieAdded, this, [this](const QString &title, const QString &year, const QString &genre, int rating, const QString &coverPath, const QString &review) {
        this->onMovieAdded(title, year, genre, rating, coverPath, review);
    });
    dialog.exec();
}

void MainWindow::onMovieAdded(const QString &title, const QString &year, const QString &genre, int rating, const QString &coverPath, const QString &review)
{
    int row = tableWidget->rowCount();
    tableWidget->insertRow(row);
    tableWidget->setItem(row, 0, new QTableWidgetItem(title));
    tableWidget->setItem(row, 1, new NumericTableWidgetItem(year));
    tableWidget->setItem(row, 2, new QTableWidgetItem(genre));
    tableWidget->setItem(row, 3, new NumericTableWidgetItem(rating > 0 ? QString::number(rating) : ""));
    tableWidget->setItem(row, 4, new QTableWidgetItem(coverPath));
    tableWidget->setItem(row, 5, new QTableWidgetItem(review));
    ensureAllItems();
    saveMoviesToFile();
}

void MainWindow::on_removeButton_clicked()
{
    int row = tableWidget->currentRow();
    if (row >= 0) {
        tableWidget->removeRow(row);
        saveMoviesToFile();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveMoviesToFile();
    saveLastFilePath();
    event->accept();
}

void MainWindow::saveLastFilePath() {
    QSettings settings("MyCompany", "MovieManagerApp");
    settings.setValue("lastImportedFilePath", lastImportedFilePath);
}

void MainWindow::loadLastFilePath() {
    QSettings settings("MyCompany", "MovieManagerApp");
    lastImportedFilePath = settings.value("lastImportedFilePath", "").toString();
}

void MainWindow::saveMoviesToFile()
{
    bool fileExists = QFile::exists(lastImportedFilePath);
    QFile file(lastImportedFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        // Если файл только что создан, пишем заголовки
        if (!fileExists) {
            QStringList headers;
            headers << "Название" << "Год выпуска" << "Жанр" << "Рейтинг" << "Обложка" << "Описание";
            out << headers.join(",") << "\n";
        }
        for (int row = 0; row < tableWidget->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < tableWidget->columnCount(); ++col) {
                QTableWidgetItem *item = tableWidget->item(row, col);
                QString text = item ? item->text().replace('"', """") : "";
                rowData << '"' + text + '"';
            }
            out << rowData.join(",") << "\n";
        }
        file.close();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи: " + lastImportedFilePath);
    }
}

void MainWindow::loadMoviesFromFile()
{
    QFile file(lastImportedFilePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        tableWidget->setRowCount(0);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.isEmpty()) continue;
            QStringList rowData;
            bool inQuotes = false;
            QString value;
            for (int i = 0; i < line.size(); ++i) {
                QChar c = line[i];
                if (c == '"') {
                    if (inQuotes && i+1 < line.size() && line[i+1] == '"') {
                        value += '"';
                        ++i;
                    } else {
                        inQuotes = !inQuotes;
                    }
                } else if (c == ',' && !inQuotes) {
                    rowData << value;
                    value.clear();
                } else {
                    value += c;
                }
            }
            rowData << value;
            int row = tableWidget->rowCount();
            tableWidget->insertRow(row);
            for (int col = 0; col < rowData.size() && col < tableWidget->columnCount(); ++col) {
                if (col == 1 || col == 3)
                    tableWidget->setItem(row, col, new NumericTableWidgetItem(rowData[col]));
                else
                    tableWidget->setItem(row, col, new QTableWidgetItem(rowData[col]));
            }
        }
        file.close();
        ensureAllItems();
        
        // Обновляем отображение обложки и описания, если есть выбранная строка
        if (tableWidget->currentRow() >= 0) {
            onTableRowSelected();
        }
    }
}

void MainWindow::onTableRowSelected()
{
    int row = tableWidget->currentRow();
    editingRow = row;
    bool hasSelection = row >= 0;
    editTitle->setEnabled(hasSelection);
    editYear->setEnabled(hasSelection);
    editGenre->setEnabled(hasSelection);
    editReview->setEnabled(hasSelection);
    saveEditButton->setEnabled(hasSelection);
    coverLabel->setEnabled(hasSelection);
    if (hasSelection) {
        saveEditButton->setStyleSheet("QPushButton { background: #43a047; color: #fff; font-weight: bold; }");
    } else {
        saveEditButton->setStyleSheet("QPushButton { background: #bbb; color: #fff; font-weight: bold; }");
    }
    for (int i = 0; i < this->ratingButtons.size(); ++i) {
        this->ratingButtons[i]->setEnabled(hasSelection);
        QString colorChecked, colorNormal = "#222";
        if (i+1 >= 1 && i+1 <= 4) colorChecked = "#d32f2f";
        else if (i+1 == 5 || i+1 == 6) colorChecked = "#888";
        else colorChecked = "#388e3c";
        if (i == row) {
            this->ratingButtons[i]->setStyleSheet(
                QString(
                    "QPushButton { border: none; background: none; color: %1; }"
                    "QPushButton:hover { color: %1; font-weight: bold; text-decoration: underline; background: none; }"
                    "QPushButton:checked { color: %1; font-weight: bold; text-decoration: underline; background: none; }"
                ).arg(colorChecked)
            );
        } else {
            this->ratingButtons[i]->setStyleSheet(
                QString(
                    "QPushButton { border: none; background: none; color: %1; }"
                    "QPushButton:hover { color: %1; background: none; }"
                    "QPushButton:checked { color: %1; background: none; }"
                ).arg(colorNormal)
            );
        }
    }
    if (!hasSelection) {
        editTitle->clear();
        editYear->clear();
        editGenre->clear();
        editReview->clear();
        coverLabel->setText("Нет обложки");
        coverLabel->setPixmap(QPixmap());
        for (QPushButton* btn : this->ratingButtons) btn->setChecked(false);
        return;
    }
    editTitle->setText(tableWidget->item(row, 0) ? tableWidget->item(row, 0)->text() : "");
    editYear->setText(tableWidget->item(row, 1) ? tableWidget->item(row, 1)->text() : "");
    editGenre->setText(tableWidget->item(row, 2) ? tableWidget->item(row, 2)->text() : "");
    QString coverPath = tableWidget->item(row, 4) ? tableWidget->item(row, 4)->text() : "";
    if (!coverPath.isEmpty() && QFile::exists(coverPath)) {
        QPixmap pix(coverPath);
        coverLabel->setPixmap(pix.scaled(300, 450, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        coverLabel->setText("");
    } else {
        coverLabel->setText("Нет обложки");
        coverLabel->setPixmap(QPixmap());
    }
    // Подсвечиваем рейтинг
    int rating = tableWidget->item(row, 3) ? tableWidget->item(row, 3)->text().toInt() : 0;
    this->editRatingValue = rating;
    for (int i = 0; i < this->ratingButtons.size(); ++i) {
        QPushButton* btn = this->ratingButtons[i];
        bool checked = (i == rating-1);
        btn->setChecked(checked);
        QString color;
        if (i+1 >= 1 && i+1 <= 4) color = "#d32f2f";
        else if (i+1 == 5 || i+1 == 6) color = "#888";
        else color = "#388e3c";
        if (checked) {
            btn->setStyleSheet(
                QString(
                    "QPushButton { border: none; background: none; color: %1; font-weight: bold; text-decoration: underline; }"
                    "QPushButton:hover { color: %1; font-weight: bold; text-decoration: underline; background: none; }"
                    "QPushButton:checked { color: %1; font-weight: bold; text-decoration: underline; background: none; }"
                ).arg(color)
            );
        } else {
            btn->setStyleSheet(
                QString(
                    "QPushButton { border: none; background: none; color: #222; }"
                    "QPushButton:hover { color: %1; font-weight: bold; text-decoration: underline; background: none; }"
                    "QPushButton:checked { color: #222; background: none; }"
                ).arg(color)
            );
        }
    }
    // Загружаем описание
    editReview->setText(tableWidget->item(row, 5) ? tableWidget->item(row, 5)->text() : "");
}

void MainWindow::onSaveEditClicked()
{
    int row = tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Ошибка", "Сначала выберите фильм в таблице!");
        return;
    }
    bool ok = false;
    int year = editYear->text().toInt(&ok);
    int currentYear = QDate::currentDate().year();
    if (!editYear->text().isEmpty() && (!ok || year > currentYear)) {
        QMessageBox::warning(this, "Ошибка", QString("Год выпуска не может быть больше %1").arg(currentYear));
        return;
    }
    tableWidget->setItem(row, 0, new QTableWidgetItem(editTitle->text()));
    tableWidget->setItem(row, 1, new QTableWidgetItem(editYear->text()));
    tableWidget->setItem(row, 2, new QTableWidgetItem(editGenre->text()));
    // Сохраняем рейтинг
    tableWidget->setItem(row, 3, new QTableWidgetItem(editRatingValue > 0 ? QString::number(editRatingValue) : ""));
    // Сохраняем описание
    tableWidget->setItem(row, 5, new QTableWidgetItem(editReview->toPlainText()));
    saveMoviesToFile();
}

void MainWindow::ensureAllItems()
{
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        for (int col = 0; col < tableWidget->columnCount(); ++col) {
            if (!tableWidget->item(row, col)) {
                tableWidget->setItem(row, col, new QTableWidgetItem(""));
            }
        }
    }
}

bool MainWindow::exportMoviesToFile(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (int row = 0; row < tableWidget->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < tableWidget->columnCount(); ++col) {
                QTableWidgetItem *item = tableWidget->item(row, col);
                QString text = item ? item->text().replace('"', """") : "";
                rowData << '"' + text + '"';
            }
            out << rowData.join(",") << "\n";
        }
        file.close();
        return true;
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи: " + filePath);
        return false;
    }
}

