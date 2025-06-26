#ifndef IQ_VIEWER_H
#define IQ_VIEWER_H

#include <QMainWindow>
#include <vector>
#include <complex>
#include "qcustomplot.h"
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
    void processIQData(const std::vector<std::complex<float>>& iq_data, int fft_size);

    // Slots for sliders
    void on_timelineSlider_valueChanged(int value);
    void on_minLevelSlider_valueChanged(int value);
    void on_maxLevelSlider_valueChanged(int value);


private:
    Ui::IQ_viewer *ui;
    QCPColorMap *m_colorMap;
    int m_fft_size;
};
#endif // IQ_VIEWER_H
