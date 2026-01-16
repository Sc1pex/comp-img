.PHONY: all clean

all: compare_images compare_images3 generate_image

generate_image: generate_image.c
	gcc generate_image.c -o generate_image

compare_images3: compare_images.c
	gcc compare_images.c -o compare_images3 -lpthread -O3

compare_images: compare_images.c
	gcc compare_images.c -o compare_images -lpthread

clean:
	rm -f compare_images compare_images3 generate_image i1.bin i2.bin
