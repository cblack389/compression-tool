#pragma once
#include <cstdint>

//we use this for building the Huffman tree
struct Node
{
	Node* left;
	Node* right;
	int freq;
	char c;

	Node() = default;
	Node(char c_i, int freq_i);
};

//this class has all the required functionality
class H_encoder
{
private:
	int freq_array[256];
	Node* root;

public:
	H_encoder();
	void generate_huffman_tree();
	void print_codes();
	void build_freq_array(const char* filename);
	void print_freq() const;
	void get_char_code(char c, char code[], int index, char temp[]);
	void compress(const char* out_name, const char * in_name);
	int build_freq_array_from_header(const char* in_name, uint8_t &nr_of_0s);
	void decompress(const char* out_name, const char* in_name);
};

