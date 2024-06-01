/* Edge Impulse Arduino examples
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Includes ---------------------------------------------------------------- */
#include <pvcColorCubes_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include <SPI.h>
#include <TFT_eSPI.h>
#include "esp_camera.h"

#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include <string>

#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5
#define Y9_GPIO_NUM 16
#define Y8_GPIO_NUM 17
#define Y7_GPIO_NUM 18
#define Y6_GPIO_NUM 12
#define Y5_GPIO_NUM 10
#define Y4_GPIO_NUM 8
#define Y3_GPIO_NUM 9
#define Y2_GPIO_NUM 11
#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13


/* Resolution -------------------------------------------------------- */
int EI_CAMERA_RAW_FRAME_BUFFER_COLS = 320;  //96
int EI_CAMERA_RAW_FRAME_BUFFER_ROWS = 240;  //96
int EI_CAMERA_FRAME_BYTE_SIZE = 3;

int camera_width[] = { 96, 160, 176, 240, 240, 320 };
int camera_height[] = { 96, 120, 144, 176, 240, 240 };

/*Push buttons for changing resolution-------------------------------- */
const int pushButton = 1;
int resolution_select = 5;
unsigned long previousMillis = 0;
uint32_t model_size;

/* Private variables ------------------------------------------------------- */
static bool debug_nn = false;  // Set this to true to see e.g. features generated from the raw signal
static bool is_initialised = false;
uint8_t *snapshot_buf;  //points to the output of the capture
sensor_t *s = NULL;
int colorCount = 0;
int fps = 0;

/* Function definitions ------------------------------------------------------- */
bool camera_setup();
bool camera_setting(String setting, int value);
void camera_deinit(void);
void change_resolution(int button_state);
void tft_setup();
void draw_fps_to_TFT(int fps);
bool draw_camera_to_TFT(camera_fb_t *fb);
void draw_inference_to_TFT(String label, float confidence, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void fill_black_screen_to_overflow_text_TFT();
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf);

TFT_eSPI tft = TFT_eSPI();

/**
* @brief      Arduino setup function
*/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //comment out the below line to start inference immediately after upload
  while (!Serial)
    ;

  pinMode(pushButton, INPUT_PULLUP);
  Serial.println("Edge Impulse Inferencing Demo");
  if (!camera_setup()) {
    ei_printf("Failed to initialize Camera!\r\n");
  } else {
    ei_printf("Camera initialized\r\n");
  }

  tft_setup();
  ei_sleep(1000);
}

void loop() {
  // instead of wait_ms, we'll wait on the signal, this allows threads to cancel us...
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    draw_fps_to_TFT(fps);
    previousMillis = currentMillis;
    fps = 0;
  }
  int buttonState = digitalRead(pushButton);
  if (buttonState == 1) {
    change_resolution(buttonState);
  }

  if (ei_sleep(5) != EI_IMPULSE_OK) {
    return;
  }
  snapshot_buf = (uint8_t *)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);

  // check if allocation was successful
  if (snapshot_buf == nullptr) {
    ei_printf("ERR: Failed to allocate snapshot buffer!\n");
    return;
  }

  ei::signal_t signal;
  signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
  signal.get_data = &ei_camera_get_data;

  if (ei_camera_capture((size_t)EI_CLASSIFIER_INPUT_WIDTH, (size_t)EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf) == false) {
    ei_printf("Failed to capture image\r\n");
    free(snapshot_buf);
    return;
  }

  // Run the classifier
  ei_impulse_result_t result = { 0 };

  EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
  if (err != EI_IMPULSE_OK) {
    ei_printf("ERR: Failed to run classifier (%d)\n", err);
    return;
  }

  // print the predictions
  ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
  bool bb_found = result.bounding_boxes[0].value > 0;
  for (size_t ix = 0; ix < result.bounding_boxes_count; ix++) {
    auto bb = result.bounding_boxes[ix];
    if (bb.value == 0) {
      continue;
    }
    ei_printf("    %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\n", bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
    colorCount++;
    draw_inference_to_TFT(bb.label, bb.value, bb.x, bb.y, bb.width, bb.height, colorCount);
  }
  colorCount = 0;
  if (!bb_found) {
    ei_printf("    No objects found\n");
  }
#else
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    ei_printf("    %s: %.5f\n", result.classification[ix].label,
              result.classification[ix].value);
  }
#endif

#if EI_CLASSIFIER_HAS_ANOMALY == 1
  ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif
  free(snapshot_buf);
  fps++;
}

bool camera_setup() {
  if (is_initialised) return true;
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 7;
  config.fb_count = 2;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    is_initialised = false;
    return false;
  }
  s = esp_camera_sensor_get();
  s->set_saturation(s, 1);
  s->set_vflip(s, 1);
  is_initialised = true;
  return true;
}


void camera_deinit(void) {

  //deinitialize the camera
  esp_err_t err = esp_camera_deinit();

  if (err != ESP_OK) {
    ei_printf("Camera deinit failed\n");
    return;
  }

  is_initialised = false;
  return;
}


