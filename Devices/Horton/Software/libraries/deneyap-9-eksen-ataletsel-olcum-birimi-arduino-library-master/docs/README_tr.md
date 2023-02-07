# Deneyap 9-Eksen Ataletsel Ölçüm Birimi MMC5603NJ, LSM6DSM Arduino Kütüphanesi

***** Fotoğraf eklenecek ****

Deneyap 9-Eksen Ataletsel Ölçüm Birimi MMC5603NJ için Arduino kütüphanesidir.

## :mag_right:Özellikler 
- `Ürün ID` **M47**, **mpv1.0**
- `MCU` MMC5603NJ, LSM6DSM
- `Weight` 
- `Module Dimension`
- `I2C Adres` 0x60, 0x6B, 0x6A

| Adres |  | 
| :---      | :---     |
| 0x60 | varsayılan adres |

## :closed_book:Dokümanlar
Deneyap 9-Eksen Ataletsel Ölçüm Birimi MMC5603NJ

[MMC5603NJ-datasheet](https://media.digikey.com/pdf/Data%20Sheets/MEMSIC%20PDFs/MMC5603NJ_RevB_7-12-18.pdf)

[LSM6DSM-datasheet](https://www.st.com/resource/en/datasheet/lsm6dsm.pdf)

[Arduino Kütüphanesi Nasıl İndirilir](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries)

### :paperclips:Bağlı Olduğu Kütüphane
[Deneyap 6 Eksen Alaletsel Olcum Birimi](https://github.com/deneyapkart/deneyap-6-eksen-ataletsel-olcum-birimi-arduino-library)

## :pushpin:Deneyap 9-Eksen Ataletsel Ölçüm Birimi
Bu Arduino kütüphanesi Deneyap 9-Eksen Ataletsel Ölçüm Birimi I2C çevre birimi ile kullanılmasını sağlar. Arduino ile uyumlu, I2C çevre birimine sahip herhangi bir geliştirme kartı ile bu kütüphaneyi projelerinizde kullanabilirsiniz. 

3 Eksen İvmeölçer, 3 Eksen Dönüölçer : [LSM6DSM Library](https://github.com/deneyapkart/deneyap-6-eksen-ataletsel-olcum-birimi-arduino-library)

## :globe_with_meridians:Repo İçeriği
- `/docs` README_tr.md ve ürün fotoğrafları
- `/examples` .ino uzantılı örnek uygulamalar
- `/src` kütüphane için .cpp ve .h uzantılı dosyalar
- `keywords.txt` Arduino IDE'de vurgulanacak anahtar kelimeler
- `library.properties` Arduino yöneticisi için genel kütüphane özellikleri

## Sürüm Geçmişi
1.0.0 - ilk sürüm

## :rocket:Donanım Bağlantıları
- Deneyap 9-Eksen Ataletsel Ölçüm Birimi ile kullanılan geliştirme kartı I2C kablosu ile bağlanabilir
- veya jumper kablolar ile ile 3V3, GND, SDA ve SCL bağlantıları yapılabilir. 

|9-Eksen Ataletsel Ölçüm Birimi| Fonksiyon| Kart pinleri |
| :---     | :---   |   :---  |
| 3.3V     | Güç    | 3.3V    |
| GND      | Toprak |GND      |
| SDA      | I2C Data  | SDA pini |
| SCL      | I2C Clock | SCL pini|
|INT1 | Kesme | herhangi bir GPIO pini |
|INT2  | Kesme | herhangi bir GPIO pini |

## :bookmark_tabs:Lisans Bilgisi 
Lisans bilgileri için [LICENSE](https://github.com/deneyapkart/deneyap-9-eksen-ataletsel-olcum-birimi-arduino-library/blob/master/LICENSE) dosyasını inceleyin.
