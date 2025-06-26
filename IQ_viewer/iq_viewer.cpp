#include "iq_viewer.h"
#include "ui_iq_viewer.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <vector>
#include <complex>

// Add this block to include kissfft headers
extern "C" {
#include "kiss_fft.h"
#include "kiss_fftr.h"
}

IQ_viewer::IQ_viewer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::IQ_viewer)
    , m_colorMap(nullptr)
    , m_fft_size(1024)
{
    ui->setupUi(this);

    // --- QCustomPlot setup ---
    ui->fftPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->fftPlot->axisRect()->setupFullAxesBox(true);
    ui->fftPlot->xAxis->setLabel("Time (FFT Blocks)");
    ui->fftPlot->yAxis->setLabel("Frequency (Bins)");

    m_colorMap = new QCPColorMap(ui->fftPlot->xAxis, ui->fftPlot->yAxis);
    m_colorMap->data()->setSize(1, m_fft_size);
    m_colorMap->data()->setRange(QCPRange(0, 1), QCPRange(0, m_fft_size));
    m_colorMap->setGradient(QCPColorGradient::gpSpectrum);

    // --- Slider Connections ---
    connect(ui->timelineSlider, &QSlider::valueChanged, this, &IQ_viewer::on_timelineSlider_valueChanged);
    connect(ui->minLevelSlider, &QSlider::valueChanged, this, &IQ_viewer::on_minLevelSlider_valueChanged);
    connect(ui->maxLevelSlider, &QSlider::valueChanged, this, &IQ_viewer::on_maxLevelSlider_valueChanged);


    // --- Slider Initial Configuration ---
    ui->minLevelSlider->setRange(-120, 0);
    ui->maxLevelSlider->setRange(-120, 0);
    ui->minLevelSlider->setValue(-100);
    ui->maxLevelSlider->setValue(-20);


    ui->fftPlot->rescaleAxes();
    ui->fftPlot->replot();

    ui->statusbar->showMessage("Ready. Open an IQ file to begin.");
}

IQ_viewer::~IQ_viewer()
{
    delete ui;
}

void IQ_viewer::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open IQ File"), "", tr("Complex Float32 (*.cf32 *.cfile *.raw)"));
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
        return;
    }

    ui->statusbar->showMessage(tr("Loading %1...").arg(fileName));
    QCoreApplication::processEvents();


    QByteArray data = file.readAll();
    file.close();

    std::vector<std::complex<float>> iq_data;
    const int sample_count = data.size() / sizeof(std::complex<float>);
    const auto* samples = reinterpret_cast<const std::complex<float>*>(data.constData());

    iq_data.resize(sample_count);
    std::copy(samples, samples + sample_count, iq_data.begin());

    ui->statusbar->showMessage(tr("Processing FFT..."));
    QCoreApplication::processEvents();

    processIQData(iq_data, m_fft_size);
}

void IQ_viewer::processIQData(const std::vector<std::complex<float>>& iq_data, int fft_size)
{
    if (iq_data.empty()) {
        ui->statusbar->showMessage("No data to process.");
        return;
    }

    const int num_ffts = iq_data.size() / fft_size;

    m_colorMap->data()->setSize(num_ffts, fft_size);
    m_colorMap->data()->setRange(QCPRange(0, num_ffts), QCPRange(0, fft_size));

    kiss_fft_cfg cfg = kiss_fft_alloc(fft_size, 0, nullptr, nullptr);
    std::vector<kiss_fft_cpx> in(fft_size);
    std::vector<kiss_fft_cpx> out(fft_size);

    for (int i = 0; i < num_ffts; ++i) {
        for (int j = 0; j < fft_size; ++j) {
            in[j].r = iq_data[i * fft_size + j].real();
            in[j].i = iq_data[i * fft_size + j].imag();
        }

        kiss_fft(cfg, in.data(), out.data());

        for (int j = 0; j < fft_size; ++j) {
            int k = (j + fft_size / 2) % fft_size;
            float power = 10 * log10(out[k].r * out[k].r + out[k].i * out[k].i + 1e-12);
            m_colorMap->data()->setCell(i, j, power);
        }
    }

    free(cfg);

    // --- Update Timeline Slider Range ---
    ui->timelineSlider->setRange(0, num_ffts);
    ui->timelineSlider->setValue(0);


    // slider values for the initial color range
    on_minLevelSlider_valueChanged(ui->minLevelSlider->value());
    on_maxLevelSlider_valueChanged(ui->maxLevelSlider->value());


    ui->fftPlot->rescaleAxes();
    ui->fftPlot->replot();

    ui->statusbar->showMessage(tr("FFT calculation complete. %1 FFTs processed.").arg(num_ffts), 5000);
}

// --- Implementation slider Slots ---

void IQ_viewer::on_timelineSlider_valueChanged(int value)
{
    if (!m_colorMap) return;

    // "Zoom"
    double visibleWidth = m_colorMap->data()->keySize() - value;
    ui->fftPlot->xAxis->setRange(0, visibleWidth);
    ui->fftPlot->replot();
}

void IQ_viewer::on_minLevelSlider_valueChanged(int value)
{
    if (!m_colorMap) return;

    // min is not greater than max
    if (value >= ui->maxLevelSlider->value()) {
        ui->maxLevelSlider->setValue(value + 1);
    }
    m_colorMap->setDataRange(QCPRange(value, ui->maxLevelSlider->value()));
    ui->fftPlot->replot();
}

void IQ_viewer::on_maxLevelSlider_valueChanged(int value)
{
    if (!m_colorMap) return;

    // max is not less than min
    if (value <= ui->minLevelSlider->value()) {
        ui->minLevelSlider->setValue(value - 1);
    }
    m_colorMap->setDataRange(QCPRange(ui->minLevelSlider->value(), value));
    ui->fftPlot->replot();
}
