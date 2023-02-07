# Deneyap 9 Dof IMU MMC5603NJ Arduino Library
[FOR TURKISH VERSION](docs/README_tr.md) ![trflag](https://github.com/deneyapkart/deneyapkart-arduino-core/blob/master/docs/tr.png)

***** Add photo ****

Arduino library for Deneyap 9 Dof IMU Sensor MMC5603NJ

## :mag_right:Specifications 
- `Product ID` **M47**, **mpv1.0**
- `MCU` MMC5603NJ, LSM6DSM
- `Weight` 
- `Module Dimension`
- `I2C address` 0x60, 0x6B, 0x6A

| Address |  | 
| :---   | :---     |
| 0x60 | default address |

## :closed_book:Documentation
Deneyap 9 Dof IMU Sensor MMC5603NJ

[MMC5603NJ-datasheet](https://media.digikey.com/pdf/Data%20Sheets/MEMSIC%20PDFs/MMC5603NJ_RevB_7-12-18.pdf)

[LSM6DSM-datasheet](https://www.st.com/resource/en/datasheet/lsm6dsm.pdf)

[How to install a Arduino Library](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries)

### :paperclips:Dependencies
[Deneyap 6 Eksen Alaletsel Olcum Birimi](https://github.com/deneyapkart/deneyap-6-eksen-ataletsel-olcum-birimi-arduino-library)

## :pushpin:Deneyap 9 Dof IMU MMC5603NJ
This Arduino library allows you to use Deneyap 9 Dof IMU MMC5603NJ with I2C peripheral. You can use this library in your projects with any Arduino compatible board with I2C peripheral.

3 axes Accelerometer, 3 axes Gyro: [LSM6DSM Library](https://github.com/deneyapkart/deneyap-6-eksen-ataletsel-olcum-birimi-arduino-library)

## :globe_with_meridians:Repository Contents
- `/docs ` README_tr.md and product photos
- `/examples ` Examples with .ino extension sketches
- `/src ` Source files (.cpp .h)
- `keywords.txt ` Keywords from this library that will be highlighted in the Arduino IDE
- `library.properties ` General library properties for the Arduino package manager

## Version History
1.0.0 - initial release

## :rocket:Hardware Connections
- Deneyap 9 Dof IMU MMC5603NJ, LSM6DSM and Board can be connected with I2C cable
- or 3V3, GND, SDA and SCL pins can be connected with jumper cables

|9 Dof IMU| Function | Board pins | 
|:--- |   :---  | :---|
|3.3V | Power   |3.3V |      
|GND  | Ground  | GND | 
|SDA  | I2C Data  | SDA pin |
|SCL  | I2C Clock | SCL pin |
|INT1| interrupt |any GPIO PİN|
|INT2| interrupt |any GPIO PİN|

## :bookmark_tabs:License Information
Please review the [LICENSE](https://github.com/deneyapkart/deneyap-9-eksen-ataletsel-olcum-birimi-arduino-library/blob/master/LICENSE) file for license information.
