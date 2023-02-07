/*
 *   LSM6DSM sensöründen 6 Eksen AccGyro Veri Okuma örneği,
 *
 *   Bu örnekte temel konfigürasyon ayarları yapılmaktadır.
 *   Sensörden gelen İvmeölçerden(Acc) X,Y,Z eksen değerleri Dönüölçerden(Gyro) X,Y,Z eksen değerleri ve C ile F cinsinden sıcaklık değerlerini
 *   seri termianle yazdırmaktadır.
 *
 *   Bu algılayıcı I2C haberleşme protokolü ile çalışmaktadır.
 *
 *   Bu örnek Deneyap 6-Eksen Ataletsel Ölçüm Birimi ve Deneyap 9-Eksen Ataletsel Ölçüm Birimi için oluşturulmuştur
 *      ------> www.....com <------ //docs
 *      ------> https://github.com/deneyapkart/deneyap-9-eksen-ataletsel-olcum-birimi-arduino-library <------
 *      ------> www.....com <------ //docs
 *      ------> https://github.com/deneyapkart/deneyap-6-eksen-ataletsel-olcum-birimi-arduino-library <------
 *
 */
#include <Deneyap_6EksenAtaletselOlcumBirimi.h>            // Deneyap_IvmeOlcerVeDonuOlcer.h kütüphanesi eklendi

LSM6DSM AccGyro;                                           // AccGyro icin Class tanimlamasi

void setup() {
    Serial.begin(115200);                                  // Seri haberleşme başlatıldı                                 
    if (AccGyro.begin() != IMU_SUCCESS) {                  // begin(slaveAdress) fonksiyonu ile cihazların haberleşmesi başlatıldı
        delay(2500);
        Serial.println("I2C bağlantısı başarısız ");       // I2C bağlantısı başarısız olursa seri terminale yazdırma
        while (1);
    }
}

void loop()
{
    Serial.println("\nAkselerometre degerleri");
    Serial.print("X ekseni: ");                             // X-eksen akselerometre verisi okuma
    Serial.print(AccGyro.readFloatAccelX());
    Serial.print("\tY ekseni: ");                           // Y-eksen akselerometre verisi okuma
    Serial.print(AccGyro.readFloatAccelY());
    Serial.print("\tZ ekseni: ");                           // Z-eksen akselerometre verisi okuma
    Serial.println(AccGyro.readFloatAccelZ());
    delay(500);

    Serial.println("\nGyro degerleri");
    Serial.print("X ekseni: ");                             // X-eksen gyro verisi okuma
    Serial.print(AccGyro.readFloatGyroX());
    Serial.print("\tY ekseni: ");                           // Y-eksen gyro verisi okuma
    Serial.print(AccGyro.readFloatGyroY());
    Serial.print("\tZ ekseni: ");                           // Z-eksen gyro verisi okuma
    Serial.println(AccGyro.readFloatGyroZ()); 
    delay(500);

    Serial.println("\nSicaklik degerleri");
    Serial.print("Celsius: ");                             // Sicaklik verisi okuma (Celsius)
    Serial.print(AccGyro.readTempC());
    Serial.print("\tFahrenheit: ");                        // Sicaklik verisi okuma (Fahrenheit)
    Serial.println(AccGyro.readTempF());
    delay(500);
}
