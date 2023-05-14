all:
		gcc -o t2 task2.c
clean:
		rm -f stat.txt
		rm -f my_file.txt.lck