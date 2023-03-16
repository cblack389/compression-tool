# compression-tool

The H_encoder class hass all the required functionality for the encoding and decoding using the Huffman technique.
We need to represent a node of the Huffman tree, so we use struct Node for that, which has pointers to children, the char corresponding to the
given node, as well as its frequency.

The H_encoder class has a pointer to one such Node, which is the root of the Huffman tree, as well as a frequency array for all the values a byte can take,
which is 2^8 = 256. We have the following methods:

build_freq_array - this iterates through the input file(which is an uncompressed file - this method is only used when compressing) and counts the number of
instances of each byte.

generate_huffman_tree - uses the internal frequency array to build the corresponding Huffman tree and updates the root member variable accordingly. To build the 
Huffman tree I use the method described in Cormen, with a priority queue.

print_codes - prints the Huffman codes using the currently stored Huffman tree. This should be called after generating the Huffman tree.

print_freq - prints the values currently stored in the frequency array.

get_char_code - gets the Huffman code in a string format for the given char(byte). This is used by the compress function.

compress - this compresses the given input file using the already built Huffman tree. The compressed file has a header that has the following fields:
nr_of_chars - 1 byte - stores the number of found chars in the input file 
nr_of_0s - 1 byte - we need to know when to stop processing bits when we decompress, so the last byte of the compressed file will have a padding of 0s. The number
of 0s in the padding is stored in this field.
char - frequency pairs - 1 byte + 4 bytes = 5 bytes - here we store the frequency array.
The actual compressed file contents start at offset 1 + 1 + nr_of_chars * 5 bytes.
This method uses bitwise operators to build the bytes that are written into the output compressed file.

build_freq_array_from_header - this reads the header section from a given compressed file and builds the frequency array from it, while also returning nr_of_chars and
nr_of_0s. This method is used in the decompress method.

decompress - takes a compressed file, builds the Huffman tree from its header using the previous method and generate_huffman_tree and decodes the compressed file using the
Huffman tree.
