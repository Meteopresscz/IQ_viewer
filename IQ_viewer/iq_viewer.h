#ifndef IQ_VIEWER_H
#define IQ_VIEWER_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class IQ_viewer;
}
QT_END_NAMESPACE

class IQ_viewer : public QMainWindow
{
    Q_OBJECT

public:
    IQ_viewer(QWidget *parent = nullptr);
    ~IQ_viewer();

private:
    Ui::IQ_viewer *ui;
};
#endif // IQ_VIEWER_H
