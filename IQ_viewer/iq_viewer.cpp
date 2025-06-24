#include "iq_viewer.h"
#include "./ui_iq_viewer.h"

// Add this block to include kissfft headers
extern "C" {
#include "kiss_fft.h"
#include "kiss_fftr.h"
}

IQ_viewer::IQ_viewer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::IQ_viewer)
{
    ui->setupUi(this);

    // You can now use kiss_fft functions here
}

IQ_viewer::~IQ_viewer()
{
    delete ui;
}
