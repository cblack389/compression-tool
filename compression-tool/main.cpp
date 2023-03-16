#include <iostream>
#include "H_encoder.h"
#include <fstream>
#include <cstring>

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		std::cout << "Invalid arguments!" << std::endl;
		return 1;
	}

	H_encoder h;

	if (strcmp(argv[1], "-c") == 0)
	{
		h.build_freq_array(argv[2]);
		h.generate_huffman_tree();
		h.print_freq();
		h.print_codes();
		h.compress(argv[3], argv[2]);
	}
	else if (strcmp(argv[1], "-d") == 0)
	{
		h.decompress(argv[3], argv[2]);
		h.print_freq();
		h.print_codes();
	}
	else
	{
		std::cout << "Mode should be \"-c\" for compression or \"-d\" for decompression." << std::endl;
		return 1;
	}

	return 0;
}