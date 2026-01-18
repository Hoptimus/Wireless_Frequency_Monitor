# Wireless_Frequency_Monitor
Monitor sound frequency using ESP32 as master and ESP8266 as remote visualisation

Technical Whitepaper: An ESP32-Based System for Directional Audio Monitoring and Frequency Analysis

1.0 Introduction to the System

Real-time audio analysis is a cornerstone of modern Internet of Things (IoT) applications, enabling devices to interact intelligently with their acoustic environments. The strategic importance of developing cost-effective, high-performance systems for tasks like sound source localization and frequency monitoring cannot be overstated, as they unlock capabilities ranging from predictive maintenance in industrial settings to sophisticated smart home automation. These systems must balance computational power, low-latency communication, and affordability to be viable for widespread deployment.

This whitepaper presents a novel ESP32-based directional audio monitoring system designed to meet these challenges. The system's architecture is centered on four orthogonally mounted analog sound sensor modules that capture acoustic data from cardinal directions. An ESP32 microcontroller serves as the central processing unit, executing a multi-stage digital signal processing pipeline on each audio channel in parallel. For wireless data transmission, the system leverages the ESP-NOW protocol to send processed frequency and amplitude data to a remote NodeMCU ESP8266 receiver with minimal latency.

The objective of this whitepaper is to provide a comprehensive technical examination of the system's architecture, from its foundational technologies to its real-world performance. We will explore the digital signal processing pipeline, analyze key implementation parameters, present empirical validation results, and discuss the critical engineering trade-offs that informed the final design.

2.0 Core Enabling Technologies

A robust system design relies on the strategic selection of foundational technologies that provide the necessary performance, efficiency, and flexibility. The architecture of this audio monitoring system is built upon two technological pillars: the Fast Fourier Transform, which enables real-time frequency analysis on a resource-constrained microcontroller, and the ESP-NOW protocol, which facilitates low-latency wireless communication essential for IoT applications. This section will deconstruct these two core components.

2.1 The Role of Fast Fourier Transform (FFT) in Audio Analysis

The Fourier Transform is the mathematical bridge that converts time-domain signals, such as raw audio waveforms, into their frequency-domain components. This transformation is fundamental to digital signal processing (DSP), as it deconstructs a complex signal into the sum of simple sine waves of different frequencies and amplitudes. By revealing the specific frequencies that exist within a signal, the FFT enables the identification of hidden patterns, fundamental tones, and harmonic content that are otherwise obscured in the time-domain representation.

Three primary variants of the Fourier Transform exist, each suited to a specific context:

* Discrete Fourier Transform (DFT): Applied to digital signals that consist of discrete, finite samples rather than continuous waves.
* Fast Fourier Transform (FFT): A highly efficient algorithm for calculating the DFT. With a computational complexity of O(n log n) compared to the DFT's O(n²), the FFT is the backbone of modern digital signal processing and is essential for real-time analysis on embedded systems.
* Fourier Series: A specialized version used exclusively for signals that are periodic, meaning they repeat in a predictable pattern over time.

For this system, the Fast Fourier Transform is the critical enabling algorithm. Its computational efficiency is paramount, allowing the ESP32 to process four independent audio streams in real-time without overwhelming its processing capabilities.

2.2 ESP-NOW Protocol for Low-Latency IoT Communication

ESP-NOW is a fast, low-power, connectionless Wi-Fi protocol developed by Espressif for direct device-to-device communication. It operates without requiring an access point or router, making it an ideal choice for localized IoT networks where speed and efficiency are critical.

The architectural advantages of ESP-NOW make it uniquely suited for this audio monitoring application:

