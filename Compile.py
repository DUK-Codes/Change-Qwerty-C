import os
import subprocess

def main():
    # Ищем все файлы .c и .cpp в текущей папке
    source_files = []
    for ext in ('.c', '.cpp'):
        source_files.extend(f for f in os.listdir() if f.endswith(ext))
    if not source_files:
        print("Не найдено файлов для компиляции (.c или .cpp).")
        return
    if len(source_files) == 1:
        selected = source_files[0]
    else:
        print("Выберите файл для компиляции:")
        for i, f in enumerate(source_files):
            print(f"{i+1}. {f}")
        try:
            choice = int(input("Введите номер файла: ")) - 1
        except: choice = 0
        selected = source_files[choice]

    # Определяем компилятор по расширению
    if selected.endswith('.cpp'):
        compiler = "g++"
        output = selected[:-4 ] + '.exe'
    else: 
        output = selected[:-2 ] + '.exe'
        compiler = "gcc"
    # Компиляция с флагами для уменьшения размера
    cmd = f"windres resources.rc -O coff -o resources.res && {compiler} -Os -s -DUNICODE -D_UNICODE -mwindows {selected} -o {output} resources.res -lgdi32 -luser32 -lcomctl32 -lole32"
    print(f"Компиляция: {cmd}")
    result = subprocess.run(cmd, shell=True)
    if result.returncode != 0:
        print("Ошибка при компиляции!")
        return
    
    print("Запуск программы...")
    subprocess.run(f"{output}", shell=True)

if __name__ == "__main__":
    while True:
        main()
        input("Нажмите Enter для перекомпиляции или Ctrl+C для выхода.")
