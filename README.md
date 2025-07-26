# GasGuard 🔥🛡️

**GasGuard** adalah sistem monitoring kebocoran gas berbasis **ESP32**, yang menggabungkan sensor gas **MQ-2**, sensor suhu & kelembapan **DHT22**, serta integrasi dengan **Blynk IoT** untuk pemantauan jarak jauh melalui internet. Sistem ini dilengkapi dengan **LCD I2C**, **relay**, dan **buzzer** sebagai notifikasi lokal.

---

## 📦 Fitur

- Pemantauan kualitas udara (tingkat gas)
- Pemantauan suhu dan kelembapan
- Kontrol relay (otomatis/manual) via Blynk
- Notifikasi peringatan kebocoran gas
- LCD 16x2 untuk tampilan waktu dan data sensor
- WiFi auto-reconnect
- Multi-tasking menggunakan FreeRTOS

---

## 🛠️ Hardware yang Dibutuhkan

| Komponen        | Deskripsi              |
|----------------|------------------------|
| ESP32          | Mikrokontroler utama   |
| Sensor MQ-2    | Deteksi kebocoran gas  |
| Sensor DHT22   | Sensor suhu & kelembapan |
| LCD I2C 16x2   | Tampilan data lokal    |
| Relay + Buzzer | Output peringatan      |
| Breadboard, kabel jumper, dll |

---

## 📡 Integrasi Blynk

Proyek ini menggunakan [Blynk IoT](https://blynk.io/) untuk pemantauan jarak jauh.

### Virtual Pin Mapping:
| Fungsi           | Virtual Pin |
|------------------|-------------|
| Relay control    | V0          |
| Gas level        | V1          |
| Temperature      | V2          |
| Humidity         | V3          |

> Pastikan untuk mengganti `BLYNK_AUTH_TOKEN` dengan milikmu sendiri di Blynk Dashboard.

---

## ⚙️ Cara Kerja

1. ESP32 membaca nilai sensor setiap detik.
2. Jika nilai gas melebihi ambang batas (`Threshold = 30%`), maka:
   - Relay ON
   - Buzzer aktif
   - Notifikasi dikirim via Blynk
3. LCD menampilkan waktu, suhu, kelembapan, dan nilai gas secara bergantian.
4. Status dan data dikirim ke Blynk.

---

## 🚀 Instalasi & Upload

1. Install Library berikut melalui Arduino IDE:
   - `BlynkSimpleEsp32`
   - `DHT sensor library`
   - `LiquidCrystal_I2C`
2. Upload `GasGuard.ino` ke ESP32
3. Buka serial monitor pada baudrate `115200`
4. Nikmati sistem GasGuard aktif!

---

## 📁 Struktur File

```
GasGuard
└── GasGuard.ino
```

---
## 🧠 Catatan Tambahan

- Gunakan sumber daya 5V yang stabil untuk sensor dan modul.
- Pastikan koneksi WiFi stabil agar data dapat dikirim ke Blynk secara konsisten.
- Proyek ini memanfaatkan fitur **FreeRTOS** pada ESP32 untuk multitasking (sensor dan waktu).
---

## 🧑‍💻 Author
Created by [Sanzy23](https://github.com/Sanzy23)  
Feel free to fork, star, or contribute!

---
## 📜 Lisensi
Proyek ini bebas digunakan untuk tujuan edukasi. Harap tetap cantumkan atribusi.
```
