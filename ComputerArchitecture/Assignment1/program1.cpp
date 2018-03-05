/* -----------------------
    David Miller
    Assignment 1
    Command to compile : g++ -std=c++11 program1.cpp
    					 ./a.out < memhier.dat
----------------------- */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <cmath>
#include <algorithm>

using namespace std;

/* ---------------------------------------------------------- */
/* ------------------------FRAMEWORK------------------------- */
/* ---------------------------------------------------------- */

/* Elements in sets for associative structs */
struct element {

	string index;
	string tag;
	bool valid = 0;
	bool dirty = 0;
	int lru_bit = 0;

};

/* DATA TRANSITION LOOK ASIDE BUUFER
   LRU Replacement */
struct DTLB {

    int sets = 0;
    int associativity = 0;
    int hits = 0;
    int misses = 0;
    char active = 'n';
    vector<vector<element>> data;

    const int max_sets = 256;
    const int max_associativity = 8;

} the_DTLB; /* <-- Main struct for simulator */

/* PAGE TABLE 
   LRU Replacement */
struct PT {

    int virtual_pages = 0;
    int physical_pages = 0;
    int page_size = 0;
    int hits = 0;
    int faults = 0;
    char virtual_address_active = 'n';
    vector<element> data;
    int next_available_page = 0;

    const int max_virtual_pages = 8192;
    const int max_physical_pages = 1024;

} the_PT; /* <-- Main struct for simulator */

/* DATA CACHE 
   LRU Replacement 
   Write-Allocate / Write-back */
struct DC {

    int sets = 0;
    int line_size = 0;
    int hits = 0;
    int misses = 0;
    int associativity = 0;
    vector<vector<element>> data;

    const int max_sets = 8192;
    const int max_associativity = 8;
    const int min_line_size = 8;

} the_DC; /* <-- Main struct for simulator */

/* VICTIM CACHE 
   LRU Replacement 
   Write-Allocate / Write-back */
struct VC {

    int hits = 0;
    int misses = 0;
    int associativity = 0;
    char active = 'n';
    vector<element> data;

	const int sets = 1;
    const int max_associativity = 8;

} the_VC; /* <-- Main struct for simulator */

/* Helper functions. Go to their initialization for description */
bool get_config();      
bool error_check(); 
void initialize_elements();
string hex_to_binary(string s);
string binary_to_hex(string s);
int hex_to_decimal(string s);
void update_lru_bits(vector<vector<element>> &v, int set_num, int accessed_element);
int lru_replacement_dc(vector<vector<element>> &v, int set_num);
void compute_addresses();   
void compute_virtual_page_nums();
void compute_page_offsets();
void compute_tlb_tags();
void compute_tlb_indices();
void compute_tlb_results();
void compute_pt_results(string s);
void compute_physical_page_nums();
void compute_dc_tags();
void compute_dc_indices();
void compute_dc_results();
void compute_vc_tags();
element compute_vc_results(element evicted, element find_this);
void print_results();   
int log_2(int x);

/* Since I calculate results column by column rather than row by row
this data struct helps me determine invalidating DTLB, DC, and VC lines
that had their corresponding page invalidated */
vector<string> invalid_ppn;
vector<bool> invalidate_ppn;
vector<string> invalid_vpn;
vector<bool> invalidate_vpn;

/* Holds column values */
vector<char> RW;
double statistics[21] = {};
vector<string> virtual_address;
vector<string> virtual_page_nums;
vector<string> page_offsets;
vector<string> tlb_tags;
vector<string> tlb_indices;
vector<string> tlb_results;
vector<string> pt_results;
vector<string> physical_page_nums;
vector<string> dc_tags;
vector<string> dc_indices;
vector<string> dc_results;
vector<string> vc_tags;
vector<string> vc_results;
vector<element> invalidate;

/* ---------------------------------------------------------- */
/* ----------------------MAIN-FUNCTION----------------------- */
/* ---------------------------------------------------------- */

/* Main function, this is where all the magic happens (sort of) */
int main(int argc, char *argv[]) {

	/* Uh oh, we have encountered a problem :( */
    if((!get_config()) || error_check())
        return -1;

    print_results();

    return 0;

}

/* ---------------------------------------------------------- */
/* -------------------- HELPER-FUCNTIONS -------------------- */
/* ---------------------------------------------------------- */

