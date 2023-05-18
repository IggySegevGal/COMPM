/* 046267 Computer Architecture - Winter 20/21 - HW #2 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits.h>
//#include <cmath.h>

using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::stringstream;
using std::vector;
using std::cmath; // need to checkkkkkkkkkkkkkkkkk

/* statistics struct */

/* A structure to return information about the currect simulator state */
typedef struct {
	double L1Miss = 0;
	double L1Hit = 0;
	double L2Miss = 0;
	double L2Hit = 0;
	double AccTime = 0;
	double AccNum = 0;
} SIM_stats;

/* ---------------------------------------------------- cache_line class ------------------------------------------------ */
class cache_line { 
public:
unsigned long int address;
unsigned long int tag;
unsigned dirty_bit;
unsigned set;


   // constructors
   cache_line(){
	this->address = ULONG_MAX; // init to maximum value of unsigned long int, assuming addersses are aligned to 4 
	this->tag = ULONG_MAX; // init to maximum value of unsigned long int, assuming block size is never zero bits
	this->dirty_bit = 0;
   }
    
    // destructor
    ~cache_line(){
    }
   
    //method functions
	void init_line(unsigned long int address, char operation,unsigned sets,unsigned BSize){
		// calculate setBits from sets:
		unsigned setBits = log2(sets);
		this->set = (address >> BSize) % sets;
		this->dirty_bit = 1;
		if (operation == 'R'){
			this->dirty_bit = 0;
		}
		this->address = address;
		this->tag = address >> (setBits+BSize); // shift right to remove bits of block size, set size
		
   	}

   };

/* ---------------------------------------------- cache_class ---------------------------------------- */
class cache_class { 
public:
unsigned MemCyc;
unsigned BSize;
unsigned LSize;
unsigned LAssoc;
unsigned LCy;
unsigned WrAlloc;
unsigned ways;
unsigned sets;
vector<vector<cache_line>> cache_table; // sets X ways [cache_line] 
// vector<unsigned> lru_vec; // for each set

   // constructors
	cache_class(){
	}
    
    // destructor
    ~cache_class(){
    }
   
    //method functions
   	void init_class(unsigned MemCyc, unsigned BSize , unsigned LSize, unsigned LAssoc, unsigned LCyc , unsigned WrAlloc){
			this->MemCyc = MemCyc;
			this->BSize = BSize;
			this->LSize = LSize;
			this->LAssoc = LAssoc;
			this->LCy = LCy;
			this->WrAlloc = WrAlloc;
			this->ways = pow(2,LAssoc);
			this->sets = (pow(2,LSize) / pow(2,BSize)) / ways;
			// init a 1D cache: sets -> number of rows, ways -> number of columns init empty;
			this->cache_table = cache_table.resize(sets);//, vector<cache_line>(ways)); // only init sets, not ways
			//this->lru_vec = lru_vec.resize(sets);
	}

	//method functions
	cache_line create_cache_line(unsigned long int address, char operation){
		// create cache_line object and initialize it:
		cache_line tmp_cache_line;
		return tmp_cache_line.init_line( address, operation, this->sets, this->BSize);
	}

   	void push_back_line(cache_line line){
		// push back to cache:
		this->cache_table[line.set].push_back(line);
	
	}

	void erase_cache_line(cache_line line){ 
		// erase line:
		vector<cache_line>::iterator it;
        for (it = cache_table[line.set].begin() ; it != cache_table[line.set].end(); ++it){
            if (it->tag == line.tag){
                cache_table[line.set].erase(it);
				break;
            }
    	}
		return;
	}

	void update_LRU(cache_line line){ // erase and push back
		// erase line:
		erase_cache_line(line);
		// push back to cache:
		push_back_line(cache_line line);
		return;
	}




   };

