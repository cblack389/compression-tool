#include "H_encoder.h"
#include <iostream>
#include <queue>
#include <vector>
#include <fstream>
#include <cstdint>

Node::Node(char c_i, int freq_i) : c{c_i}, freq{freq_i}, left{nullptr}, right{nullptr}
{
}

bool compare(Node *n1, Node *n2)
{
	return (n1->freq > n2->freq);
}

H_encoder::H_encoder()
{
	for (int i = 0; i < 256; i++)
		freq_array[i] = 0;

	root = nullptr;
}

//this generates the huffman tree using the
//internal frequency array
void H_encoder::generate_huffman_tree()
{
	//we create nodes for each char that was found in input file, as we need them for our priority queue
	std::vector<Node*>initial_nodes;

	int cnt{ 0 };
	for (int i = 0; i < 256; i++)
	{
		if (freq_array[i] != 0)
		{
			cnt++;
			//create node for this char and insert it
			Node* n = new Node(static_cast<char>(i), freq_array[i]);
			initial_nodes.push_back(n);
		}
	}

	std::priority_queue<Node*, std::vector<Node*>, bool(*) (Node*, Node*)> q(compare);

	for (unsigned j = 0; j < static_cast<unsigned>(initial_nodes.size()); j++)
	{
		q.push(initial_nodes.at(j));
	}

	if (cnt == 1)
	{
		root = q.top();
		return;
	}

	for (int j = 0; j < cnt - 1; j++)
	{
		Node* z = new Node();
		Node* x = q.top();
		q.pop();
		Node* y = q.top();
		q.pop();

		z->left = x;
		z->right = y;
		z->freq = x->freq + y->freq;
		
		q.push(z);
	}
	
	root = q.top();
	q.pop();
}

void print_codes_internal(Node *node, char arr[], int index)
{
	if (node == nullptr)
	{
		std::cout << "Root is null - tree was not generated." << std::endl;
		return;
	}

	if (node->left != nullptr)
	{
		arr[index] = '0';
		print_codes_internal(node->left, arr, index + 1);
	}

	if (node->right != nullptr)
	{
		arr[index] = '1';
		print_codes_internal(node->right, arr, index + 1);
	}

	//if both both left and right are null then we have a leaf
	if (node->left == nullptr && node->right == nullptr)
	{
		arr[index] = 0;
		std::cout << node->c << ": " << arr << '\n';
	}
}

//prints the generated huffman codes
void H_encoder::print_codes()
{
	std::cout << "\nThe generated codes are:\n";
	char arr[255];
	if (root->left == nullptr && root->right == nullptr)
	{
		std::cout << root->c << ": " << '1' << std::endl;
		return;
	}
		
	print_codes_internal(root, arr, 0);
}

//this computes the frequency of the characters in our input file
//and updates the frequency array
void H_encoder::build_freq_array(const char* filename)
{
	std::ifstream fin(filename);

	//error checking
	if (!fin.is_open())
	{
		std::cout << "Could not open file \"" << filename << "\"." << std::endl;
		return;
	}

	//read the file in blocks of size 512
	char buf[512];

	//keep reading until eof and increase the freq
	while (!fin.eof())
	{
		fin.read(buf, 512);

		for (int i = 0; i < fin.gcount(); i++)
		{
			freq_array[buf[i]]++;
		}
	}

	fin.close();
}

//prints the values for each char
//useful for debugging purposes
void H_encoder::print_freq() const
{
	std::cout << "\nThe character frequencies are:\n";
	for (int i = 0; i < 256; i++)
	{
		if(freq_array[i] > 0)
			std::cout << static_cast<char>(i)<< ": " << freq_array[i] << '\n';
	}
}

void get_char_code_internal(Node* node, char c, char code[], int index, char temp[])
{
	if (node == nullptr)
	{
		std::cout << "Node is null - tree not generated." << std::endl;
		return;
	}

	//if both both left and right are null then we have a leaf
	if (node->left == nullptr && node->right == nullptr)
	{
		if (node->c == c)
		{
			temp[index] = 0;
			for (int i = 0; i < index; i++)
			{
				code[i] = temp[i];
			}
			code[index] = 0;

			return;
		}
		else
		{
			return;
		}
	}

	if (node->left != nullptr)
	{
		temp[index] = '0';
		get_char_code_internal(node->left, c, code, index + 1, temp);
	}

	if (node->right != nullptr)
	{
		temp[index] = '1';
		get_char_code_internal(node->right, c, code, index + 1, temp);
	}
}

