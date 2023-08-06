# WinThreads_Lesson1
Программа представляет собой простой пример использования функций и структур из библиотеки Windows.h для получения информации о процессах и потоках, работающих на компьютере.

Давайте рассмотрим каждую часть кода более подробно:

1. Подключение необходимых заголовочных файлов:\t
#include <iostream> - заголовочный файл для ввода/вывода на консоль.
#include <Windows.h> - заголовочный файл для работы с функциями и структурами операционной системы Windows.
#include <TlHelp32.h> - заголовочный файл для работы с функциями и структурами, связанными с процессами и потоками.
#include <vector> - заголовочный файл для работы с контейнером std::vector.

2. Макрос UNICODE:
#ifdef UNICODE
#define tcout std::wcout
#elif
#define tcout std::cout
#endif 
Этот макрос определяет, какой потоковый объект следует использовать для вывода текста на консоль. Если определен макрос UNICODE, будет использоваться std::wcout (широкий поток вывода). В противном случае будет использоваться std::cout (обычный поток вывода).

3. Класс SmartHandle:
Это класс обертка, который позволяет автоматически закрывать дескрипторы при выходе из области видимости. В данном случае используется для работы с Handle, которые представляют собой идентификаторы, используемые ОС Windows для представления различных объектов, например, процессов или потоков.

4. Структура ProcessINFO:
Эта структура содержит информацию о процессе, включая информацию о процессе (PROCESSENTRY32) и список потоков, связанных с этим процессом (std::vector<THREADENTRY32>).

5. Функция main():
  a. setlocale(LC_ALL, "ru"); - установка локали, чтобы можно было выводить русский текст на консоль.
  b. Создание двух экземпляров класса SmartHandle с помощью вызова функции CreateToolhelp32Snapshot. Эта функция создает снимок текущего состояния системы и возвращает дескриптор снимка. 
    SmartHandle processSnap(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) - создание снимка процессов.
    SmartHandle threadSnap(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0)) - создание снимка потоков.
  c. Проверяется успешность создания снимков процессов и потоков. Если хотя бы один из снимков не был создан, программа завершится с кодом 1.
  d. Созадются два вектора: processInfos для хранения информации о процессах и threads для хранения информации о потоках.
  e. Инициализация структуры THREADENTRY32 te{ sizeof(THREADENTRY32) } - установка размера структуры и инициализация значениями по умолчанию.
  f. Вызов функции Thread32First для получения информации о первом потоке, связанном с созданным снимком потоков. В случае ошибки программа завершится с кодом 2.
  g. Цикл do-while для перебора всех потоков. Информация о каждом потоке добавляется в вектор threads.
  h. Аналогично пункту f и g, получение информации о процессах и добавление ее в вектор processInfos.
  i. Цикл for для вывода информации о каждом процессе и связанных с ним потоках на консоль с помощью объекта tcout (вывод в широком или обычном формате, в зависимости от макроса UNICODE).