/* --------------------------------------------------- general function ------------------------------------------- */
void handle_command(unsigned long int address, char operation,cache_class L1,cache_class L2,SIM_stats stats){
	// prepare new cache lines:
	cache_line L1_newline = L1.create_cache_line(address, operation);
	cache_line L2_newline = L2.create_cache_line(address, operation);
	
	// update LRU == (erase and pushback, because most recently used element is last)
	
	// hit L1 (write & read)
		// update LRU, update Dirty if write
	// miss L1 hit L2
		// L1 WA: (WA && operation='W') || (operation='R')
			// update LRU L2 with new
			// if L1 set is full:
				//not dirty: remove first element in set from L1
				//dirty: write first element in set from L1 to L2 and then remove from L1 update LRU L2 with removed
			// insert line to L1, update LRU L1
			
		// L1 NWA: else(NWA)
			// L1 nothing
			// update Dirty L2 if write
			// update LRU L2 
		
	// __________________ need to complete __________________________:
	// miss L1 miss L2
		//(operation='W')
			// L1 WA L2 WA

			// L1 WA L2 NWA
			// L1 NWA L2 WA
			// L1 NWA L2 NWA
		//(operation='R')
		// insert line to L2, update LRU L2
		// insert line to L1, update LRU L1
}

/* ---------------------------------------------------- globals: -------------------------------------------------- */
cache_class L1;
cache_class L2;
SIM_stats stats;

/* ----------------------------------------------------- main ------------------------------------------------------- */
int main(int argc, char **argv) {

	if (argc < 19) {
		cerr << "Not enough arguments" << endl;
		return 0;
	}

	// Get input arguments

	// File
	// Assuming it is the first argument
	char* fileString = argv[1];
	ifstream file(fileString); //input file stream
	string line;
	if (!file || !file.good()) {
		// File doesn't exist or some other error
		cerr << "File not found" << endl;
		return 0;
	}

	unsigned MemCyc = 0, BSize = 0, L1Size = 0, L2Size = 0, L1Assoc = 0,
			L2Assoc = 0, L1Cyc = 0, L2Cyc = 0, WrAlloc = 0;

	for (int i = 2; i < 19; i += 2) {
		string s(argv[i]);
		if (s == "--mem-cyc") {
			MemCyc = atoi(argv[i + 1]);
		} else if (s == "--bsize") {
			BSize = atoi(argv[i + 1]);
		} else if (s == "--l1-size") {
			L1Size = atoi(argv[i + 1]);
		} else if (s == "--l2-size") {
			L2Size = atoi(argv[i + 1]);
		} else if (s == "--l1-cyc") {
			L1Cyc = atoi(argv[i + 1]);
		} else if (s == "--l2-cyc") {
			L2Cyc = atoi(argv[i + 1]);
		} else if (s == "--l1-assoc") {
			L1Assoc = atoi(argv[i + 1]);
		} else if (s == "--l2-assoc") {
			L2Assoc = atoi(argv[i + 1]);
		} else if (s == "--wr-alloc") {
			WrAlloc = atoi(argv[i + 1]);
		} else {
			cerr << "Error in arguments" << endl;
			return 0;
		}
	}

	/* init caches: */
	L1.init_class( MemCyc,  BSize ,  L1Size,  L1Assoc,  L1Cyc ,  WrAlloc);
	L2.init_class( MemCyc,  BSize ,  L2Size,  L2Assoc,  L2Cyc ,  WrAlloc);

	while (getline(file, line)) {

		stringstream ss(line);
		string address;
		char operation = 0; // read (R) or write (W)
		if (!(ss >> operation >> address)) {
			// Operation appears in an Invalid format
			cout << "Command Format error" << endl;
			return 0;
		}

		// DEBUG - remove this line
		cout << "operation: " << operation;

		string cutAddress = address.substr(2); // Removing the "0x" part of the address

		// DEBUG - remove this line
		cout << ", address (hex)" << cutAddress;

		unsigned long int num = 0;
		num = strtoul(cutAddress.c_str(), NULL, 16);

		// DEBUG - remove this line
		cout << " (dec) " << num << endl;

	}

	double L1MissRate;
	double L2MissRate;
	double avgAccTime;

	printf("L1miss=%.03f ", L1MissRate);
	printf("L2miss=%.03f ", L2MissRate);
	printf("AccTimeAvg=%.03f\n", avgAccTime);

	return 0;
}