//get the code for the given byte
void H_encoder::get_char_code(char c, char code[], int index, char temp[])
{
	if (root->left == nullptr && root->right == nullptr)
	{
		code[0] = '1';
		return;
	}
	get_char_code_internal(root, c, code, 0, temp);
}

//the output file has the following format:
//header: nr_of_chars - 1 byte
//		  nr_of_0's in last byte - 1 byte (we need this field so we know where to stop when we decompress)
//		  char - freq pairs - 1 + 4 = 5 bytes
//file contents: starts at offset (1 + 1 + nr_of_chars * 5) bytes
void H_encoder::compress(const char* out_name, const char *in_name)
{
	//open files and do some error checking
	std::ofstream fout;
	fout.open(out_name, std::ios::out | std::ios::binary);

	if (!fout.is_open())
	{
		std::cout << "Could not open output file!" << std::endl;
		return;
	}

	std::ifstream fin;
	fin.open(in_name);

	if (!fin.is_open())
	{
		std::cout << "Could not open input file!" << std::endl;
		return;
	}

	//first we write the header
	//we need to compute the nr_of_chars and also to prepare the required chars and their freqs for writing
	uint8_t nr_of_chars{ 0 }, nr_of_0s{ 0 };
	char chars[256], freqs[1024];
	int freqs_index{ 0 }, chars_index{ 0 };

	for (int i = 0; i < 256; i++)
	{
		if (freq_array[i] > 0)
		{
			nr_of_chars++;
			chars[chars_index++] = static_cast<char>(i);
	
			char* tmp = reinterpret_cast<char*>(&freq_array[i]);

			for (int j = 0; j < 4; j++)
				freqs[freqs_index++] = *(tmp + j);
		}
	}

	//build the buffer that will be used to write the header
	char buff[2000];
	int buff_index{ 2 };
	buff[0] = static_cast<char>(nr_of_chars);
	buff[1] = static_cast<char>(nr_of_0s);

	freqs_index = 0;
	for (int i = 0; i < nr_of_chars; i++)
	{
		buff[buff_index++] = chars[i];

		for (int j = 0; j < 4; j++)
			buff[buff_index++] = freqs[freqs_index++];
	}

	fout.write(buff, buff_index);

	//after the header we write the encoded file contents
	char in_buff[512];
	int code_index = 0, byte_index = 0;
	uint8_t byte = 0, mask1 = 1, mask2 = 0;

	//read from the input file and write into output
	while (!fin.eof())
	{
		fin.read(in_buff, 512);

		//for all the characters that were read, we get thier codes and write them into output
		for (int i = 0; i < fin.gcount(); i++)
		{
			char temp[512], code[512];

			get_char_code(in_buff[i], code, 0, temp);

			//we iterate through the code string and build the next byte to be written 
			while (code[code_index] != 0)
			{
				if (code[code_index] == '1')
					byte = byte | (mask1 << (7 - byte_index));
				else if(code[code_index] == '0')
					byte = byte | (mask2 << (7 - byte_index));

				code_index++;
				byte_index++;

				//if a full byte has been built then we can write it and reset it and its index
				if (byte_index == 8)
				{
					fout.write(reinterpret_cast<char*>(&byte), 1);
					byte = 0;
					byte_index = 0;
				}
			}

			//we need to reset the code index so we can iterate through the next char's code
			code_index = 0;
		}
	}

	//the last byte has nr_of_0s bits of 0 appended 
	fout.write(reinterpret_cast<char*>(&byte), 1);
	nr_of_0s = 8 - byte_index;
	fout.seekp(1);

	//now we can update the nr_of_0s field in the header
	fout.write(reinterpret_cast<char*>(&nr_of_0s), 1);

	fout.close();
	fin.close();
}

