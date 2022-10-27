SOURCE = main.c

TARGET = backup.out

PATH_BIN = bin
PATH_OBJ = obj

FLAGS = -O0 -DEBUG -Wall -Werror -Wno-unused-function
AFLAGS = -fsanitize=address -fsanitize=undefined

default: debug

run:
	(cd $(PATH_BIN) ; ./$(TARGET) $(filter-out $@,$(MAKECMDGOALS)))

d:
	(cd $(PATH_BIN) ; ./$(TARGET) ./test ./test_backup)
	
# Структура репозитория.
dir_obj:
	$(if $(wildcard obj), , mkdir obj)

dir_bin:
	$(if $(wildcard bin), , mkdir bin)
# ! Структура репозитория.

# Модуль файловая система.
file.o: file_file.o file_text.o

file_file.o: dir_obj file/file.c file/file.h
	gcc -o $(PATH_OBJ)/file.o $(FLAGS) -g -c file/file.c
file_text.o: dir_obj file/text.c file/text.h
	gcc -o $(PATH_OBJ)/text.o $(FLAGS) -g -c file/text.c
# ! Модуль файловая система.

# Модуль расширяющийся массив.
ext_array.o: ext_array_c.o

ext_array_c.o: dir_obj ext_array/ext_array.c ext_array/ext_array.h
	gcc -o $(PATH_OBJ)/ext_array.o $(FLAGS) -g -c ext_array/ext_array.c
# ! Модуль расширяющийся массив.

# Модуль логов.
logs.o: logs_logs.o

logs_logs.o: dir_obj logs/logs.c logs/logs.h
	gcc -o $(PATH_OBJ)/logs.o $(FLAGS) -g -c logs/logs.c
# ! Модуль логов.

main.o: dir_obj main.c
	gcc -o $(PATH_OBJ)/main.o $(FLAGS) -g -c main.c

backup.o: dir_obj backup.c backup.h
	gcc -o $(PATH_OBJ)/backup.o $(FLAGS) -g -c backup.c

bash.o: dir_obj bash.c bash.h
	gcc -o $(PATH_OBJ)/bash.o $(FLAGS) -g -c bash.c

debug: dir_bin main.o backup.o file.o logs.o ext_array.o bash.o
	gcc -o $(PATH_BIN)/$(TARGET) $(PATH_OBJ)/main.o $(PATH_OBJ)/backup.o \
	$(PATH_OBJ)/file.o $(PATH_OBJ)/logs.o $(PATH_OBJ)/ext_array.o $(PATH_OBJ)/bash.o

clean_obj:
	rm -r $(PATH_OBJ)

clean_bin:
	rm $(PATH_BIN)/$(TARGET)

update_file:
	cp -r ../libs/file ./

update_logs:
	cp -r ../libs/logs ./

update_ext_array:
	cp -r ../libs/ext_array ./