* Reduced Latency: By operating primarily at the data-link layer, ESP-NOW bypasses the network, transport, and application layers of a typical Wi-Fi connection. This streamlined approach significantly reduces transmission time, enabling near real-time data transfer.
* Efficiency: The simplified protocol eliminates the need for complex packet headers and handshaking procedures, reducing overhead and minimizing delays caused by packet loss in congested environments.
* Resource Management: ESP-NOW consumes minimal CPU and flash memory resources, leaving more capacity available for the computationally intensive FFT calculations.
* Security: Data transmission is secured using the Elliptic-curve Diffie–Hellman (ECDH) key exchange protocol and 128-bit AES encryption.
* Versatility: The protocol supports flexible one-to-one, one-to-many, and many-to-many device configurations, allowing the system to be scaled or adapted for different use cases.
* Payload Capacity: Each transmission can carry a data payload of up to 250 bytes, which is sufficient for sending the processed frequency and amplitude data from all four sensors.
* Power Consumption: A window synchronization mechanism allows devices to wake up only when necessary, greatly reducing power usage and making the protocol suitable for battery-powered applications.

By leveraging these features, ESP-NOW provides a robust and efficient communication link between the audio analyzer and the remote display unit.

3.0 System Architecture and Implementation

The system's architecture integrates specialized hardware with a sophisticated software pipeline to achieve its monitoring objectives. The physical arrangement of the sensors is designed for directional sensitivity, while the multi-stage digital signal processing pipeline systematically transforms raw audio samples into actionable data. This section details the hardware configuration, the DSP workflow, and the core parameters that govern the system's analytical performance.

3.1 Hardware Configuration and Physical Setup

The system is composed of two distinct units: a transmitter/analyzer unit that captures and processes audio, and a receiver/display unit that visualizes the results.

* Transmitter/Analyzer Unit:
  * ESP32 Development Board
  * Analog Sound Sensors (x4)
* Receiver/Display Unit:
  * NodeMCU ESP8266 Development Board
  * I2C 20x4 LCD display
  * LEDs (x4)
  * 220 Ω Resistors (one per LED, 4 total)

The physical arrangement of the transmitter is critical for directional sensing. The four analog sound sensor modules are spatially arranged in an orthogonal configuration on the ESP32 platform. This layout ensures that the system can capture sound wavefronts from all cardinal directions. The entire transmitter assembly is housed within a cardboard enclosure measuring 270 mm x 225 mm x 90 mm to improve the directional reception of audio signals.

3.2 Digital Signal Processing (DSP) Pipeline

To ensure consistent and comparable results across all channels, each of the four microphone inputs is processed through an identical five-stage analysis pipeline.

1. Total Energy Integration: The initial stage measures the aggregate acoustic power across the entire captured spectrum. This provides a raw assessment of the overall loudness or Sound Pressure Level (SPL) of the incoming signal.
2. Spectral Window Conditioning: A Hamming taper (windowing function) is applied to the block of sampled data. This mathematical conditioning smooths the signal at its edges, eliminating discontinuities that can cause spectral leakage—an artifact where signal energy "leaks" into adjacent frequency bins, distorting the FFT output.
3. Frequency Domain Transformation: A 512-point Fast Fourier Transform is performed on the windowed data. This core operation decomposes the time-domain signal into its constituent frequency components, revealing the amplitude of the signal at discrete frequency intervals.
4. Dominant Frequency Extraction: The system analyzes the FFT output to identify the frequency bin with the highest energy. This "major peak" represents the primary tonal component of the captured sound, identified with the precision defined by the system's frequency resolution.
5. Logarithmic Level Quantification: The Root Mean Square (RMS) amplitudes from the FFT are converted into a more intuitive logarithmic scale (relative decibels) using the following formula. This conversion to a logarithmic decibel scale is critical because human hearing perceives sound intensity logarithmically, not linearly. This ensures that the reported Sound Pressure Level (SPL) values are both intuitive and representative of how an operator would actually perceive the sound's loudness. fRMS = 20.0 × log10(RMS + 1e-6)

3.3 Core Implementation Parameters

The performance of the FFT analysis is governed by a set of primary and derived parameters. These values define the system's measurement capabilities and represent a deliberate trade-off between speed and precision.