/* Parses file with config for simulator */
bool get_config() {

    vector<int> parameters;
    vector<char> decisions;
    char c;

    /* While we still have reads or writes */
    while(cin.get(c)) {

    	/* Store the read or write */
    	RW.push_back(c);
    	if(c == 'R')
    		statistics[15] = statistics[15] + 1.0;
    	else
    		statistics[16] = statistics[16] + 1.0;

    	/* Eat colon character */
    	cin.get(c);
    	
    	/* Build the virtual address */
    	char address[32] = "";
    	cin.get(c);
    	int i = 0;
    	while(c != '\n') {
    		address[i] = c;
    		cin.get(c);
    		i++;
    	}

    	/* Store the virtual address */
    	virtual_address.push_back(address);

   	}

   	statistics[17] = statistics[15] / (statistics[15] + statistics[16]);

    /* Open trace.config file */
    ifstream config_file;
    config_file.open("trace.config");

    /* Proceed if trace.config was found and opened */
    if(config_file.is_open()) {

        /* Iterate over every line of the file */
        string line;
        while(getline(config_file, line)) {

            stringstream linestream(line);
            string word;

            /* Get last word of file. */
            while(!linestream.eof()) {
                linestream >> word;
            }

            /* If word is an int (parameter) store it */
            int parameter;
            char decision;
            if(stringstream(word) >> parameter)
                parameters.push_back(parameter);

            /* If word is a character (decision) store it */
            stringstream(word) >> decision;
            if((decision == 'y') || (decision == 'n')) {
                decisions.push_back(decision);
            }
            
        }

        /* Done with file so close it */
        config_file.close();

        /* Set DTLB values from parameters */
        the_DTLB.sets = parameters[0];
        the_DTLB.associativity = parameters[1];

        /* Set PT values from parameters */
        the_PT.virtual_pages = parameters[2];
        the_PT.physical_pages = parameters[3];
        the_PT.page_size = parameters[4];

        /* Set DC values from parameters */
        the_DC.sets = parameters[5];
        the_DC.associativity = parameters[6];
        the_DC.line_size = parameters[7];

        /* Set VC values from parameters */
        the_VC.associativity = parameters[8];

        /* Activate structs based decision values */
        the_PT.virtual_address_active = decisions[0];
        the_DTLB.active = decisions[1];
        the_VC.active = decisions[2];

        /* Push empty values onto our result vectors */
        initialize_elements();

        return true;

    }

    /* Uh oh, could not find or open the file :( */
    cout << "File \"trace.config\" was not found or can not be opened." << endl;
    return false;

}

/* Checks that parameters are within specified limits */
bool error_check() {

	/* Check if we exceed max sets for DTLB */
	if(the_DTLB.sets > the_DTLB.max_sets) {
		cout << "DTLB can have only a maximum of " << the_DTLB.max_sets << " sets." << endl;
		return true;
	}

	/* Check if we exceed max sets for DTLB */
	if(the_DC.sets > the_DC.max_sets) {
		cout << "DTLB can have only a maximum of " << the_DC.max_sets << " sets." << endl;
		return true;
	}

	/* Check if we exceed max associativity for DTLB */
	if(the_DTLB.associativity > the_DTLB.max_associativity) {
		cout << "DTLB can have only a maximum of " << the_DTLB.max_associativity << " associativity" << endl;
		return true;
	}

	/* Check if we exceed max associativity for DC */
	if(the_DC.associativity > the_DC.max_associativity) {
		cout << "DC can have only a maximum of " << the_DC.max_associativity << " associativity" << endl;
		return true;
	}

	/* Check if we exceed max associativity for VC */
	if(the_VC.associativity > the_VC.max_associativity) {
		cout << "VC can have only a maximum of " << the_VC.max_associativity << " associativity" << endl;
		return true;
	}

	if((the_PT.virtual_address_active == 'n') && (the_DTLB.active == 'y')) {
		cout << "TLB cannot be enabled when virtual addresses are disabled" << endl;
		return true;
	}

	/* Yay, no error found :) */
    return false;

}

/* Initiaoze our structs with empty data */
void initialize_elements() {

	element empty;
	empty.tag = "";
	empty.index = "";
	empty.valid = 0;
	empty.dirty = 0;
	vector<element> empty_vector;

	for(int i = 0; i < the_DTLB.associativity; i++)
		empty_vector.push_back(empty);

	/* Initialize the DTLB with empty sets */
	for(int i = 0; i < the_DTLB.sets; i++)
		the_DTLB.data.push_back(empty_vector);

	empty_vector.clear();

	for(int i = 0; i < the_DC.associativity; i++)
		empty_vector.push_back(empty);

	/* Initialize the DC with empty sets */
	for(int i = 0; i < the_DC.sets; i++)
		the_DC.data.push_back(empty_vector);

	empty_vector.clear();

	/* Initialize VC with empty elements */
	for(int i = 0; i < the_VC.associativity; i++)
		the_VC.data.push_back(empty);

	/* Initialize the PT with empty elements */
	for(int i = 0; i < the_PT.virtual_pages; i++)
		the_PT.data.push_back(empty);

	return;
}

/* Convert hexadecimal address to binary */
string hex_to_binary(string s) {

	string result;

	/* Get the decimal representation of each hex character */
	for(int i = 0; i < s.length(); i++) {
		int num = 0;
		switch(s[i]) {
			case 'a':
				num = 10;
				break;
			case 'b':
				num = 11;
				break;
			case 'c':
				num = 12;
				break;
			case 'd':
				num = 13;
				break;
			case 'e':
				num = 14;
				break;
			case 'f':
				num = 15;
				break;
			default:
				num = s[i] - 48;
				break;
		}

		/* Get the binary representation of each hex character */
		int j = 3;
		while(j >= 0) {
			if(num >= pow(2,j)) {
				result.push_back('1');
				num = num - pow(2,j);
			}
			else {
				result.push_back('0');
			}
			j--;
		}

	}

	return result;

}

