#ifndef MENUE_H
#define MENUE_H

#include <QMenu>

#include <QObject>

class Menu : public QMenu
{
public:
    Menu(QWidget *parent);

   // resizeEvent(self, event);


    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event);




};

#endif // MENUE_H
