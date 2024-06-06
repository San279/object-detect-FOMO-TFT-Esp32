## camera-tft-esp32
 [For English version](https://github.com/San279/Esp32-camera-to-tft)
 <br/>
 <br/>
 โปรเจ็คนี้ถูกออกแบบมาใช้กับ [FOMO](https://docs.edgeimpulse.com/docs/edge-impulse-studio/learning-blocks/object-detection/fomo-object-detection-for-constrained-devices) AI ตรวจจับวัตถุ ในส่วนของการแสดงผล AI บนจอ TFT ใน AIoT บอร์ด หรือ Esp32Cam ที่มี PSRAM เท่านั้น
 <strong> - ก่อนใช้ไฟล์นี้ต้องมีไฟล์โมเดล [FOMO](https://github.com/San279/train-FOMO-object-detect-esp32) และ [เซ็ทกล้องกับจอ TFT](https://github.com/San279/Esp32-camera-to-tft) บน AIoT บอร์ด</strong> 
<br/>
## สิงที่ต้องมี
 - [AIoT](https://wirelesssolution.asia/) บอร์ด Esp32-S3 หรือ Esp32 ที่มี PSRAM
 - กล้อง OV 2640
 - ST7789 หรือ จอ TFT แบบไหนก้ได้ <br/> <br/>
 - [Arduino IDE](https://www.arduino.cc/en/software) อันเก่าหรือใหม่ก้ได้
   รูปแผงวงจรของกล้องกับจอ TFT ใน AIot บอร์ด <br/> <br/>
  ![alt text](/Images_for_readme/AIOT_push_button.jpg)
## โครงสร้าง
 - FOMO_object_detect_TFT_ino - มีไฟล์ Arduino เพื่อแสดงผล AI ขึ้นบนหน้าจอ TFT.
## วิธีรันโปรเจ็ค
<strong> 1. ดาวน์โหลดแฟ้มเป็น zip และแตกไฟล์ในแฟ้ม Arduino. </strong>
<br /><br />
![alt_text](/Images_for_readme/download_directory.PNG)
<br /><br /><br /><br />
<strong> 2. เปิดไฟล์ FOMO_object_detect_TFT. บน Arduino และกดไปที่ sketch และ Add .Zip library หลังจากนั้นให้เลือกไฟล์โมเดล </strong>
<br /><br />
![alt_text](/Images_for_readme/arduino_model_zip.PNG)
<br /><br /><br /><br />
<strong> 3. เปลี่ยนชื่อไฟล์บรรทัดที่ 24 ให้ตรงกับชื่อโปรเจ็คใน Edge Impulse </strong> 
<br/><br/>
![alt_text](/Images_for_readme/match_name.PNG)
<strong> 4. กดไปที่ tools ตรงตัวเลือกด้านบนและเปลี่ยน Board เป็น "ESP32S3 Dev Module" และเปลี่ยน PSRAM เป็น "OPI PSRAM".  </strong>
<br /><br />
![alt_text](/Images_for_readme/IDE_configure.PNG)
<br /><br /><br /><br />
<strong> 5. อัพโหลดโค้ดขึ้นบน ESP32-S3  </strong> <br/> <br/>
![alt_text](/Images_for_readme/320_240.PNG)
<br /><br /><br /><br />
## เครดิต 
ต้องขอขอบคุณ [WIRELESS SOLUTION ASIA CO.,LTD](https://wirelesssolution.asia/) สำหรับการสนับสนุนโปรเจ็คนี้ และ [Bodmer / TFT_eSPI](https://github.com/Bodmer/TFT_eSPI/blob/master/README.md)
สำหรับโค้ดส่วนจอ TFT และโค้ดส่วนรัน FOMO จาก [Edge Impulse](https://edge-impulse.gitbook.io/docs/edge-impulse-studio/learning-blocks/object-detection/fomo-object-detection-for-constrained-devices) 