/**
 * @brief      Capture, rescale and crop image
 *
 * @param[in]  img_width     width of output image
 * @param[in]  img_height    height of output image
 * @param[in]  out_buf       pointer to store output image, NULL may be used
 *                           if ei_camera_frame_buffer is to be used for capture and resize/cropping.
 *
 * @retval     false if not initialised, image captured, rescaled or cropped failed
 *
 */
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) {
  bool do_resize = false;
  char *part_buf[64];

  if (!is_initialised) {
    ei_printf("ERR: Camera is not initialized\r\n");
    return false;
  }

  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb) {
    ei_printf("Camera capture failed\n");
    return false;
  }

  if (!draw_camera_to_TFT(fb)) {
    ei_printf("Failed to draw to TFT\n");
  }

  bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);
  if (!converted) {
    ei_printf("Conversion failed\n");
    return false;
  }

  esp_camera_fb_return(fb);

  if ((img_width != EI_CAMERA_RAW_FRAME_BUFFER_COLS)
      || (img_height != EI_CAMERA_RAW_FRAME_BUFFER_ROWS)) {
    do_resize = true;
  }

  if (do_resize) {
    ei::image::processing::crop_and_interpolate_rgb888(
      out_buf,
      EI_CAMERA_RAW_FRAME_BUFFER_COLS,
      EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
      out_buf,
      img_width,
      img_height);
  }
  model_size = img_width;

  return true;
}

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr) {
  // we already have a RGB888 buffer, so recalculate offset into pixel index
  size_t pixel_ix = offset * 3;
  size_t pixels_left = length;
  size_t out_ptr_ix = 0;

  while (pixels_left != 0) {
    out_ptr[out_ptr_ix] = (snapshot_buf[pixel_ix + 2] << 16) + (snapshot_buf[pixel_ix + 1] << 8) + snapshot_buf[pixel_ix];

    // go to the next pixel
    out_ptr_ix++;
    pixel_ix += 3;
    pixels_left--;
  }
  // and done!
  return 0;
}

void tft_setup() {
  tft.init();
  tft.setSwapBytes(true);
  tft.setRotation(3);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
}

bool draw_camera_to_TFT(camera_fb_t *fb) {
  uint8_t *rgb565 = (uint8_t *)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * 3);
  bool converted = jpg2rgb565(fb->buf, fb->len, rgb565, JPG_SCALE_NONE);
  if (!converted) {
    Serial.println("failed to convert JPG to RGB565");
    return false;
  }
  tft.pushImage(0, 0, EI_CAMERA_RAW_FRAME_BUFFER_COLS, EI_CAMERA_RAW_FRAME_BUFFER_ROWS, (uint16_t *)rgb565);
  fill_black_screen_to_overflow_text_TFT(EI_CAMERA_RAW_FRAME_BUFFER_COLS);
  free(rgb565);
  return true;
}

void draw_inference_to_TFT(String label, float confidence, uint16_t x, uint16_t y, uint16_t width, uint16_t height, int color_count) {
  float xPos = float(x) * ((float)EI_CAMERA_RAW_FRAME_BUFFER_COLS/float(model_size));
  float yPos = float(y) * ((float)EI_CAMERA_RAW_FRAME_BUFFER_ROWS/float(model_size));
  int confidenceRound = (int)(confidence * 100);
  int rectangleWidth = (label.length() * 10) + 40; 
  if (color_count == 1) {
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawSpot(xPos, yPos, 6, TFT_RED, TFT_RED);
    //tft.fillRect(xPos, yPos - 23, rectangleWidth, 18, TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
  } else if (color_count == 2) {
      tft.setTextColor(TFT_WHITE, TFT_GREEN);
    tft.drawSpot(xPos, yPos, 6, TFT_GREEN, TFT_GREEN);
    //tft.fillRect(xPos, yPos - 23, rectangleWidth, 18, TFT_GREEN);
    tft.setTextColor(TFT_WHITE, TFT_GREEN);
  } else if (color_count == 3) {
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.drawSpot(xPos, yPos, 6, TFT_BLUE, TFT_BLUE);
    //tft.fillRect(xPos, yPos - 23, rectangleWidth, 18, TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
  } else {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawSpot(xPos, yPos, 6, TFT_BLACK, TFT_BLACK);
    //tft.fillRect(xPos, yPos - 23, rectangleWidth, 18, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  }
  tft.setCursor(xPos, yPos - 23);
  tft.print(label + " " + String(confidenceRound));
}

void draw_fps_to_TFT(int fps){
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (EI_CAMERA_RAW_FRAME_BUFFER_COLS != 320){
    tft.setCursor(0, 260);
    tft.print("FPS : " + String(fps) + " (" + String(EI_CAMERA_RAW_FRAME_BUFFER_COLS) + " X " + String(EI_CAMERA_RAW_FRAME_BUFFER_ROWS) + ")");
  }
}
void change_resolution(int state) {
  tft.fillScreen(TFT_BLACK);
  camera_deinit();
  if (!camera_setup()) {
    is_initialised = false;
    return;
  }
  resolution_select++;
  if (resolution_select < 5) {
    tft.setRotation(2);
  } else if (resolution_select > 5) {
    resolution_select = 0;
    tft.setRotation(2);
  } else if (resolution_select == 5) {  //for 320 X 240 resolution
    tft.setRotation(3);
  }
  s->set_framesize(s, (framesize_t)resolution_select);
  EI_CAMERA_RAW_FRAME_BUFFER_COLS = camera_width[resolution_select];
  EI_CAMERA_RAW_FRAME_BUFFER_ROWS = camera_height[resolution_select];
}

void fill_black_screen_to_overflow_text_TFT(int image_width){
  if (image_width < 240){
    int rectangleWidth = 240 - EI_CAMERA_RAW_FRAME_BUFFER_COLS;
    tft.fillRect(EI_CAMERA_RAW_FRAME_BUFFER_COLS, 0, rectangleWidth, 200, TFT_BLACK);
  }
}
#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "Invalid model for current sensor"
#endif
