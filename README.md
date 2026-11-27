# fernoCloud ID Bot

Bot Telegram C++ yang canggih untuk pencarian dan pengelolaan database dengan kemampuan manajemen pengguna.

## Daftar Isi

- [Fitur](#fitur)
- [Arsitektur Sistem](#arsitektur-sistem)
- [Prasyarat](#prasyarat)
- [Instalasi](#instalasi)
  - [Linux (Ubuntu/Debian)](#linux-ubuntudebian)
  - [Windows](#windows)
  - [macOS](#macos)
- [Konfigurasi](#konfigurasi)
- [Membangun Proyek](#membangun-proyek)
- [Menjalankan Bot](#menjalankan-bot)
- [Referensi Perintah](#referensi-perintah)
- [Referensi API](#referensi-api)
- [Struktur Proyek](#struktur-proyek)
- [Pengembangan](#pengembangan)
- [Pemecahan Masalah](#pemecahan-masalah)
- [Lisensi](#lisensi)
- [Kontak](#kontak)

## Fitur

- **Kemampuan Pencarian Lanjutan**:

  - Pencarian kata kunci di beberapa file database
  - Menghasilkan daftar website berdasarkan TLD (Top Level Domain)
  - Pencarian multi-thread untuk performa optimal
  - Pemfilteran dan pemformatan hasil
- **Sistem Manajemen Pengguna**:

  - Pengguna premium dengan akses tanpa batas
  - Pengguna gratis dengan akses berbasis kredit
  - Hierarki admin (Master, Tingkat Pertama, Tingkat Kedua)
  - Pelacakan aktivitas pengguna
- **Kontrol Admin**:

  - Tambah/hapus pengguna dan admin
  - Perpanjang periode akses pengguna
  - Tutup/buka perintah untuk pengguna non-admin
  - Lihat statistik pengguna secara detail
- **Optimasi Performa**:

  - Operasi file asinkron
  - Thread pooling untuk pencarian bersamaan
  - Sistem caching database
  - Pembatasan rate untuk mencegah penyalahgunaan
- **Fitur Keandalan**:

  - Penanganan kesalahan dan pemulihan
  - Backup database otomatis
  - Penanganan sinyal untuk shutdown yang bersih
  - Sistem logging komprehensif

## Arsitektur Sistem

InfernoCloud ID Bot dibangun dengan arsitektur modular:

1. **Komponen Inti**:

   - `DatabaseManager`: Menangani operasi database dan caching file
   - `UserManager`: Mengelola autentikasi dan izin pengguna
   - `CommandHandlers`: Memproses perintah Telegram
   - `Logger`: Menyediakan fungsionalitas logging
2. **Model Data**:

   - `EnhancedUser`: Merepresentasikan pengguna premium/terdaftar
   - `UnregisteredUser`: Merepresentasikan pengguna gratis dengan kredit terbatas
3. **Layanan Utilitas**:

   - Pencarian dan pemrosesan file
   - Penanganan tanggal/waktu
   - Sanitasi string
   - Manajemen konfigurasi

## Prasyarat

Untuk membangun dan menjalankan bot ini, Anda memerlukan dependensi berikut:

- **Kompiler yang kompatibel dengan C++17**:
  - GCC 8+ (Linux)
  - Clang 6+ (macOS)
  - MSVC 2019+ (Windows)
- **CMake 3.15+**
- **Boost 1.67+** (komponen sistem)
- **OpenSSL**
- **CURL**
- **TgBot-cpp** (wrapper API Bot Telegram untuk C++)
- **nlohmann/json** (disertakan sebagai dependensi)

## Instalasi

### Linux (Ubuntu/Debian)

1. Instal paket yang diperlukan:

```bash
sudo apt update
sudo apt install -y g++ make cmake binutils git \
    libboost-all-dev libssl-dev libcurl4-openssl-dev \
    zlib1g-dev
```

2. Clone repositori:

```bash
git clone https://github.com/IkuzaDev/InfernoCloud-bot.git
cd infernocloud-bot
```

3. Buat direktori build dan kompilasi:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Windows

#### Opsi 1: Menggunakan Visual Studio dan vcpkg

1. Instal [Visual Studio](https://visualstudio.microsoft.com/downloads/) dengan workload pengembangan C++
2. Instal [vcpkg](https://github.com/microsoft/vcpkg):

```cmd
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
```

3. Instal dependensi:

```cmd
vcpkg install boost:x64-windows openssl:x64-windows curl:x64-windows zlib:x64-windows
```

4. Clone repositori:

```cmd
git clone https://github.com/IkuzaDev/InfernoCloud-bot.git
cd infernocloud-bot
```

5. Hasilkan solusi Visual Studio:

```cmd
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
```

6. Buka solusi yang dihasilkan di Visual Studio dan build proyek

#### Opsi 2: Menggunakan MinGW/MSYS2

1. Instal [MSYS2](https://www.msys2.org/)
2. Buka terminal MSYS2 dan instal dependensi:

```bash
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-boost mingw-w64-x86_64-openssl mingw-w64-x86_64-curl mingw-w64-x86_64-cmake mingw-w64-x86_64-make git
```

3. Clone dan build:

```bash
git clone https://github.com/IkuzaDev/InfernoCloud-bot.git
cd infernocloud-bot
mkdir build && cd build
cmake -G "MSYS Makefiles" ..
mingw32-make
```

### macOS

1. Instal [Homebrew](https://brew.sh/):

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

2. Instal dependensi:

```bash
brew install cmake boost openssl curl
```

3. Clone dan build:

```bash
git clone https://github.com/IkuzaDev/InfernoCloud-bot.git
cd infernocloud-bot
mkdir build && cd build
cmake .. -DOPENSSL_ROOT_DIR=$(brew --prefix openssl)
make -j$(sysctl -n hw.ncpu)
```

## Konfigurasi

Sebelum menjalankan bot, Anda perlu mengkonfigurasinya dengan mengedit `include/Config.h`:

```cpp
// Token API Bot - Dapatkan dari @BotFather di Telegram
inline const std::string TOKEN = "TOKEN_BOT_ANDA_DISINI";

// ID Admin - ID pengguna Telegram untuk administrator
inline const std::unordered_set<int64_t> MASTER_ADMIN_IDS = {ID_MASTER_ADMIN_ANDA};
inline const std::unordered_set<int64_t> FIRST_ADMIN_IDS = {ID_ADMIN_PERTAMA};
inline const std::unordered_set<int64_t> SECOND_ADMIN_IDS = {ID_ADMIN_KEDUA};

// Informasi kontak admin
inline const std::string ADMIN_CONTACT = "@username_admin_anda";

// Path direktori untuk file database
inline const std::string DIRECTORY_PATH = "/path/ke/direktori/database/anda/";

// Pengaturan performa - sesuaikan berdasarkan kapasitas server Anda
inline const int WORKERS = 60;                           // Jumlah thread worker
inline const int MAX_CONCURRENT_REQUESTS = 3;            // Maks permintaan bersamaan per pengguna
inline const int FREE_CREDIT_DAILY = 5;                  // Kredit gratis untuk pengguna tidak terdaftar
inline const size_t MAX_FILE_SIZE = 100 * 1024 * 1024;   // Ukuran file maksimum untuk pencarian (100MB)
```

### Parameter Konfigurasi Penting

| Parameter                   | Deskripsi                                    | Nilai yang Direkomendasikan |
| --------------------------- | -------------------------------------------- | --------------------------- |
| `WORKERS`                 | Jumlah thread worker                         | CPU cores × 1.5            |
| `TOKEN_RESET_HOURS`       | Jam sebelum reset token                      | 24                          |
| `REQUEST_TIMEOUT`         | Timeout permintaan dalam detik               | 300                         |
| `MAX_SEARCH_RESULTS`      | Hasil pencarian maksimum                     | 1000-5000                   |
| `MAX_WEBLIST_RESULTS`     | Hasil weblist maksimum                       | 10000-50000                 |
| `MAX_CONCURRENT_REQUESTS` | Maks permintaan bersamaan per pengguna       | 2-5                         |
| `FREE_CREDIT_DAILY`       | Kredit gratis untuk pengguna tidak terdaftar | 3-10                        |
| `SEARCH_BATCH_SIZE`       | Ukuran batch untuk operasi pencarian         | 50-100                      |

## Membangun Proyek

Setelah menginstal dependensi dan mengkonfigurasi bot:

```bash
# Buat direktori build
mkdir -p build
cd build

# Konfigurasi dengan CMake
cmake ..

# Build
cmake --build . --config Release
```

## Menjalankan Bot

### Linux/macOS

```bash
cd build
./inferno_bot
```

Untuk menjalankan sebagai layanan, buat file layanan systemd:

```bash
sudo nano /etc/systemd/system/infernobot.service
```

Tambahkan konten berikut:

```ini
[Unit]
Description=InfernoCloud ID Bot
After=network.target

[Service]
Type=simple
User=username_anda
WorkingDirectory=/path/ke/infernocloud-bot/build
ExecStart=/path/ke/infernocloud-bot/build/inferno_bot
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Aktifkan dan mulai layanan:

```bash
sudo systemctl enable infernobot
sudo systemctl start infernobot
```

### Windows

Jalankan executable dari direktori build:

```cmd
cd build\Release
inferno_bot.exe
```

Untuk menjalankan sebagai layanan Windows, gunakan [NSSM](https://nssm.cc/):

```cmd
nssm install InfernoBot
```

Konfigurasikan layanan dengan path ke executable Anda.

## Referensi Perintah

### Perintah Pengguna

| Perintah       | Deskripsi                          | Penggunaan               |
| -------------- | ---------------------------------- | ------------------------ |
| `/start`     | Mulai bot                          | `/start`               |
| `/menu`      | Tampilkan menu utama               | `/menu`                |
| `/mytokens`  | Periksa sisa kredit                | `/mytokens`            |
| `/search`    | Cari kata kunci di database        | `/search <kata_kunci>` |
| `/weblist`   | Daftar website dengan TLD tertentu | `/weblist <tld>`       |
| `/usertotal` | Tampilkan jumlah pengguna aktif    | `/usertotal`           |
| `/dbtotal`   | Tampilkan jumlah database          | `/dbtotal`             |

### Perintah Admin

| Perintah      | Deskripsi                 | Penggunaan                       |
| ------------- | ------------------------- | -------------------------------- |
| `/add`      | Tambah pengguna baru      | `/add <id> <durasi> <unit>`    |
| `/dell`     | Hapus pengguna            | `/dell <id>`                   |
| `/extend`   | Perpanjang akses pengguna | `/extend <id> <durasi> <unit>` |
| `/listuser` | Daftar semua pengguna     | `/listuser`                    |

### Perintah Master Admin

| Perintah       | Deskripsi                      | Penggunaan                  |
| -------------- | ------------------------------ | --------------------------- |
| `/add_admin` | Tambah admin                   | `/add_admin <id> <level>` |
| `/ban_admin` | Ban admin                      | `/ban_admin <id>`         |
| `/close`     | Tutup perintah untuk non-admin | `/close <perintah>`       |
| `/closed`    | Daftar perintah tertutup       | `/closed`                 |

## Referensi API

### API Manajemen Pengguna

Bot ini menyediakan sistem manajemen pengguna yang komprehensif:

#### Tipe Pengguna

1. **EnhancedUser**: Pengguna premium dengan akses tanpa batas

   - Properti: userId, username, firstName, lastName, expiryDate, dll.
   - Metode: isActive(), canMakeRequest(), dll.
2. **UnregisteredUser**: Pengguna gratis dengan kredit terbatas

   - Properti: userId, username, tokens, lastReset, dll.
   - Metode: checkAndUpdateTokens(), canMakeRequest(), dll.

#### Operasi Pengguna

- Membuat pengguna baru
- Memperpanjang periode akses pengguna
- Memeriksa status dan izin pengguna
- Melacak aktivitas dan penggunaan pengguna

### API Pencarian

Bot ini menyediakan kemampuan pencarian yang kuat:

- **Pencarian Kata Kunci**: Mencari kata kunci tertentu di file database
- **Pencarian TLD**: Menghasilkan daftar website dengan TLD tertentu
- **Pemfilteran**: Memfilter hasil berdasarkan kriteria yang telah ditentukan
- **Pemformatan**: Memformat hasil untuk pembacaan dan analisis yang mudah

## Struktur Proyek

```
infernocloud-bot/
├── CMakeLists.txt           # Konfigurasi build CMake
├── README.md                # Dokumentasi ini
├── include/                 # File header
│   ├── Config.h             # Pengaturan konfigurasi
│   ├── CommandHandlers.h    # Handler perintah Telegram
│   ├── DatabaseManager.h    # Operasi database
│   ├── Logger.h             # Sistem logging
│   ├── User.h               # Kelas pengguna
│   └── UserManager.h        # Manajemen pengguna
├── src/                     # File sumber
│   ├── CommandHandlers.cpp  # Implementasi handler perintah
│   ├── DatabaseManager.cpp  # Implementasi operasi database
│   ├── Logger.cpp           # Implementasi logger
│   ├── main.cpp             # Titik masuk utama
│   ├── User.cpp             # Implementasi kelas pengguna
│   ├── UserManager.cpp      # Implementasi manajemen pengguna
│   └── Utils.cpp            # Fungsi utilitas
└── docs/                    # Dokumentasi tambahan
```

## Pengembangan

### Menambahkan Perintah Baru

Untuk menambahkan perintah baru ke bot:

1. Definisikan fungsi handler perintah di `CommandHandlers.h`:

```cpp
static void perintahBaru(const TgBot::Message::Ptr& message, TgBot::Bot& bot, const std::vector<std::string>& args);
```

2. Implementasikan fungsi di `CommandHandlers.cpp`
3. Daftarkan perintah di fungsi `registerHandlers`:

```cpp
bot.getEvents().onCommand("perintahbaru", [&bot](TgBot::Message::Ptr message) {
    std::string args = message->text;
    args = args.substr(12); // Hapus "/perintahbaru "
    std::vector<std::string> argList;
    std::istringstream iss(args);
    for (std::string s; iss >> s;) {
        argList.push_back(s);
    }
    perintahBaru(message, bot, argList);
});
```

### Memperluas Manajemen Pengguna

Untuk menambahkan properti atau fungsionalitas pengguna baru:

1. Modifikasi kelas `EnhancedUser` atau `UnregisteredUser` di `User.h`
2. Perbarui implementasi yang sesuai di `User.cpp`
3. Perbarui metode serialisasi/deserialisasi JSON untuk menyertakan properti baru

## Pemecahan Masalah

### Masalah Umum

1. **Bot tidak merespons**:

   - Periksa apakah bot berjalan
   - Verifikasi token bot di `Config.h`
   - Periksa konektivitas jaringan
2. **Error build**:

   - Pastikan semua dependensi terinstal
   - Periksa versi kompiler (harus mendukung C++17)
   - Verifikasi versi CMake (3.15+)
3. **Pencarian tidak berfungsi**:

   - Periksa path direktori database di `Config.h`
   - Pastikan file database dapat dibaca
   - Verifikasi izin file
4. **Masalah performa**:

   - Sesuaikan pengaturan `WORKERS` berdasarkan CPU Anda
   - Kurangi `MAX_CONCURRENT_REQUESTS` jika server kelebihan beban
   - Pertimbangkan untuk meningkatkan `SEARCH_BATCH_SIZE` untuk pencarian lebih cepat

### Logging

Bot ini menggunakan sistem logging yang komprehensif. Log ditulis ke `infernobot.log` di direktori kerja.

Level log dapat disesuaikan di `Config.h`:

```cpp
inline const LogLevel LOG_LEVEL = LogLevel::INFO;
```

Level log yang tersedia:

- `DEBUG`: Informasi debugging detail
- `INFO`: Pesan informasi umum
- `WARNING`: Pesan peringatan
- `ERROR`: Pesan error
- `CRITICAL`: Error kritis

## Lisensi

Proyek ini adalah perangkat lunak proprietary. Hak cipta dilindungi.

## Kontak

Untuk informasi lebih lanjut, hubungi IkuzaDev.

Untuk dukungan teknis atau permintaan fitur, silakan buka issue di repositori proyek.