/* Convert binary string to hex string */
string binary_to_hex(string s) {

	if(s.length() < 4) {
		string zero = "0000";
		zero.replace(4-s.length(), s.length(), s);
		s.assign(zero);
	}

	int size = s.length()/4;
	int start = 0;

	if(((s.length() % 4) != 0)) 
		size++;

	string::iterator it;
	string result;

	for(int i = 1; i <= size; i++) {

		if(4*i > s.length()) {
			it = s.end() - 4*(i-1) - (s.length() % 4);
			start = 4 - (s.length() % 4);
		}
		else {
			it = s.end()-4*i;
			start = 0;
		}

		int sum = 0;
		for(int j = start; j < 4; j++) {
			if(*it == '1')
				sum = sum + pow(2,3-j);
			it++;
		}

		if(sum == 10)
			result.push_back('a');
		else if(sum == 11)
			result.push_back('b');
		else if(sum == 12)
			result.push_back('c');
		else if(sum == 13)
			result.push_back('d');
		else if(sum == 14)
			result.push_back('e');
		else if(sum == 15)
			result.push_back('f');
		else
			result.push_back('0' + sum);

	}

	if(size != 1)
		reverse(result.begin(), result.end());
	return result;

}

/* Convert hex string to decimal value */
int hex_to_decimal(string s) {

	string binary_string = hex_to_binary(s);
	int result = 0; 

	for(int i = binary_string.length()-1; i >= 0; i--) 
		if(binary_string[i] == '1')
			result = result + pow(2,binary_string.length()-i-1);

	return result;

}

/* Update LRU bits for a set in the DC */
void update_lru_bits(vector<vector<element>> &v, int set_num, int accessed_element, int associativity) {

	for(int i = 0; i < associativity; i++)
		v[set_num][i].lru_bit++;

	if(accessed_element != -1)
		v[set_num][accessed_element].lru_bit = 0;

	return;
}

/* LRU replacement for the DC */
int lru_replacement_dc(vector<vector<element>> &v, int set_num) {

	int max = -1;
	int index = -1;

	for(int i = 0; i < v[set_num].size(); i++) {
		if(v[set_num][i].lru_bit > max) {
			max = v[set_num][i].lru_bit;
			index = i;
		}
	}

	v[set_num][index].lru_bit = 0;

	return index;
}

/* Compute the addresses */
void compute_addresses() {

	vector<string> padded_addresses;

	/* Iterate over addresses */
	for(int i = 0; i < virtual_address.size(); i++) {
		
		char zero[9] = "00000000";
		for(int j = virtual_address[i].length(); j > 0; j--)
			zero[strlen(zero)-j] = virtual_address[i][virtual_address[i].length()-j];

		padded_addresses.push_back(zero);

	}

	virtual_address.swap(padded_addresses);

	return;
}

/* Compute the virtual page numbers */
void compute_virtual_page_nums() {

	string binary_address, binary_page_num, hex_page_num; 

	/* Iterate over addresses */
		for(int i = 0; i < virtual_address.size(); i++) {

			binary_address = hex_to_binary(virtual_address[i]);
			binary_page_num = binary_address.substr(binary_address.length() - (log_2(the_PT.page_size) + log_2(the_PT.virtual_pages)), log_2(the_PT.virtual_pages));
			hex_page_num = binary_to_hex(binary_page_num);

			while(hex_page_num[0] == '0')
				hex_page_num = hex_page_num.substr(1, hex_page_num.length());

			if(hex_page_num.length() == 0)
				hex_page_num = '0';

			virtual_page_nums.push_back(hex_page_num);

		}

	return;
}

/* Compute the page offsets */
void compute_page_offsets() {

	string binary_address, binary_offset, hex_offset;

	for(int i = 0; i < virtual_address.size(); i++) {

		binary_address = hex_to_binary(virtual_address[i]);
		//cout << "Binary address : " << binary_address << endl;
		binary_offset = binary_address.substr(binary_address.length() - log_2(the_PT.page_size),log_2(the_PT.page_size));
		
		//cout << "Sending binary offset : " << binary_offset << endl;

		hex_offset = binary_to_hex(binary_offset); 

		while(hex_offset[0] == '0')
			hex_offset = hex_offset.substr(1, hex_offset.length()-1);

		if(hex_offset.length() == 0)
			hex_offset = '0';

		page_offsets.push_back(hex_offset);

	}

	return;
}

/* Compute the TLB tags */
void compute_tlb_tags() {

	string binary_address, binary_tlb_tag, hex_tlb_tag;

	for(int i = 0; i < virtual_address.size(); i++) {

		binary_address = hex_to_binary(virtual_address[i]);
		binary_tlb_tag = binary_address.substr(0, binary_address.length() - (log_2(the_PT.page_size) + log_2(the_DTLB.sets)));

		hex_tlb_tag = binary_to_hex(binary_tlb_tag);

		while(hex_tlb_tag[0] == '0')
			hex_tlb_tag = hex_tlb_tag.substr(1, hex_tlb_tag.length() - 1);

		if(hex_tlb_tag.length() == 0)
			hex_tlb_tag = '0';

		tlb_tags.push_back(hex_tlb_tag);

	}

	return;
}

/* Compute the TLB indices */
void compute_tlb_indices() {

	string binary_address, binary_tlb_index, hex_tlb_index;

	for(int i = 0; i < virtual_address.size(); i++) {

		binary_address = hex_to_binary(virtual_address[i]);
		binary_tlb_index = binary_address.substr(binary_address.length() - (log_2(the_PT.page_size) + log_2(the_DTLB.sets)), log_2(the_DTLB.sets));

		hex_tlb_index = binary_to_hex(binary_tlb_index);

		while(hex_tlb_index[0] == '0')
			hex_tlb_index = hex_tlb_index.substr(1, hex_tlb_index.length() - 1);

		if(hex_tlb_index.length() == 0)
			hex_tlb_index = '0';

		tlb_indices.push_back(hex_tlb_index);

	}

	return;
}

