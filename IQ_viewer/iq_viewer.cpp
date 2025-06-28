#include "iq_viewer.h"
#include "ui_iq_viewer.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <vector>
#include <complex>
#include <cmath>

//kissfft headers
extern "C" {
#include "kiss_fft.h"
//#include "kiss_fftr.h"
}

IQ_viewer::IQ_viewer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::IQ_viewer)
    , m_colorMap(nullptr)
    , m_fft_size(1024)
{
    ui->setupUi(this);

    setupUiControls();

    // --- QCustomPlot setup ---
    ui->fftPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->fftPlot->axisRect()->setupFullAxesBox(true);
    ui->fftPlot->xAxis->setLabel("Time (FFT Blocks)");
    ui->fftPlot->yAxis->setLabel("Frequency (Bins)");

    m_colorMap = new QCPColorMap(ui->fftPlot->xAxis, ui->fftPlot->yAxis);
    m_colorMap->data()->setSize(1, m_fft_size);
    m_colorMap->data()->setRange(QCPRange(0, 1), QCPRange(0, m_fft_size));

    ui->fftPlot->rescaleAxes();
    ui->fftPlot->replot();

    ui->statusbar->showMessage("Ready. Open an IQ file to begin.");
}

IQ_viewer::~IQ_viewer()
{
    delete ui;
}

void IQ_viewer::setupUiControls()
{
    // --- Connections ---
    connect(ui->timelineSlider, &QSlider::valueChanged, this, &IQ_viewer::on_timelineSlider_valueChanged);
    connect(ui->minLevelSlider, &QSlider::sliderReleased, this, &IQ_viewer::updateColorMapRange);
    connect(ui->maxLevelSlider, &QSlider::sliderReleased, this, &IQ_viewer::updateColorMapRange);
    connect(ui->colorMapComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IQ_viewer::on_colorMapComboBox_currentIndexChanged);
    connect(ui->fftSizeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IQ_viewer::reprocessData);
    connect(ui->windowFunctionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IQ_viewer::reprocessData);

    // --- range change ---
    connect(ui->fftPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(clampAxisRanges()));
    connect(ui->fftPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(clampAxisRanges()));


    // --- default settings ---
    ui->fftSizeComboBox->addItems({"256", "512", "1024", "2048", "4096", "8192"});
    ui->windowFunctionComboBox->addItems({"Rectangular", "Hann", "Hamming", "Blackman"});
    ui->fftSizeComboBox->setCurrentIndex(2);
    ui->windowFunctionComboBox->setCurrentIndex(1);


    // --- Slider Init ---
    ui->minLevelSlider->setRange(-120, 0);
    ui->maxLevelSlider->setRange(-120, 0);
    ui->minLevelSlider->setValue(-100);
    ui->maxLevelSlider->setValue(-20);
}

void IQ_viewer::generateWindow(int type, int size)
{

    // --- FFT windowing ---
    m_window.resize(size);
    for (int i = 0; i < size; i++)
    {
        switch (type)
        {
        case 1: // Hann
            m_window[i] = 0.5 * (1 - cos(2 * M_PI * i / (size - 1)));
            break;
        case 2: // Hamming
            m_window[i] = 0.54 - 0.46 * cos(2 * M_PI * i / (size - 1));
            break;
        case 3: // Blackman
            m_window[i] = 0.42 - 0.5 * cos(2 * M_PI * i / (size - 1)) + 0.08 * cos(4 * M_PI * i / (size - 1));
            break;
        default: // Case 0: Rectangular
            m_window[i] = 1.0;
            break;
        }
    }
}

void IQ_viewer::on_actionOpen_triggered()
{
    // --- open file ---
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open IQ File"), "", tr("Complex Float32 (*.cf32 *.cfile)"));
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
        return;
    }

    // --- basic info ---
    ui->statusbar->showMessage(tr("Loading %1...").arg(fileName));
    QCoreApplication::processEvents();

    QByteArray data = file.readAll();
    file.close();

    m_iq_data.clear();
    const int sample_count = data.size() / sizeof(std::complex<float>);
    const auto* samples = reinterpret_cast<const std::complex<float>*>(data.constData());
    m_iq_data.resize(sample_count);
    std::copy(samples, samples + sample_count, m_iq_data.begin());

    reprocessData();
}

// --- when settings change ---
void IQ_viewer::reprocessData()
{
    if (m_iq_data.empty()) return;

    m_fft_size = ui->fftSizeComboBox->currentText().toInt();
    int window_type = ui->windowFunctionComboBox->currentIndex();

    ui->statusbar->showMessage(tr("Processing FFT with size %1...").arg(m_fft_size));
    QCoreApplication::processEvents();

    processIQData(m_iq_data, m_fft_size, window_type);
}

