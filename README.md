## Контроллер периферии для ретро-компьютера Scorpion ZS-256

## Ревизия B:

Подключается к разъему периферии платы компьютера. Позволяет использовать USB клавиатуры, мыши, джойстики, геймпады.

<img src="https://github.com/psk7/scorpion-card-pub/assets/27000982/ea121704-7a0e-4f26-98a5-88fce9f6ac13" width="800" >

## Ревизия С:
                 
Автономный контроллер USB клавиатуры и геймпада. Не привязан к разъему Scorpion и может
быть использован с любым Spectrum совместимым компьютером, использующим стандартную ZX клавиатуру, но
не имеет поддержки Kempston Mouse и возможностей, обеспечиваемых периферийным разъемом ZS-Scorpion за исключением
клавиатуры и Kempston джойстика.

<img src="https://user-images.githubusercontent.com/27000982/211162365-2488c57a-c85b-4176-8bae-3781c7c2bac0.jpg" width="400" >

## Уже реализовано:
- USB-хост: поддержка клавиатур, HID мышей, HID джойстиков/геймпадов, USB хабов
- USB-устройство: подключается к PC, возможно взаимодействие PC со Spectrum
- Безвейтовая ZX-клавиатура с произвольными раскладками, настраиваемыми с PC 
- Reset, Magic с клавиатуры
- Настраиваемые раскладки USB джойстика/геймпада, возможно задействование до 16 кнопок 
- Регенерация синхросмеси. Никаких больше перемычек/разрезов на плате. PAL кодер теперь дает нормальный цветной сигнал
- С доп. платой (в слот шины) USB мышь превращается в Kempston mouse
- Транзитом передает на гребенки: RGB сигнал (с выбором джампером оригинальная/регенерированная синхросмесь), стерео-аудио, моно-аудио, оригинальный видео сигнал, магнитофонный вход/выход, RS-232
- Единая точка подключения питания
- Подключение внешнего джойстика (от Spectrum)
- Настраиваемое отображение кнопок USB джойстика на кнопки ZX клавиатуры/Kempston Joystick
- Управление/индикация на клавиатуре турбой через двухпиновую колодку скорпиона
- Захват centronix в передача на PC
- Проигрыватель tap/tzx с PC через USB порт в Спектрум (`LOAD ""`)
- Транзитная передача с RS232 Скорпиона 
- Поддержка нескольких джойстиков/геймпадов

### Планируется:
- Настройки контроллера (раскладки, мышь, джойстик) через доп. плату нативным приложением для скорпиона

### Недостатки: :smile:
- Нет поддержки PS/2 мышей и клавиатур

## Ревизия А: (Устаревшая)

ВНИМАНИЕ!!! Эта ревизия НЕ РЕКОМЕНДУЕТСЯ К ИЗГОТОВЛЕНИЮ. К сожалению, при проектировании был выбран микроконтроллер не позволяющий реализовать все желаемое по причине нехватки ОЗУ. Готовится к публикации ревизия B, в которой эта проблема устранена.

Файлы для производства удалены из репозитория.

## Содержимое репозитория:
### revB
- cpld - Прошивки микросхем EPM3064
- gerbers - Файлы для производства плат

### revC
- kbd - Прошивка клавиатурной матрицы
- kicad - Проект плат
- atmega - Прошивка управляющего контроллера
