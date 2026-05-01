# TinyML ESP32 Template

This is a boilerplate PlatformIO project for running Edge Impulse or TensorFlow Lite Micro models on an ESP32 or ESP32-S3.

## Why this template?

Deploying neural networks to microcontrollers requires a specific setup. You need to manage limited SRAM, integrate the C++ inference engine, and hook up sensors (microphone, IMU, camera) to feed raw data into your model.

## Setup Instructions

1. **PlatformIO:** Open this folder in VS Code with the PlatformIO extension.
2. **Library Dependency:** In your `platformio.ini`, you must include your exported Edge Impulse Arduino library as a dependency.
    ```ini
    lib_deps =
        https://github.com/yourusername/your-edge-impulse-export.zip
    ```
3. **Partition Scheme:** Neural networks are large. You MUST use a custom partition scheme to allocate enough Flash memory to the `app` partition. This is already configured in `platformio.ini` (`board_build.partitions = huge_app.csv`).

## Boilerplate Code (`src/main.cpp`)

```cpp
#include <Arduino.h>
#include <Your_Project_inferencing.h> // Replace with your downloaded library

// Allocate a buffer for your sensor data
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Setup");
}

void loop() {
    // 1. Gather raw data from sensors (e.g., MPU6050, I2S Mic)
    // fill_features_buffer(features);

    // 2. Wrap data in Edge Impulse signal struct
    signal_t signal;
    int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
        Serial.printf("Failed to create signal: %d\n", err);
        return;
    }

    // 3. Run Inference
    ei_impulse_result_t result = { 0 };
    err = run_classifier(&signal, &result, false /* debug */);
    if (err != EI_IMPULSE_OK) {
        Serial.printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

    // 4. Print Results
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        Serial.printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
    }
    
    delay(1000);
}
```
