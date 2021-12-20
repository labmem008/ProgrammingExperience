#include <vector>
#include "hw5.h"
#include <utility>      
using namespace std;
// (a)
void bubble_down(int x, std::vector<int> &min_heap) {
	int size = min_heap.size();
	int smallest = x;
	int left = 2 * x + 1;
	int right = 2* x + 2;
	if (left < size && min_heap[smallest] > min_heap[left])
		smallest = left;
	if (right < size && min_heap[smallest] > min_heap[right])
		smallest = right;
	if (smallest != x) {
		swap(min_heap[x], min_heap[smallest]);
		bubble_down(smallest, min_heap);
	}
}
std::vector<int> findMinK(const int k, const std::vector<int> &min_heap) {
	vector<int> answer;
	vector<int> v = min_heap;
	for (int i = 0; i < k; i++) {
		answer.push_back(v[0]);
		v[0] = v[v.size() - 1];
		v.pop_back();
		bubble_down(0, v);
	}	
	return answer;
}
// (b)
class GoodHashFun_0 : public HashFun {
    unsigned int operator()(const std::string& key) {
		unsigned int seed = 131; /* 31 131 1313 13131 131313 etc.. */
   unsigned int hash = 0;
   unsigned int i    = 0;
   unsigned int length = key.length();
   const char* str = key.c_str();

   for (i = 0; i < length; ++str, ++i)
   {
      hash = (hash * seed) + (*str);
   }

   return hash;
    }
};
class GoodHashFun_1 : public HashFun {
    unsigned int operator()(const std::string& key) {
        unsigned int hash = 0xAAAAAAAA;
		unsigned int length = key.length();
   const char* str = key.c_str();
   unsigned int i    = 0;

   for (i = 0; i < length; ++str, ++i)
   {
      hash ^= ((i & 1) == 0) ? (  (hash <<  7) ^ (*str) * (hash >> 3)) :
                               (~((hash << 11) + ((*str) ^ (hash >> 5))));
   }

   return hash;
	}
};
// ...
// You can make as many hash functions as you want!
// ...
void BloomFilter::add_hash_funs() {
    // You will need to implement this function, it is responsible
    // for adding your custom hash functions to your bloom filter.
	// Here's an example where I add two hash functions.
    // You will need to add at least one hash function.
    GoodHashFun_0 *h1 = new GoodHashFun_0();
     GoodHashFun_1 *h2 = new GoodHashFun_1();
	this->hash_funs.push_back(h1);
     this->hash_funs.push_back(h2);
}
void BloomFilter::insert(const std::string& key) {
    // You will need to implement this function, it is responsible
    // for inserting key into the bloom filter.
	// Here's how you can call your hash functions that you added in add_hash_funs()
    //unsigned int first_hash_result = this->call_hash(0, key);
    // std::cout << "Hash of '" << key << "' is '" << first_hash_result << "' using the first hash function." << std::endl;
    //unsigned int second_hash_result = this->call_hash(1, key);
    // std::cout << "Hash of '" << key << "' is '" << second_hash_result << "' using the second hash function." << std::endl;
	// You'll need to use the results of the hash function to update this->filter
    // Here's an example
	for (int i = 0; i < this->hash_funs.size(); i++) {
		unsigned int result = this -> call_hash(i, key);
		(this -> filter)[result % 330] = 1;
	}
    
}
bool BloomFilter::member(const std::string& key) {
    // This function should report true if key is potentially in the bloom filter.
    // It should only return false if you know for sure that key is not in the bloom filter.
	for (int i = 0; i < this->hash_funs.size(); i++) {
		unsigned int result = this -> call_hash(i, key);
		if ((this -> filter)[result % 330] == 0)
			return false;
		
	}
	return true;
	
}
