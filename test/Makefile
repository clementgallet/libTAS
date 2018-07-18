all: hooklib3 hooklib2 hooklib1 hookmain

hookmain: hookmain.c
	gcc -g -o hookmain hookmain.c -lhooklib1 -ldl -L.

hooklib1: hooklib1.c
	gcc -g -o libhooklib1.so hooklib1.c -shared

hooklib2: hooklib2.c hooklib3.c
	gcc -g -o libhooklib2.so hooklib2.c -lhooklib3 -shared -L.

hooklib3: hooklib3.c
	gcc -g -o libhooklib3.so hooklib3.c -shared

clean:
	rm hookmain libhooklib1.so libhooklib2.so libhooklib3.so
