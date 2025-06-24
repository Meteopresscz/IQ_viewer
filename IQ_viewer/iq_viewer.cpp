#include "iq_viewer.h"
#include "./ui_iq_viewer.h"

IQ_viewer::IQ_viewer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::IQ_viewer)
{
    ui->setupUi(this);
}

IQ_viewer::~IQ_viewer()
{
    delete ui;
}
