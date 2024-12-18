#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include <immintrin.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef char i8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef int i32;
typedef unsigned u32;
typedef unsigned long u64;

#define PRINT_ERROR(cstring) write(STDERR_FILENO, cstring, sizeof(cstring) - 1)

#pragma pack(1)
struct bmp_header
{
	// Note: header
	i8  signature[2]; // should equal to "BM"
	u32 file_size;
	u32 unused_0;
	u32 data_offset;

	// Note: info header
	u32 info_header_size;
	u32 width; // in px
	u32 height; // in px
	u16 number_of_planes; // should be 1
	u16 bit_per_pixel; // 1, 4, 8, 16, 24 or 32
	u32 compression_type; // should be 0
	u32 compressed_image_size; // should be 0
	// Note: there are more stuff there but it is not important here
};

struct file_content
{
	i8*   data;
	u32   size;
};

struct file_content   read_entire_file(char* filename)
{
	char* file_data = 0;
	unsigned long	file_size = 0;
	int input_file_fd = open(filename, O_RDONLY);
	if (input_file_fd >= 0)
	{
		struct stat input_file_stat = {0};
		stat(filename, &input_file_stat);
		file_size = input_file_stat.st_size;
		file_data = mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, input_file_fd, 0);
		close(input_file_fd);
	}
	return (struct file_content){file_data, file_size};
}

// u8 is_header(u32 *start, struct bmp_header *header)
// {
// 	u32* tmp = start + header->width;
// 	int i = -1;
// 	while (++i < 6)
// 	{
// 		if (*tmp != *start)
// 			return (0);
// 		tmp += header->width;
// 	}
// 	i = -1;
// 	tmp ++;
// 	while (++i < 6)
// 	{
// 		if (*tmp != *start)
// 			return (0);
// 		tmp ++;
// 	}
// 	return (1);
// }
u32* is_header(u32 *start, struct bmp_header *header)
{
	u32* tmp = start;
	int down = 0;
	int left= 0;
	int right = 0;
	while (*tmp == *start)
		{
			left++;
			// *tmp = 0x00ff00ff;
			tmp--;
		}
	tmp = start;
	while (*(++tmp) == *start)
		right++;
	if (right + left == 6)
	{
		tmp = start - left;
		while (*tmp == *start)
		{
			*tmp = 0x00ff00ff;
			if (down == 7)
				return (tmp);
			down++;
			tmp -= header->width;
		}
	}
	return (NULL);
}


u8 *find_header(i8 *start, struct bmp_header *header)
{
	long size = header->width * header->height;
	long i = -1;
	u8* current = (u8*)start;
	while (++i < size)
	{
		if ((current[0] == (u8)127) && 
		(current[1] == (u8)188) &&
		(current[2] == (u8)217) && 
		is_header((u32 *)current, header))
			return (current);
		current = current + 24;
	}
	return (NULL);
}



void extract_string(struct file_content* content, struct bmp_header *header)
{
	u8 *current;
	current = find_header(&(content->data[header->data_offset]), header);
	if (!current)
	{
		PRINT_ERROR("No header found!\n");
		exit(1);
	}
	current = current + 7 * header->width * 4 + 7 * 4;
	size_t len = (size_t)current[0] + (size_t)current[2];
	current -= 2 * header->width * 4 + 20;
	u32* px = (u32*)current;
	size_t column = 0;
	while (len > 0)
	{
		if (len > 3)
		{
			write(STDOUT_FILENO, px, 3);
			len -= 3;
		}
		else
		{
			write(STDOUT_FILENO, px, len);
			len = 0;
		}
		column++;
		if (column == 6)
		{
			px -= header->width + 5;
			column = 0;
		}
		else
			px++;
	}
	// file error testing
	int fd = open("test.bmp", O_RDWR | O_CREAT | O_TRUNC, 0644);
	printf("%d\n", fd);
	if (fd > 0)
	{
		write (fd, content->data, content->size);
		close (fd);
	}
	else
		PRINT_ERROR("opend failed!");
	return ;
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		PRINT_ERROR("Usage: decode <input_filename>\n");
		return 1;
	}
	struct file_content file_content = read_entire_file(argv[1]);
	if (file_content.data == NULL)
	{
		PRINT_ERROR("Failed to read file\n");
		return 1;
	}
	struct bmp_header* header = (struct bmp_header*) file_content.data;
	// printf("signature: %.2s\nfile_size: %u\ndata_offset: %u\ninfo_header_size: %u\nwidth: %u\nheight: %u\nplanes: %i\nbit_per_px: %i\ncompression_type: %u\ncompression_size: %u\n", header->signature, header->file_size, header->data_offset, header->info_header_size, header->width, header->height, header->number_of_planes, header->bit_per_pixel, header->compression_type, header->compressed_image_size);

	extract_string(&file_content, header);
	return 0;
}


