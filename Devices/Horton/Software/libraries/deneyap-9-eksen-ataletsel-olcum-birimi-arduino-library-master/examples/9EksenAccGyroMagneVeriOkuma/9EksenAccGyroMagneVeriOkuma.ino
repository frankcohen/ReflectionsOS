/*
 *   LSM6DSM ve MMC5603NJ sensöründen 9 Eksen AccGyro Veri Okuma örneği,
 *
 *   Bu örnekte temel konfigürasyon ayarları yapılmaktadır.
 *   Sensörlerden gelen İvmeölçerden(Acc) X,Y,Z Dönüölçerden(Gyro) X,Y,Z, Manyetometre(Magne) X,Y,Z ve Sıcaklık C ve F cinsinden sıcaklık değerlerini
 *   seri termianle yazdırmaktadır.
 *
 *   Bu algılayıcı I2C haberleşme protokolü ile çalışmaktadır.
 *
 *   Bu örnek Deneyap 9-Eksen Ataletsel Ölçüm Birimi için oluşturulmuştur
 *      ------> www.....com <------ //docs
 *      ------> https://github.com/deneyapkart/deneyap-9-eksen-ataletsel-olcum-birimi-arduino-library <------
 *
 */
#include <Deneyap_6EksenAtaletselOlcumBirimi.h>                // Deneyap_6EksenAtaletselOlcumBirimi.h kütüphanesi eklendi
#include <Deneyap_9EksenAtaletselOlcumBirimi.h>                // Deneyap_9EksenAtaletselOlcumBirimi.h kütüphanesi eklendi

LSM6DSM AccGyro;                                               // LSM6DSM icin Class tanimlamasi
MAGNETOMETER Magne;                                            // MAGNETOMETER icin Class tanimlamasi

void setup() {
    Serial.begin(115200);                                      // Seri haberleşme başlatıldı
    if (AccGyro.begin() != IMU_SUCCESS) {                      // begin(slaveAdress) fonksiyonu ile cihazların haberleşmesi başlatıldı
        delay(2500);
        Serial.println("LSM6DSM I2C bağlantısı başarısız ");   // I2C bağlantısı başarısız olursa seri terminale yazdırma
        while (1);
    }
    if (!Magne.begin(0x60)) {                                   // begin(slaveAdress) fonksiyonu ile cihazların haberleşmesi başlatıldı
        delay(2500);
        Serial.println("MMC5603NJ I2C bağlantısı başarısız ");  // I2C bağlantısı başarısız olursa seri terminale yazdırma
        while (1);
    }
}

void loop() {
    Serial.println("\nAkselerometre degerleri");
    Serial.print("X ekseni: ");                                  // X-eksen akselerometre verisi okuma
    Serial.print(AccGyro.readFloatAccelX());
    Serial.print("\tY ekseni: ");                                // Y-eksen akselerometre verisi okuma
    Serial.print(AccGyro.readFloatAccelY());
    Serial.print("\tZ ekseni: ");                                // Z-eksen akselerometre verisi okuma
    Serial.println(AccGyro.readFloatAccelZ());
    delay(500);

    Serial.println("\nGyro degerleri");
    Serial.print("X ekseni: ");                                  // X-eksen gyro verisi okuma
    Serial.print(AccGyro.readFloatGyroX());
    Serial.print("\tY ekseni: ");                                // Y-eksen gyro verisi okuma
    Serial.print(AccGyro.readFloatGyroY());
    Serial.print("\tZ ekseni: ");                                // Z-eksen gyro verisi okuma
    Serial.println(AccGyro.readFloatGyroZ());
    delay(500);

    Serial.println("\nMagnetometre degerleri");
    Serial.print("X ekseni:");
    Serial.print(Magne.readMagnetometerX());                     // X-eksen manyetometre verisi okuma
    Serial.print("\tY ekseni:");
    Serial.print(Magne.readMagnetometerY());                     // Y-eksen manyetometre verisi okuma
    Serial.print("\tZ ekseni:");
    Serial.println(Magne.readMagnetometerZ());                   // Z-eksen manyetometre verisi okuma
    delay(500);

    Serial.println("\nSicaklik degerleri");
    Serial.print("Celsius: ");                                   // Sicaklik verisi okuma (Celsius)
    Serial.print(AccGyro.readTempC());
    Serial.print("\tFahrenheit: ");                              // Sicaklik verisi okuma (Fahrenheit)
    Serial.println(AccGyro.readTempF());
    delay(500);
}
