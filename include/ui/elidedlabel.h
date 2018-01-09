#pragma once

#include <QLabel>

class ElidedLabel : public QLabel
{
  Q_OBJECT
 public:
     ElidedLabel(QWidget* parent=0, Qt::WindowFlags f=0);
     ElidedLabel(const QString& txt, QWidget* parent=0, Qt::WindowFlags f=0);
     ElidedLabel(const QString& txt,
         Qt::TextElideMode elideMode=Qt::ElideRight,
         QWidget* parent=0,
         Qt::WindowFlags f=0
     );
     // Set the elide mode used for displaying text.
     void setElideMode(Qt::TextElideMode elideMode) {
         elideMode_ = elideMode;
         updateGeometry();
     }
     // Get the elide mode currently used to display text.
     Qt::TextElideMode elideMode() const { return elideMode_; }
     // QLabel overrides
     void setText(const QString &);

 protected: // QLabel overrides
     void paintEvent(QPaintEvent*);
     void resizeEvent(QResizeEvent*);
     // Cache the elided text so as to not recompute it every paint event
     void cacheElidedText(int w);

 private:
     Qt::TextElideMode elideMode_;
     QString cachedElidedText;
};
