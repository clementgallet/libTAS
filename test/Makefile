all: hooklib3 hooklib2 hooklib1 hookmain

hookmain: hookmain.c
	gcc -g -o hookmain hookmain.c -lhooklib1 -ldl -Lhooklib1 -Wl,-rpath,hooklib1:hooklib2

hooklib1: hooklib1.c
	mkdir -p hooklib1
	gcc -g -o hooklib1/libhooklib1.so hooklib1.c -shared

hooklib2: hooklib2.c hooklib3.c
	mkdir -p hooklib2
	gcc -g -o hooklib2/libhooklib2.so hooklib2.c -lhooklib3 -shared -Lhooklib3 -Wl,-rpath,hooklib3

hooklib3: hooklib3.c
	mkdir -p hooklib3
	gcc -g -o hooklib3/libhooklib3.so hooklib3.c -shared

clean:
	rm -f hookmain hooklib1/libhooklib1.so hooklib2/libhooklib2.so hooklib3/libhooklib3.so
	rmdir hooklib1 hooklib2 hooklib3 2>/dev/null
