#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>

class ClickableLabel : public QLabel
{
		Q_OBJECT
	public:
		explicit ClickableLabel(QWidget *parent = 0);
		ClickableLabel(const QString& text, QWidget *parent = 0);

	signals:
		void clicked();

	protected:
		void mousePressEvent(QMouseEvent*);
};

#endif // CLICKABLELABEL_H