/* Compute the DTLB results */
void compute_tlb_results() {

	bool hit = false;
	double hit_count = 0;
	double miss_count = 0;

	/* Iterate over addresses */
	for(int i = 0; i < virtual_address.size(); i++) {

		hit = false;

		for(int j = 0; j < the_DTLB.associativity; j++) {

			for(int n = 0; n < i; n++) {
				if((invalidate_vpn[i]) && (virtual_page_nums[n].compare(invalid_vpn[i]))) {
					the_DTLB.data[hex_to_decimal(tlb_indices[n])][j].valid = 0; 
					invalidate_vpn[i] = false;
					break;
				}
			}

			/* Free spot in the DTLB */
			if(the_DTLB.data[hex_to_decimal(tlb_indices[i])][j].valid == 0) { 
				the_DTLB.data[hex_to_decimal(tlb_indices[i])][j].valid = 1;
				the_DTLB.data[hex_to_decimal(tlb_indices[i])][j].tag.assign(tlb_tags[i]);
				tlb_results.push_back("miss");
				miss_count++;
				hit = true;

				for(int k = 0; k < the_DTLB.associativity; k++)
					the_DTLB.data[hex_to_decimal(tlb_indices[i])][k].lru_bit++;

				the_DTLB.data[hex_to_decimal(tlb_indices[i])][j].lru_bit = 0;

				break;
			}
			/* Found a match in the DTLB */
			if((the_DTLB.data[hex_to_decimal(tlb_indices[i])][j].valid == 1) && (tlb_tags[i].compare(the_DTLB.data[hex_to_decimal(tlb_indices[i])][j].tag) == 0)) {
				tlb_results.push_back("hit");
				hit_count++;
				hit = true;

				for(int k = 0; k < the_DTLB.associativity; k++)
					the_DTLB.data[hex_to_decimal(tlb_indices[i])][k].lru_bit++;

				the_DTLB.data[hex_to_decimal(tlb_indices[i])][j].lru_bit = 0;

				break;
			}
		}

		if(!hit) {

			tlb_results.push_back("miss");

			int max = -1;
			int index = -1;

			for(int j = 0; j < the_DTLB.associativity; j++) {
				if(the_DTLB.data[hex_to_decimal(tlb_indices[i])][j].lru_bit > max)
					max = the_DTLB.data[hex_to_decimal(tlb_indices[i])][j].lru_bit;
					index = j;
			}

			the_DTLB.data[hex_to_decimal(tlb_indices[i])][index].tag.assign(tlb_tags[i]);
			the_DTLB.data[hex_to_decimal(tlb_indices[i])][index].lru_bit = 0;

		}

	}	

	statistics[0] = hit_count;
	statistics[1] = miss_count;
	statistics[2] = (hit_count / (hit_count + miss_count));

	return;
}

/* Compute the PT results given some virtual address */
void compute_pt_results(string s) {

	bool hit = false;

	for(int i = 0; i < the_PT.physical_pages; i++) {

		if(the_PT.data[i].valid == 0) {

			the_PT.data[i].valid = 1;
			the_PT.data[i].index.assign(s);
			the_PT.data[i].tag = to_string(i);

			for(int j = 0; j < the_PT.physical_pages; j++)
				the_PT.data[j].lru_bit++;

			the_PT.data[i].lru_bit = 0;

			physical_page_nums.push_back(the_PT.data[i].tag);
			pt_results.push_back("miss");
			statistics[4]++;
			hit = true;
			break;
		}
		if((the_PT.data[i].valid == 1) && (s.compare(the_PT.data[i].index) == 0)) {

			pt_results.push_back("hit");
			physical_page_nums.push_back(the_PT.data[i].tag);
			statistics[3]++;
			hit = true;

			for(int j = 0; j < the_PT.physical_pages; j++)
				the_PT.data[j].lru_bit++;

			the_PT.data[i].lru_bit = 0;
			break;
		}
	}

	if(!hit) {

		int max = -1;
		int index = -1;

		for(int i = 0; i < the_PT.physical_pages; i++) {
			if(the_PT.data[i].lru_bit > max) {
				max = the_PT.data[i].lru_bit;
				index = i;
			}
		}

		/* Evicting a write so we must access disk to update data */
		if(RW[index] == 'W')
			statistics[20]++;


		for(int k = 0; k < the_PT.physical_pages; k++)
			the_PT.data[k].lru_bit++;

		invalid_ppn.push_back(the_PT.data[index].tag);
		invalidate_ppn.push_back(true);
		invalid_vpn.push_back(the_PT.data[index].index);
		invalidate_vpn.push_back(true);
		
		the_PT.data[index].index.assign(s);
		the_PT.data[index].lru_bit = 0;

		pt_results.push_back("miss");
		statistics[4]++;
		physical_page_nums.push_back(to_string(index));
	}
	else {
		string empty = "trash string";
		invalid_ppn.push_back(empty);
		invalidate_ppn.push_back(false);
		invalid_vpn.push_back(empty);
		invalidate_vpn.push_back(false);
	}

	statistics[5] = statistics[3] / (statistics[3] + statistics[4]);

	return;
}

