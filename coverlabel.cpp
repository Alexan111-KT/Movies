#include "coverlabel.h"

CoverLabel::CoverLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent) {}

void CoverLabel::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QLabel::mousePressEvent(event);
} 