#include "iq_viewer.h"
#include "ui_iq_viewer.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <vector>
#include <complex>

//kissfft headers
extern "C" {
#include "kiss_fft.h"
#include "kiss_fftr.h"
}

IQ_viewer::IQ_viewer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::IQ_viewer)
{
    ui->setupUi(this);

    // Initial message in the status bar
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

    QByteArray data = file.readAll();
    file.close();

    std::vector<std::complex<float>> iq_data;
    const int sample_count = data.size() / sizeof(std::complex<float>);
    const auto* samples = reinterpret_cast<const std::complex<float>*>(data.constData());

    iq_data.resize(sample_count);
    std::copy(samples, samples + sample_count, iq_data.begin());


    ui->statusbar->showMessage(tr("Processing FFT..."));
    // Process the data and update the UI when done
    processIQData(iq_data, 1024); // Using a default size 1024
}

void IQ_viewer::processIQData(const std::vector<std::complex<float>>& iq_data, int fft_size)
{
    if (iq_data.empty()) {
        ui->statusbar->showMessage("No data to process.");
        return;
    }

    const int num_ffts = iq_data.size() / fft_size;

    kiss_fft_cfg cfg = kiss_fft_alloc(fft_size, 0, nullptr, nullptr);
    std::vector<kiss_fft_cpx> in(fft_size);
    std::vector<kiss_fft_cpx> out(fft_size);


    for (int i = 0; i < num_ffts; ++i) {
        for (int j = 0; j < fft_size; ++j) {
            in[j].r = iq_data[i * fft_size + j].real();
            in[j].i = iq_data[i * fft_size + j].imag();
        }

        kiss_fft(cfg, in.data(), out.data());

    }

    free(cfg);

    // Display a message in the status bar to confirm completion.
    ui->statusbar->showMessage(tr("FFT calculation complete. %1 FFTs processed.").arg(num_ffts), 5000); // Message shown for 5 seconds
}
