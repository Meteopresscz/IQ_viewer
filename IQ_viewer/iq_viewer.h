#ifndef IQ_VIEWER_H
#define IQ_VIEWER_H

#include <QMainWindow>
#include <vector>
#include <complex>
#include "qcustomplot.h"
//#include "kiss_fft.h"

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
    void processIQData(const std::vector<std::complex<float>>& iq_data, int fft_size, int window_type);
    void reprocessData();

    void on_timelineSlider_valueChanged(int value);
    void updateColorMapRange();
    void on_colorMapComboBox_currentIndexChanged(int index);

    // clamp slot
    void clampAxisRanges();

private:
    void setupUiControls();
    void generateWindow(int type, int size);

    Ui::IQ_viewer *ui;
    QCPColorMap *m_colorMap;

    std::vector<std::complex<float>> m_iq_data;
    std::vector<float> m_window;
    int m_fft_size;

    // clamped valid raange
    QCPRange m_xDataRange;
    QCPRange m_yDataRange;
};
#endif // IQ_VIEWER_H
