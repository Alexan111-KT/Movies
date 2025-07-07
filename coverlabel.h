#ifndef COVERLABEL_H
#define COVERLABEL_H

#include <QLabel>
#include <QWidget>
#include <QMouseEvent>

class CoverLabel : public QLabel {
    Q_OBJECT
public:
    explicit CoverLabel(const QString &text = "", QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // COVERLABEL_H 