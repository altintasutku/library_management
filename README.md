> ⚠️ **Uyarı**: Bu proje, Algoritma ve Programlama dersinin ilk dönem projesi olarak hazırlanmıştır.

# 🚀 Kütüphane Yönetim Sistemi

## 📝 Proje Açıklaması
Bu proje, C programlama dili kullanılarak geliştirilmiş terminal tabanlı bir kütüphane yönetim sistemidir. SQLite veritabanı ve ncurses kullanıcı arayüzü kütüphanesi ile oluşturulmuştur.

📎 GitHub Repository: [github.com/altintasutku/library_management](https://github.com/altintasutku/library_management)

## 🎯 Özellikler
* 📖 Kitap İşlemleri
  - Kitap ekleme
  - Kitap listeleme
  - ID ile kitap arama
  - Kitap bilgilerini güncelleme
  - Başlığa göre kitap arama

* 👥 Kullanıcı İşlemleri
  - Kitap ödünç alma
  - Kitap iade etme
  - Ödünç alınan kitapları listeleme
  - Kitap arama

## 🛠️ Gereksinimler
* GCC Derleyici
* SQLite3
* NCurses Kütüphanesi
* Make (Derleme için)

## 📦 Kurulum

### 1. Gereksinimleri Yükleyin
Ubuntu/Debian için:
```bash
sudo apt-get update
sudo apt-get install gcc libsqlite3-dev libncurses5-dev make
```

### 2. Projeyi Klonlayın
```bash
git clone https://github.com/altintasutku/library_management.git
cd library_management
```

### 3. Projeyi Derleyin
```bash
make
```

### 4. Programı Çalıştırın
```bash
./build/library_management
```

## 💻 Kullanım

### Ana Menü
* Books: Kitap işlemleri menüsüne erişim
* Users: Kullanıcı işlemleri menüsüne erişim

### Kitap İşlemleri
* Add Book: Yeni kitap ekleme
* List Books: Tüm kitapları listeleme
* Find Book by ID: ID ile kitap arama
* Update Book: Kitap bilgilerini güncelleme
* Search Book by Title: Başlığa göre kitap arama

### Kullanıcı İşlemleri
* Borrow Book: Kitap ödünç alma
* Return Book: Kitap iade etme
* List Borrowed Books: Ödünç alınan kitapları listeleme
* Search Book by Title: Başlığa göre kitap arama

## 🗄️ Veritabanı Yapısı

### Books Tablosu
* ID (Primary Key)
* Title
* Author
* Publisher
* Year

### Loans Tablosu
* ID (Primary Key)
* Book_ID (Foreign Key)
* Borrower_Name
* Borrow_Date
* Return_Date

## 🤝 Katkıda Bulunma
1. Bu projeyi fork edin
2. Yeni bir branch oluşturun (`git checkout -b yenilik/özellik`)
3. Değişikliklerinizi commit edin (`git commit -m 'Yeni özellik eklendi'`)
4. Branch'inizi push edin (`git push origin yenilik/özellik`)
5. Pull Request oluşturun

## 🐛 Hata Bildirimi
Bir hata bulduysanız veya öneriniz varsa, lütfen "Issues" bölümünden yeni bir konu açın.

## 📝 Lisans
Bu proje MIT lisansı altında lisanslanmıştır. Detaylar için [LICENSE](LICENSE) dosyasına bakınız.