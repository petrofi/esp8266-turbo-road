# Kurulum Rehberi

## Donanım

NodeMCU V3 ESP8266 ile 1.3" SH1106 OLED arasındaki bağlantılar:

| OLED | NodeMCU V3 |
|---|---|
| GND | GND |
| VCC | 3V3 |
| SCL | D1 / GPIO5 |
| SDA | D2 / GPIO4 |

## Arduino IDE

1. ESP8266 kart paketini kurun.
2. `NodeMCU 1.0 (ESP-12E Module)` kartını seçin.
3. `Adafruit GFX Library` ve `Adafruit SH110X` kütüphanelerini kurun.
4. `TurboRoad_ESP8266/TurboRoad_ESP8266.ino` dosyasını açın.
5. Doğru COM portunu seçin.
6. **Yükle** butonuna basın.

## Telefon Bağlantısı

Wi‑Fi:

```text
TURBO-ROAD
```

Şifre:

```text
12345678
```

Tarayıcı:

```text
http://192.168.4.1
```

Telefon internetsiz ağ uyarısı verirse bağlantıda kalın.

## Sorun Giderme

### OLED açılmıyor

Kodda:

```cpp
#define OLED_ADDRESS 0x3C
```

değerini bazı ekranlarda:

```cpp
#define OLED_ADDRESS 0x3D
```

olarak değiştirmek gerekebilir.

### Kod derlenmiyor

Arduino IDE'de seçili kartın:

```text
NodeMCU 1.0 (ESP-12E Module)
```

olduğunu kontrol edin.

### `ESP8266WiFi.h: No such file or directory`

ESP8266 kart paketi kurulu değildir veya yanlış kart seçilmiştir.

### Telefonda kontrol sayfası açılmıyor

1. Telefonun `TURBO-ROAD` Wi‑Fi ağına bağlı olduğundan emin olun.
2. Mobil veriyi geçici olarak kapatmayı deneyin.
3. Tarayıcıya tam olarak `192.168.4.1` yazın.