/* Compute the physical page numbers */
void compute_physical_page_nums() {

	/* Virtual address not set, calulate without accessing PT */
	if(the_PT.virtual_address_active == 'n') {

		string binary_address, binary_page_num, hex_page_num;	

		/* Iterate over addresses */
		for(int i = 0; i < virtual_address.size(); i++) {

			binary_address = hex_to_binary(virtual_address[i]);
			binary_page_num = binary_address.substr(binary_address.length() - (log_2(the_PT.page_size) + log_2(the_PT.physical_pages)), log_2(the_PT.physical_pages));

			hex_page_num = binary_to_hex(binary_page_num);

			while(hex_page_num[0] == '0')
				hex_page_num = hex_page_num.substr(1, hex_page_num.length() - 1);

			if(hex_page_num.length() == 0)
				hex_page_num = '0';

			physical_page_nums.push_back(hex_page_num);

		} 
	}
	/* Virtual address, index into PT via virtual address to get physical page number */
	else{
		for(int i = 0; i < virtual_address.size(); i++)
			compute_pt_results(virtual_page_nums[i]);
	}

	return;
}

/* Compute the DC tags */
void compute_dc_tags() {

	/* Virtual address not used */
	if(the_PT.virtual_address_active == 'n') {

		string binary_address, binary_dc_tag, hex_dc_tag;

		/* Iterate over addresses */
		for(int i = 0; i < virtual_address.size(); i++) {

			binary_address = hex_to_binary(virtual_address[i]);
			binary_dc_tag = binary_address.substr(0, binary_address.length() - (log_2(the_DC.line_size) + log_2(the_DC.sets)));

			hex_dc_tag = binary_to_hex(binary_dc_tag);

			while(hex_dc_tag[0] == '0')
				hex_dc_tag = hex_dc_tag.substr(1, hex_dc_tag.length() - 1);

			if(hex_dc_tag.length() == 0)
				hex_dc_tag = '0';

			dc_tags.push_back(hex_dc_tag);

		}
	}
	/* Virtual address used */
	else {

		for(int i = 0; i < virtual_address.size(); i++) {

			string pad;
			string full_offset = hex_to_binary(page_offsets[i]);
			int end = log_2(the_PT.page_size) - full_offset.length();

			if(end > 0) {
				for(int j = 0; j < end; j++)
					pad.push_back('0');
				full_offset.insert(0, pad);
			}
			if(end < 0) {
				full_offset.erase(0,-1*end);
			}
			
			string binary_crap = hex_to_binary(physical_page_nums[i]) + full_offset;

			string binary_tag = binary_crap.substr(0, binary_crap.length() - (log_2(the_DC.line_size) + log_2(the_DC.sets)));
			string hex_tag = binary_to_hex(binary_tag);

			while(hex_tag[0] == '0')
				hex_tag = hex_tag.substr(1, hex_tag.length() - 1);

			if(hex_tag.length() == 0)
				hex_tag = '0';

			dc_tags.push_back(hex_tag);

		}

	}

	return;
}

/* Compute the DC indices */
void compute_dc_indices() {

	string binary_address, binary_dc_index, hex_dc_index;

	/* Iterate over addresses */
	for(int i = 0; i < virtual_address.size(); i++) {

		binary_address = hex_to_binary(virtual_address[i]);
		binary_dc_index = binary_address.substr(binary_address.length() - (log_2(the_DC.line_size) + log_2(the_DC.sets)), log_2(the_DC.sets));

		hex_dc_index = binary_to_hex(binary_dc_index);

		while(hex_dc_index[0] == '0')
			hex_dc_index = hex_dc_index.substr(1, hex_dc_index.length() - 1);

		if(hex_dc_index.length() == 0)
			hex_dc_index = '0';

		string zero = "0";
		if(the_DC.sets == 1)
			dc_indices.push_back(zero);
		else
			dc_indices.push_back(hex_dc_index);

	}

	return;
}

