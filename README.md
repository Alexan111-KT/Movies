# ApplicationMovies
Лёгкое настольное приложение для учета выш любимых фильмов. Работает на Qt 6 и C++20, поддерживает просмотр, редактирование, добавление, удаление, импорт и экспорт фильмов.

## Требования

- **Qt** 6.9.0 MinGW 64-bit *(или любая Qt ≥ 6.8.3)*
- **CMake** ≥ 3.16
- **Компилятор с поддержкой C++20**:
  - MinGW (GCC 11+), поставляется с Qt 6.9.0
  - MSVC v143 (Visual Studio 2022) — Windows
  - GCC ≥ 11 или Clang ≥ 15 — Linux/macOS
- **CMake Generator**: рекомендуется Ninja

---

## Сборка через Qt Creator

### 1. Открытие проекта
- Запустите Qt Creator.
- Выберите **Файл → Открыть файл или проект…** и укажите `CMakeLists.txt` в корне проекта.

### 2. Настройка Kit-а
- В списке "Текущий проект" выберите: `Desktop Qt 6.9.0 MinGW 64-bit`.
- Перейдите в **Сборка и запуск → Сборка**, проверьте:
  - `Build type`: `Debug` (для отладки) или `Release` (финальная сборка)
  - `CMake generator`: `Ninja`
  - `CMAKE_PREFIX_PATH`: `C:/Qt/6.9.0/mingw_64/lib/cmake`

### 3. Параметры окружения
- В **Projects → Build Environment** добавьте:
  ```sh
  PATH=C:/Qt/Tools/mingw1310_64/bin;...
