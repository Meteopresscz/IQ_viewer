# IQ Spectrum Analyzer

Tool for visualizing I/Q data from software-defined radios and other sources. This application provides a spectrogram display, allowing for basic signal analysis.

![screenshot](https://github.com/Meteopresscz/IQ_viewer/blob/main/readme.png)

## Features

* **Spectrogram Display**: View I/Q data as a waterfall plot with a configurable color map.
* **Controls**:
    * Adjustable **FFT size** to balance resolution and performance.
    * Selectable **Windowing functions** (Rectangular, Hann, Hamming, Blackman) to reduce spectral leakage.
    * Adjustable **Min/Max power levels (dB)** to change the dynamic range of the display.
    * Adjustable **slider** to zoom and navigate through the capture.
* **File Support**: Loads raw I/Q data from `.cf32 .cfile .raw` (complex float32) files.
  
## Dependencies

To build the IQ Spectrum Analyzer from source, you will need the following dependencies:

* **Qt 5 or Qt 6**: The core application framework.
* **CMake**
* **C++17 compliant compiler**
* **KissFFT**: Included as a submodule for performing the FFT.
* **QCustomPlot**: Included as a submodule for plotting.

## Usage

1.  Launch the `IQ_viewer` executable.
2.  Go to `File > Open` in the menu bar.
3.  Select a `.cf32` `.cfile` or `.raw` containing complex float32 I/Q data.
4.  Use the controls in the left-hand dock to adjust the display and processing parameters.

## License

This project is licensed under the GNU General Public License v3.0
