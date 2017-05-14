/*
author Michal Citko

tlv buffer consists of a header and data. Header has tag, flag that tells if there are tags in data and data size.
Use example, you create Tlv object with default constructor, serialize classes and add them to Tlv object with
add method, then you use getAllData to get vector with whole Tlv buffer, you send it to someone.
Other person receives the buffer, creates Tlv object using constructor with data and size, then he can use
findTagData to find data of a certain tag(it returns only data without header).

*/
#pragma once
#include <cstdio>
#include <vector>
#include <memory>

class Tlv
{
	public:
		Tlv();
		Tlv(const unsigned char * const data, const unsigned char size);
		~Tlv();
		void add(const unsigned int tag, const bool embedded_tags_flag, const unsigned char size,
				const unsigned char * const data);
		//std::vector<unsigned char> get(const unsigned int tag);	
		std::vector<unsigned char> findTagData(const unsigned int tag) const;
		std::vector<unsigned char> getAllData() const;
	private:
		struct TlvNode
		{
			TlvNode(const unsigned int tag, const unsigned char size, const unsigned char * const data); 
			TlvNode(const unsigned int tag, const unsigned char gsize);
			~TlvNode();
			
			TlvNode *next_sibling_;
			TlvNode *first_child_;
			const unsigned int tag_;
			const unsigned char size_;
			const unsigned char global_size_;
			unsigned char *data_;
		};
		const unsigned int getTag(const unsigned char a, const unsigned char b, 
				const unsigned char c) const;

		void addNode(const unsigned int tag, const unsigned char size, const unsigned char * const data, 
			const bool embedded_tags_flag, TlvNode **position);

		const TlvNode * const findNode(const TlvNode * const node, const unsigned int tag) const;
		void getFullBuffer(const TlvNode * const node, std::vector<unsigned char> &buffer) const;
		void pushToBuffer(const TlvNode * const node, std::vector<unsigned char> &buffer) const;
		void pushTagToBuffer(const unsigned int tag, std::vector<unsigned char> &buffer) const;
		void pushDataToBuffer(const TlvNode * const node, std::vector<unsigned char> &buffer) const;
		void deleteNode(const TlvNode * const node);

		TlvNode *head_;
		TlvNode *last_;
		const int EMBEDDED_TAGS_FLAG_POS = 3;
		const int SIZE_POS = 4;
		const int HEADER_OFFSET = 5;
		const unsigned int DUMMY_TAG = 5;
		const unsigned char HAS_CHILDREN = 1;
		const unsigned char HAS_NO_CHILDREN = 0;
};