/* Compute DC hits and misses */
void compute_dc_results() {

	bool hit = false;
	bool vc_hit = false;
	double hit_count = 0;
	double miss_count = 0;

	/* Iterate over addresses */
	for(int i = 0; i < virtual_address.size(); i++) {

		hit = false;

		for(int j = 0; j < the_DC.associativity; j++) {

			if(the_PT.virtual_address_active == 'y') {
				for(int n = 0; n < i; n++) {
					if((invalidate_ppn[i]) && (invalid_ppn[i].compare(physical_page_nums[n]) == 0)) {
						the_DC.data[n][j].valid = 0;
						if(the_DC.data[n][j].dirty == 1) {
							statistics[18]++;
							the_DC.data[n][j].dirty = 0;
						} 
						invalidate_ppn[i] = false;
						break;
					}
				}
			}

			/* Found a match in the DTLB */
			if((the_DC.data[hex_to_decimal(dc_indices[i])][j].valid == 1) && (dc_tags[i].compare(the_DC.data[hex_to_decimal(dc_indices[i])][j].tag)) == 0) {
				dc_results.push_back("hit");
				hit_count++;

				/* Toggle dirty bit on write hit */
				if(RW[i] == 'W')
					the_DC.data[hex_to_decimal(dc_indices[i])][j].dirty = 1;

				if(the_VC.active == 'n')
					statistics[12]++;

				hit = true;
				update_lru_bits(the_DC.data, hex_to_decimal(dc_indices[i]), j, the_DC.associativity);
				break;
			}

			if(the_DC.data[hex_to_decimal(dc_indices[i])][j].valid == 0) {
				dc_results.push_back("miss");
				the_DC.data[hex_to_decimal(dc_indices[i])][j].valid = 1;
				the_DC.data[hex_to_decimal(dc_indices[i])][j].tag.assign(dc_tags[i]);
				miss_count++;
				hit = true;

				/* Toggle dirty bit on write allocation */
				if(RW[i] == 'W')
					the_DC.data[hex_to_decimal(dc_indices[i])][j].dirty = 1;

				if(the_VC.active == 'n')
					statistics[13]++;

				statistics[18]++;
				update_lru_bits(the_DC.data, hex_to_decimal(dc_indices[i]), -1, the_DC.associativity);
				break;
			}
		}
		/* Did not find a match in the DC */
		if(!hit) { 

			dc_results.push_back("miss");
			miss_count++;
			vc_hit = false;

			/* Determine which line in which set will be replaced */
			int replace_this = lru_replacement_dc(the_DC.data, hex_to_decimal(dc_indices[i]));

			if(the_DC.data[hex_to_decimal(dc_indices[i])][replace_this].dirty == 1) {
				the_DC.data[hex_to_decimal(dc_indices[i])][replace_this].dirty = 0;
				statistics[18]++;
			}

			/* Not in DC but will search for it in the VC */
			if(the_VC.active == 'y') {
				element look_for_me;
				look_for_me.tag = dc_tags[i];
				look_for_me.index = dc_indices[i];
				element incoming = compute_vc_results(the_DC.data[hex_to_decimal(dc_indices[i])][replace_this], look_for_me);
			
				/* Found it in the VC */
				if(incoming.lru_bit == 0) {
					the_DC.data[hex_to_decimal(dc_indices[i])][replace_this].tag = incoming.tag;
					the_DC.data[hex_to_decimal(dc_indices[i])][replace_this].index = incoming.index;
					the_DC.data[hex_to_decimal(dc_indices[i])][replace_this].lru_bit = incoming.lru_bit;
					vc_results.push_back("hit");
					vc_hit = true;
				}
				else{
					vc_results.push_back("miss");
				}

			}

			/* We are replacinga a line which has to be written back to lower level of memory */
			if(the_DC.data[hex_to_decimal(dc_indices[i])][replace_this].dirty == 1) {
				the_DC.data[hex_to_decimal(dc_indices[i])][replace_this].dirty = 0;
				statistics[18]++;
			}

			if(!vc_hit) {
				the_DC.data[hex_to_decimal(dc_indices[i])][replace_this].tag.assign(dc_tags[i]);
			}

			if(the_VC.active == 'n')
				statistics[13]++;

		}
		else {
			vc_results.push_back("miss");
		}

	}

	statistics[6] = hit_count;
	statistics[7] = miss_count;
	statistics[8] = hit_count / (hit_count + miss_count);

	return;
}

/* Compute the VC tags */
void compute_vc_tags() {

	/* Virtual address is used */
	if(the_PT.virtual_address_active == 'n') {

		string binary_address, binary_vc_tag, hex_vc_tag;

		/* Iterate over addresses */
		for(int i = 0; i < virtual_address.size(); i++) {

			binary_address = hex_to_binary(virtual_address[i]);
			binary_vc_tag = binary_address.substr(0, binary_address.length() - log_2(the_DC.line_size));

			hex_vc_tag = binary_to_hex(binary_vc_tag);

			while(hex_vc_tag[0] == '0')
				hex_vc_tag = hex_vc_tag.substr(1, hex_vc_tag.length() - 1);

			if(hex_vc_tag.length() == 0)
				hex_vc_tag = '0';

			vc_tags.push_back(hex_vc_tag);

		}
	}
	else {

		for(int i = 0; i < virtual_address.size(); i++) {

			string pad;
			string full_offset = hex_to_binary(page_offsets[i]);
			int end = log_2(the_PT.page_size) - full_offset.length();

			if(end > 0) {
				for(int j = 0; j < end; j++)
					pad.push_back('0');
				full_offset.insert(0, pad);
			}
			if(end < 0) {
				full_offset.erase(0,-1*end);
			}
			
			string binary_crap = hex_to_binary(physical_page_nums[i]) + full_offset;
			string binary_tag = binary_crap.substr(0, binary_crap.length() - (log_2(the_DC.line_size)));
			string hex_tag = binary_to_hex(binary_tag);

			while(hex_tag[0] == '0')
				hex_tag = hex_tag.substr(1, hex_tag.length() - 1);

			if(hex_tag.length() == 0)
				hex_tag = '0';

			vc_tags.push_back(hex_tag);

		}

	}

	return;
}