Parameter	Definition
Sampling Frequency (fs): 8000 Hz	The number of audio samples captured per second.
Blocklength (BL): 512 samples	The number of samples in each analysis window (must be a power of 2).
Nyquist Frequency (fn): 4000 Hz	The maximum detectable frequency, calculated as fs/2.
Capture Duration (D): 64 ms	The time required to capture one block of samples, calculated as BL/fs.
Frequency Resolution (∆f): 15.625 Hz	The frequency spacing between two consecutive FFT bins, calculated as fs/BL.

These parameters establish a clear operational envelope for the system. The transition from this theoretical design to practical application is validated through empirical testing, which quantifies the system's real-world performance.

4.0 Performance Analysis and Validation

Empirical testing is critical to validate any system's design and quantify its performance in a real-world context. To assess the accuracy and operational range of the audio monitoring system, a series of experiments were conducted. This section presents the results from calibration tests using standard tuning forks to verify frequency accuracy and analyzes the inherent limitations of the measurement methodology.

4.1 Frequency Response Calibration with Tuning Forks

To analyze the frequency response of each microphone sensor, a set of standard laboratory tuning forks was used to generate pure, known frequencies. The system's detected frequency for each input was recorded and compared against the target frequency of the tuning fork. The results are summarized below.

Tuning Fork Frequency	Detected Frequency (MIC 1)	Detected Frequency (MIC 2)	Detected Frequency (MIC 3)	Detected Frequency (MIC 4)
256 Hz	254 Hz	254 Hz	254 Hz	254 Hz
288 Hz	289 Hz	289 Hz	289 Hz	289 Hz
320 Hz	325 Hz	325 Hz	325 Hz	325 Hz
341 Hz	343 Hz	343 Hz	343 Hz	343 Hz
384 Hz	388 Hz	388 Hz	389 Hz	388 Hz
426 Hz	431 Hz	431 Hz	431 Hz	431 Hz
480 Hz	484 Hz	484 Hz	484 Hz	484 Hz
512 Hz	516 Hz	516 Hz	516 Hz	516 Hz

4.2 Analysis of Measurement Discrepancies

The observed differences between the tuning fork frequencies and the system's measurements are not due to sensor error but are a direct consequence of the FFT's discrete nature. The FFT.majorPeak() function returns the center frequency of the FFT bin containing the most signal energy; it does not interpolate to find the true peak frequency between bins. This "binning" effect is dictated by the system's frequency resolution of 15.625 Hz.

The 426 Hz test case provides a clear illustration of this principle. The true frequency of 426 Hz falls between two adjacent FFT bins:

* Bin 27: Represents a center frequency of 15.625 Hz × 27 = 421.875 Hz
* Bin 28: Represents a center frequency of 15.625 Hz × 28 = 437.500 Hz

The resulting output of 431 Hz, as recorded in the experiment, indicates the algorithm determined the peak energy to be between the centers of Bin 27 and Bin 28, reporting a value close to their midpoint. This demonstrates a core performance characteristic tied directly to the fundamental parameters of the DSP pipeline.

4.3 Signal Detection Range Analysis

A second experiment was conducted using an online tone generator to evaluate the relationship between sound frequency and detection distance. Tones of varying frequencies were played at a fixed volume, and the maximum distance at which the sensor could reliably detect the frequency was recorded.

Test Tone	Detected Frequency	Detection Distance
A4 (440 Hz)	441 Hz	9 cm
D5 (587 Hz)	591 Hz	21 cm
F5 (698 Hz)	703 Hz	34 cm
A5 (880 Hz)	884 Hz	50 cm
D6 (1174 Hz)	1180 Hz	56 cm
F6 (1396 Hz)	1405 Hz	62 cm
A6 (1760 Hz)	1769 Hz	67 cm

The results clearly indicate a positive correlation: as the frequency of the sound increases, the distance at which it can be reliably detected by the sensor also increases. This finding provides valuable insight into the system's operational range for different acoustic sources.

5.0 Technical Rationale and System Trade-offs

