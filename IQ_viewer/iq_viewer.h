#ifndef IQ_VIEWER_H
#define IQ_VIEWER_H

#include <QMainWindow>
#include <vector> // Use std::vector instead of QVector
#include <complex>
#include "kiss_fft.h"

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

private slots:
    void on_actionOpen_triggered();
    // Use std::vector in the declaration to match the implementation
    void processIQData(const std::vector<std::complex<float>>& iq_data, int fft_size);

private:
    Ui::IQ_viewer *ui;
};
#endif // IQ_VIEWER_H