//build the frequnecy array from the header and return the offset
//of byte where the file contents start
int H_encoder::build_freq_array_from_header(const char* in_name, uint8_t &nr_of_0s)
{
	std::ifstream fin;
	fin.open(in_name, std::ios::in | std::ios::binary);

	if (!fin.is_open())
	{
		std::cout << "Could not open file \"" << in_name << "\"." << std::endl;
		return 0;
	}

	//we read the whole header into memory so we can work with it easily
	//the header has a max size of 1 + 1 + 256 * 5 = 1282 bytes, so we use
	//a buffer that will have enough space for it
	char header_buff[2000];
	int header_index{ 2 };

	fin.read(header_buff, 2000);

	//we need to know how many chars we have so we can iterate through the rest of the header
	uint8_t nr_of_chars = *(reinterpret_cast<uint8_t*>(header_buff));
	//we also need to know the number of 0s padded into the last byte to know where to stop when we decompress
	uint8_t nr_of_0s_read = *(reinterpret_cast<uint8_t*>(header_buff + 1));

	nr_of_0s = nr_of_0s_read;
	
	for (int i = 0; i < static_cast<int>(nr_of_chars); i++)
	{
		//get the character
		char c = header_buff[header_index++];
		//get its frequency
		int freq = *(reinterpret_cast<int*>(header_buff + header_index));

		//update the frequency array
		freq_array[static_cast<int>(c)] = freq;

		header_index += 4;
	}

	fin.close();

	return (1 + 1 + static_cast<int>(nr_of_chars) * 5);
}

//this builds the initial file from a compressed file using the header
void H_encoder::decompress(const char* out_name, const char* in_name)
{
	//the Huffman tree is generated using the header
	uint8_t nr_of_0s;
	int in_offset = build_freq_array_from_header(in_name, nr_of_0s);
	generate_huffman_tree();
	
	//open the files and do some error checking
	std::ifstream fin;
	std::ofstream fout;

	fin.open(in_name, std::ios::in | std::ios::binary);
	fout.open(out_name, std::ios::out | std::ios::binary);

	if (!fin.is_open())
	{
		std::cout << "Could not open file \"" << in_name << "\"." << std::endl;
		return;
	}

	if (!fout.is_open())
	{
		std::cout << "Could not open file \"" << out_name << "\"." << std::endl;
		return;
	}

	//if we had only one type of byte in the initial file, then just write it freq times and return
	if (root->left == nullptr && root->right == nullptr)
	{
		for(int i = 0; i < root->freq; i++)
			fout.write(&root->c, 1);

		fin.close();
		fout.close();
		return;
	}

	//initialize the input and output buffers that are used to process the data
	char in_buff[512], out_buff[512];
	//in_index corresponds to the current bit in the byte that we process
	//out_index corresponds to the nr of bytes that we have in out_buff
	int in_index{ 0 }, out_index{ 0 };
	//we need a mask to check each bit from input
	uint8_t mask{ 1 };

	//go to the offset of the file contents
	fin.seekg(in_offset);

	//read and process
	while (!fin.eof())
	{
		fin.read(in_buff, 512);
		long read_bytes = static_cast<long>(fin.gcount());
		
		for (int i = 0; i < read_bytes; i++)
		{
			Node* current_node = root;

			while (1)
			{
				//if we are at the last byte from the compressed file, we need to know where to stop
				if (read_bytes < 512 && i == read_bytes - 1)
				{
					//we use the known nr of 0s from the last byte that we stored in the header to know where to stop
					if (in_index == 8 - nr_of_0s + 1)
					{
						i++;
						break;
					}
				}

				//we need to move to the next byte in this case
				if (in_index == 8)
				{
					in_index = 0;
					i++;

					//if we have a full input buffer we need to read the next block so we can continue processing
					if (i == read_bytes)
					{
						fin.read(in_buff, 512);
						read_bytes = static_cast<long>(fin.gcount());
						i = 0;
					}

				}

				//if we have reached a leaf in the Huffman tree we add the byte to the output buffer and break
				if (current_node != 0)
				{
					if (current_node->left == nullptr && current_node->right == nullptr)
					{
						
						if (out_index == 512)
						{
							fout.write(out_buff, out_index);
							out_index = 0;
						}

						out_buff[out_index++] = current_node->c;
						
						break;
					}
				}
				
				//we check each bit and use the Huffman tree to get the required initial byte
				uint8_t bit_res = (static_cast<uint8_t>(in_buff[i]) & (mask << (7 - in_index)));
				
				if (bit_res == 0)
				{
					if(current_node != 0)
						current_node = current_node->left;
					in_index++;
				}
				else
				{
					if(current_node != 0)
						current_node = current_node->right;
					in_index++;
				}
			}

			i--;
		}

		fout.write(out_buff, out_index);
		out_index = 0;
	}

	fin.close();
	fout.close();
}