The design of an embedded system is a process of deliberate engineering decisions and compromises. The choice of a central processing unit and the configuration of its software algorithms are driven by the specific demands of the application. This section justifies the selection of the ESP32 microcontroller over alternatives by analyzing its unique combination of features and examines the fundamental trade-off between measurement resolution and system latency inherent in FFT-based analysis.

5.1 Justification for the ESP32 Platform

The ESP32 was selected as the core of this system because it offers a unique convergence of processing power, peripheral support, and cost-effectiveness that is unmatched by other microcontrollers for this specific application.

* High-Speed Dual-Core Processing: The computational load of the DSP pipeline is significant. A single 512-point complex FFT requires approximately 4,600 floating-point operations. Processing four channels in parallel results in a total load of over 18,400 operations per measurement cycle. The ESP32's dual-core architecture is essential for handling this workload and maintaining the system's ~300ms update rate.
* High-Fidelity ADC: The integrated 12-bit Analog-to-Digital Converter (ADC) is a key component for achieving quality audio sampling, providing a higher dynamic range and finer amplitude resolution than the 10-bit ADCs found on many competing platforms.
* Integrated ESP-NOW Protocol: Native support for the ESP-NOW wireless protocol is a critical advantage. This integration eliminates the need for external radio modules and complex driver integration, which would be required on platforms such as the STM32, thereby simplifying the hardware design and reducing costs.
* Sufficient Memory Architecture: Real-time FFT processing demands significant RAM. Each of the four channels requires 8KB of memory for its data buffers, totaling 32KB for buffers alone. The total RAM requirement of approximately 53KB (33KB for dynamic buffers and 20KB for program code) is well within the ESP32's capabilities.
* Cost-Effectiveness: When considering the combined requirements for processing speed, integrated wireless communication, ADC quality, and memory, no alternative microcontroller provides this specific combination of features at a comparable price point.

In essence, the ESP32 is not merely a suitable choice but the only viable off-the-shelf microcontroller that holistically addresses the project's core requirements: parallel multi-channel DSP, high-fidelity analog acquisition, native low-latency wireless communication, and sufficient memory, all within a cost-effective footprint.

5.2 The Resolution vs. Latency Trade-off

A fundamental engineering trade-off in any FFT-based system exists between frequency resolution and measurement latency, both of which are controlled by the Blocklength (BL) parameter.

* A smaller Blocklength results in a shorter data capture duration, leading to faster measurement cycles and lower system latency. However, it provides a coarser frequency resolution (larger ∆f), making it more difficult to distinguish between closely spaced frequencies.
* A larger Blocklength increases the frequency resolution (smaller ∆f), allowing for finer discrimination between tones. However, this comes at the cost of a longer capture duration, which increases the overall system latency.

The system's chosen configuration of 512 samples represents an optimal balance for this application. It delivers a practical frequency resolution of 15.625 Hz, which is sufficient for many monitoring tasks, while enabling a full four-microphone scan cycle of approximately 300ms. This update rate is suitable for real-time monitoring and provides timely feedback via the wireless ESP-NOW link.

6.0 Conclusion

This project successfully demonstrates the design and implementation of an ESP32-based directional audio monitoring system. By integrating multi-channel audio capture, real-time FFT-based signal processing, and low-latency wireless transmission, the system provides a powerful and cost-effective solution for IoT audio analysis. The strategic selection of the ESP32 platform and the ESP-NOW protocol proved essential to meeting the demanding computational and communication requirements of the application.

The analysis of the system's performance underscores the key engineering trade-off between frequency resolution and update latency. The currently implemented parameters are optimized to provide a practical balance, delivering a 15.625 Hz resolution with a near real-time ~300ms full-system scan cycle suitable for wireless monitoring. This work serves as a robust foundation for further development in intelligent acoustic sensing.

The project's source code is publicly available for review and further development on GitHub: https://github.com/Hoptimus/Wireless_Frequency_Monitor.git