// --- IQ processing ---
void IQ_viewer::processIQData(const std::vector<std::complex<float>>& iq_data, int fft_size, int window_type)
{
    if (iq_data.empty()) {
        ui->statusbar->showMessage("No data to process.");
        return;
    }

    generateWindow(window_type, fft_size);

    const int num_ffts = iq_data.size() / fft_size;

    m_colorMap->data()->setSize(num_ffts, fft_size);
    m_colorMap->data()->setRange(QCPRange(0, num_ffts), QCPRange(0, fft_size));

    kiss_fft_cfg cfg = kiss_fft_alloc(fft_size, 0, nullptr, nullptr);
    std::vector<kiss_fft_cpx> in(fft_size);
    std::vector<kiss_fft_cpx> out(fft_size);

    for (int i = 0; i < num_ffts; ++i) {
        for (int j = 0; j < fft_size; ++j) {
            in[j].r = iq_data[i * fft_size + j].real() * m_window[j];
            in[j].i = iq_data[i * fft_size + j].imag() * m_window[j];
        }
        kiss_fft(cfg, in.data(), out.data());
        for (int j = 0; j < fft_size; ++j) {
            int k = (j + fft_size / 2) % fft_size;
            float power = 10 * log10(out[k].r * out[k].r + out[k].i * out[k].i + 1e-12);
            m_colorMap->data()->setCell(i, j, power);
        }
    }
    free(cfg);

    // --- data range ---
    m_xDataRange = QCPRange(0, num_ffts);
    m_yDataRange = QCPRange(0, fft_size);

    // --- full view Init ---
    ui->fftPlot->xAxis->setRange(m_xDataRange);
    ui->fftPlot->yAxis->setRange(m_yDataRange);

    ui->timelineSlider->setRange(0, num_ffts);
    ui->timelineSlider->setValue(num_ffts);

    on_colorMapComboBox_currentIndexChanged(ui->colorMapComboBox->currentIndex());
    updateColorMapRange();

    ui->fftPlot->replot();
    ui->statusbar->showMessage(tr("FFT calculation complete. %1 FFTs processed.").arg(num_ffts), 5000);
}

// --- clamp plot/graph ---
void IQ_viewer::clampAxisRanges()
{
    // Clamp X-Axis
    QCPRange currentXRange = ui->fftPlot->xAxis->range();
    if (currentXRange.lower < m_xDataRange.lower)
    {
        currentXRange.lower = m_xDataRange.lower;
        currentXRange.upper = m_xDataRange.lower + currentXRange.size();
    }
    if (currentXRange.upper > m_xDataRange.upper)
    {
        currentXRange.upper = m_xDataRange.upper;
        currentXRange.lower = m_xDataRange.upper - currentXRange.size();
    }
    ui->fftPlot->xAxis->setRange(currentXRange);

    // Clamp Y-Axis
    QCPRange currentYRange = ui->fftPlot->yAxis->range();
    if (currentYRange.lower < m_yDataRange.lower)
    {
        currentYRange.lower = m_yDataRange.lower;
        currentYRange.upper = m_yDataRange.lower + currentYRange.size();
    }
    if (currentYRange.upper > m_yDataRange.upper)
    {
        currentYRange.upper = m_yDataRange.upper;
        currentYRange.lower = m_yDataRange.upper - currentYRange.size();
    }
    ui->fftPlot->yAxis->setRange(currentYRange);
}

// --- "zoom" ---
void IQ_viewer::on_timelineSlider_valueChanged(int value)
{
    if (!m_colorMap || value == 0) return;

    ui->fftPlot->xAxis->setRange(0, value);
    ui->fftPlot->replot();
}


// --- colors ---
void IQ_viewer::updateColorMapRange()
{
    if (!m_colorMap) return;
    int minVal = ui->minLevelSlider->value();
    int maxVal = ui->maxLevelSlider->value();
    if (minVal >= maxVal) {
        minVal = maxVal - 1;
        ui->minLevelSlider->setValue(minVal);
    }
    m_colorMap->setDataRange(QCPRange(minVal, maxVal));
    ui->fftPlot->replot();
}

// --- colors choices ---
void IQ_viewer::on_colorMapComboBox_currentIndexChanged(int index)
{
    if (!m_colorMap) return;
    QCPColorGradient gradient;
    switch (index)
    {
    case 0: gradient = QCPColorGradient::gpGrayscale; break;
    case 1: gradient = QCPColorGradient::gpSpectrum; break;
    case 2: gradient = QCPColorGradient::gpJet; break;
    }
    m_colorMap->setGradient(gradient);
    ui->fftPlot->replot();
}
