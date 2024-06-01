# FOMO-object-detect-TFT-Esp32
This project was intended for displaying camera feed and [FOMO object detection model](https://edge-impulse.gitbook.io/docs/edge-impulse-studio/learning-blocks/object-detection/fomo-object-detection-for-constrained-devices) by [Edge Impulse](https://edgeimpulse.com/) inference results to TFT screen. The Hardware that was used were provided by [Wireless Solution Asia](https://wirelesssolution.asia/). To run this project users must obtain a model from Edge Impulse. <br/>
<strong>Please refer to this [repository](https://github.com/San279/Esp32-camera-to-tft) to set up TFT and Camera. </strong><br/> <br/>
![alt text](Images_for_readme/320_240.PNG)
<br/> <br/><br/>
## What you'll need
- [Arduino IDE](https://www.arduino.cc/en/software), preferably the latest ones, but older versions will still do the job.
- ESP32-S3 is preferable but older version will do just fine.
- ST7789 or any TFT screens.
- OV2640 camera or any OV series.

## Project files descriptions
1. FOMO_object_detect_TFT.ino - Containes Arduino codes displaying camera feed and FOMO results to TFT screens.

## How to install and run the project
1. Download the zip file and extract it to Arduino directory. <br /><br />
![alt text](Images_for_readme/folder_directory.PNG)
<br /><br /><br /><br />
2. Open Fomo_object_detect_TFT.ino, under tools change your Board to "ESP32S3 Dev Module" and PSRAM to "OPI PSRAM". <br /><br />
![alt text](/Images_for_readme/IDE_configure.PNG)
<br /><br /><br /><br />
3. Add zip folder of the model obtained from Edge Impulse to Arduino IDE. </strong> <br /><br />
![alt text](Images_for_readme/arduino_model_zip.PNG)
<br /><br />
![alt text](Images_for_readme/FOMO_model_zip.PNG)
<br /><br /><br /><br />
4. Connect pin no.1 to a push button. <br /><br />
![alt text](/Images_for_readme/IDE_configure.PNG)
<br /><br /><br /><br />
5. Upload the code to Esp32-S3. This process might take up to 30 minutes, and you're Done!! <br /><br />
![alt text](/Images_for_readme/320_240.PNG)
<br /><br /><br /><br />

## Project features
- User can change resolution of the camera when the push button is pressed. The avaiable resolution is as follow. <br /><br />
- 96 X 96 <br /><br />
![alt text](/Images_for_readme/96_96.PNG)
<br /><br /><br />
- QQVGA 160 X 120  <br /><br />
![alt text](/Images_for_readme/160_120.PNG)
<br /><br /><br />
- 176 X 144  <br /><br />
![alt text](/Images_for_readme/176_144.PNG)
<br /><br /><br />
- 240 X 176  <br /><br />
![alt text](/Images_for_readme/240_176.PNG)
<br /><br /><br />
- 240 X 240  <br /><br />
![alt text](/Images_for_readme/240_240.PNG)
<br /><br /><br />
- 320 X 240  <br /><br />
![alt text](/Images_for_readme/320_240.PNG)
<br /><br /><br />

## Credit
Thanks to [WIRELESS SOLUTION ASIA CO.,LTD](https://wirelesssolution.asia/) for providing AIOT board to support this project. Also thanks to [Bodmer / TFT_eSPI](https://github.com/Bodmer/TFT_eSPI/blob/master/README.md) for TFT libraries. Scripted used for Esp32 FOMO object detection inferencing were from [Edge Impulse](https://edge-impulse.gitbook.io/docs/edge-impulse-studio/learning-blocks/object-detection/fomo-object-detection-for-constrained-devices). 