/* Compute the VC results */
element compute_vc_results(element evicted, element find_this) {

	element e;
	e.lru_bit = -1; 
	bool found = false;

	for(int i = 0; i < the_VC.associativity; i++) {

		if(the_PT.virtual_address_active == 'y') {
			for(int n = 0; n < i; n++) {
				if((invalidate_ppn[i]) && (invalid_ppn[i].compare(physical_page_nums[n]) == 0)) {
					the_VC.data[n].valid = 0; 
					invalidate_ppn[i] = false;
					break;
				}
			}
		}

		/* We found a match in the VC */
		if((find_this.tag.compare(the_VC.data[i].tag) == 0) && (find_this.index.compare(the_VC.data[i].index))) {
			e.tag = the_VC.data[i].tag;
			e.index = the_VC.data[i].index;
			e.lru_bit = 0;
			the_VC.data[i].tag.swap(evicted.tag);
			the_VC.data[i].index.swap(evicted.index);
			found = true;
			statistics[9]++;

			for(int j = 0; j < the_VC.data.size(); j++)	
				the_VC.data[j].lru_bit++;
			the_VC.data[i].lru_bit = 0;

			break;
		}
		/* There is an open spot to occupy (this menas no match was found in the VC) */
		if(the_VC.data[i].valid == 0) {
			the_VC.data[i].tag.swap(evicted.tag);
			the_VC.data[i].index.swap(evicted.index);
			the_VC.data[i].valid = 1;
			found = true;
			for(int j = 0; j < the_VC.data.size(); j++)	
				the_VC.data[j].lru_bit++;
			the_VC.data[i].lru_bit = 0;

			break;
		}
	}

	/* There was no match in VC and it is at capacity so we need to make room for the evicted line */
	if(!found) {

		int max = -2;
		int index = -1;
		for(int i = 0; i < the_VC.data.size(); i++) {
			if(the_VC.data[i].lru_bit > max) {
				max = the_VC.data[i].lru_bit;
				index = i;
			}
		}

		the_VC.data[index].tag = evicted.tag;
		the_VC.data[index].index = evicted.index;
		the_VC.data[index].lru_bit = 0;
	}

	return e;
}

/* Prints the output for all the data and statistics */
void print_results() {

	int FILL_OUT = 420;

	cout << "Data TLB contains " << the_DTLB.sets << " sets." << endl;
	cout << "Each set contains " << the_DTLB.associativity << " entries." << endl;
	cout << "Number of bits used for index is " << log_2(the_DTLB.sets) << "." << endl << endl;

	cout << "Number of virtual pages is " << the_PT.virtual_pages << "." << endl;
	cout << "Number of physical pages is " << the_PT.physical_pages << "." << endl;
	cout << "Each page contains " << the_PT.page_size << " bytes." << endl;
	cout << "Number of bits used for the page table index is " << log_2(the_PT.virtual_pages) << "." << endl;
	cout << "Number of bits used for the page offset is " << log_2(the_PT.page_size) << "." << endl << endl;

	cout << "D-cache contains " << the_DC.sets << " sets." << endl;
	cout << "Each set contains " << the_DC.associativity << " entries." << endl;
	cout << "Each line is " << the_DC.line_size << " bytes." << endl;
	cout << "Number of bits used for the index is " << log_2(the_DC.sets) << "." << endl;
	cout << "Number of bits used for the offset is " << log_2(the_DC.line_size) << "." << endl << endl;

	cout << "Each V-cache set contains " << the_VC.associativity << " entries." << endl << endl;

	if(the_PT.virtual_address_active == 'y') {
		cout << "The addresses read in are virtual addresses" << endl;
		if(the_DTLB.active = 'n') {
			cout << "TLB is disabled in this configuration." << endl;
		}
		if(the_VC.active == 'n') {
			cout << "VC is disabled in this configuration." << endl;
		}
		cout << endl << endl;
		cout << "Virtual  Virt.  Page TLB    TLB TLB  PT   Phys        DC  DC          VC  " << endl;
	}
	else {
		cout << "The addresses read in are physical addresses" << endl;
		if(the_DTLB.active = 'n') {
			cout << "TLB is disabled in this configuration." << endl;
		}
		if(the_VC.active == 'n') {
			cout << "VC is disabled in this configuration." << endl;
		}
		cout << endl << endl;
		cout << "Physical Virt.  Page TLB    TLB TLB  PT   Phys        DC  DC          VC  " << endl;
	}

	cout << "Address  Page # Off  Tag    Ind Res. Res. Pg # DC Tag Ind Res. VC Tag Res." << endl;
	cout << "-------- ------ ---- ------ --- ---- ---- ---- ------ --- ---- ------ ----" << endl;

	/* Print out the columns */
	compute_addresses();

	if(the_PT.virtual_address_active == 'y') {
		compute_virtual_page_nums();
	}

	compute_page_offsets();
	compute_physical_page_nums();

	if(the_DTLB.active == 'y') {
		compute_tlb_tags();
		compute_tlb_indices();
		compute_tlb_results();
		// compute_pt_results();
	}

	compute_dc_tags();
	compute_dc_indices();
	compute_dc_results();

	if(the_VC.active == 'y') {
		compute_vc_tags();
		statistics[10] = virtual_address.size() - statistics[9];
	}

	/* Correct PT hits if TLB is implemented */
	if(the_DTLB.active == 'y') {
		for(int i = 0; i < virtual_address.size(); i++) 
			if(tlb_results[i].compare("hit") == 0) {
				pt_results[i].clear();
				statistics[3]--;
			}
		statistics[5] = statistics[3] / (statistics[3] + statistics[4]);
	}

	statistics[12] = statistics[9] + statistics[6];
	statistics[13] = virtual_address.size() - statistics[12];
	statistics[14] = statistics[12] / (statistics[12] + statistics[13]);

	string print_nothing = "";
	for(int i = 0; i < virtual_address.size(); i++) {


		/* Print virtual/physical page address */
		cout << setw(8) << virtual_address[i];

		/* Print virtual page numbers */
		if(the_PT.virtual_address_active == 'y')
			cout << " " << right << setw(6) << virtual_page_nums[i];
		else
			cout << " " << right << setw(6) << print_nothing;

		/* Print page offset */
		cout << " " << right << setw(4) << page_offsets[i];

		/* Print TLB tags */
		if(the_DTLB.active == 'y') {
			cout << " " << right << setw(6) << tlb_tags[i];
			cout << " " << right << setw(3) << tlb_indices[i];
			cout << " " << left << setw(4) << tlb_results[i];
		}
		else {
			cout << " " << right << setw(6) << print_nothing;
			cout << " " << right << setw(3) << print_nothing;
			cout << " " << right << setw(4) << print_nothing;
		}

		/* Print PT results */
		if(the_PT.virtual_address_active == 'y')
			cout << " " << left << setw(4) << pt_results[i];
		else
			cout << " " << right << setw(4) << print_nothing;

		/* Print Physical Page numbers */
		cout << " " << right << setw(4) << physical_page_nums[i];

		/* Print DC tag results */
		cout << " " << right << setw(6) << dc_tags[i];

		/* Print DC index results */
		cout << " " << right << setw(3) << dc_indices[i];

		cout << " " << left << setw(4) << dc_results[i];

		if(the_VC.active == 'y') {
			cout << " " << right << setw(6) << vc_tags[i];
			cout << " " << left  << setw(4) << vc_results[i];
		}
		else {
			cout << " " << right << setw(6) << print_nothing;
			cout << " " << right << setw(4) << print_nothing;
		}

		cout << endl;
	}

	cout << endl << endl;
	cout << "Simulation Statistics" << endl << endl;

	cout << "dtlb hits        : " << statistics[0] << endl;
	cout << "dtlb misses      : " << statistics[1] << endl;
	cout << "dtlb hit ratio   : "; 
	if((statistics[0] + statistics[1]) == 0)
		cout << "N/A" << endl << endl;
	else 
		cout << fixed << setprecision(6) << statistics[2] << endl << endl;

	cout << fixed << setprecision(0) << "pt hits          : " << statistics[3] << endl;
	cout << "pt faults        : " << statistics[4] << endl;
	cout << "pt hit ratio     : ";
	if((statistics[3] + statistics[4]) == 0)
		cout << "N/A" << endl << endl;
	else 
		cout << fixed << setprecision(6) << statistics[5] << endl << endl;

	cout << fixed << setprecision(0) << "dc hits          : " << statistics[6] << endl;
	cout << "dc misses        : " << statistics[7] << endl;
	cout << "dc hit ratio     : ";
	if((statistics[6] + statistics[7]) == 0)
		cout << "N/A" << endl << endl;
	else 
		cout << fixed << setprecision(6) << statistics[8] << endl << endl;

	cout << fixed << setprecision(0) << "vc hits          : " << statistics[9] << endl;
	cout << "vc misses        : " << statistics[10] << endl;
	cout << "vc hit ratio     : ";
	if((statistics[9] + statistics[10]) == 0)
		cout << "N/A" << endl << endl;
	else 
		cout << fixed << setprecision(6) << statistics[11] << endl << endl;

	cout << fixed << setprecision(0) << "L1 hits          : " << statistics[12] << endl;
	cout << "L1 misses        : " << statistics[13] << endl;
	cout << "L1 hit ratio     : ";
	if((statistics[12] + statistics[13]) == 0)
		cout << "N/A" << endl << endl;
	else 
		cout << fixed << setprecision(6) << statistics[14] << endl << endl;

	cout << fixed << setprecision(0) << "Total reads      : " << statistics[15] << endl;
	cout << "Total writes     : " << statistics[16] << endl;
	cout << "Ratio of reads   : ";
	if((statistics[15] + statistics[16]) == 0)
		cout << fixed << setprecision(6) << "N/A" << endl << endl;
	else 
		cout << fixed << setprecision(6) << statistics[17] << endl << endl;

	statistics[19] = statistics[3] + statistics[4];

	cout << fixed << setprecision(0) << "main memory refs : " << statistics[18] << endl;
	cout << "page table refs  : " << statistics[19] << endl;

	statistics[20] = statistics[20] + statistics[4];
	cout << "disk refs        : " << statistics[20] << endl;

    return;

} 

/* Calculate ceil(log_2(x)) */
int log_2(int x) {

	/* Cant calculate log(0), it is undefined */
	if(x == 0) {
		cout << "log_2(0) is undefined; returning \"0\" as log_2(0)" << endl;
		return 0;
	}

	/* Special case, return 1 if input is 1 */
	if(x == 1)
		return 0;

	/* Rest of code determines how many are required to represent x */
	double y = x;
	int b = 0;

	while(y > 1) {
		y /= 2;
		b++;
	}

	return b;

}