#pragma once

#include <cstdio>
#include <vector>
#include <memory>
/*
author Michal Citko

tlv buffer consists of a header and data. Header has tag, flag that tells if there are tags in data and data size.
Use example, you create Tlv object with default constructor, serialize classes and add them to Tlv object with
add method, then you use getAllData to get vector with whole Tlv buffer, you send it to someone.
Other person receives the buffer, creates Tlv object using constructor with data and size, then he can use
getTagData to find data of a certain tag(it returns only data without header).

*/

class Tlv {
	public:
		/**
		 *	@brief
		 *		create empty Tlv object
		*/
		Tlv();

		/**
		 *	@brief
		 *		create Tlv object from data
		 *	@param data
		 *		pointer to data buffer with header and data
		 *	@param size
		 *		number of elements in data
		*/
		Tlv(const unsigned char * const data, const unsigned char size);
		~Tlv();
		/**
		 * @brief
		 *		add data associated with tag to Tlv object
		 *	@param tag
		 *		tag associated with data
		 *	@param embedded_tags_flag
		 *		1 - if there are other tags inside data 0 - if not
		 *	@param size
		 *		number of elements in data
		 *	@param data
		 *		pointer to data buffer with header and data
		*/
		void add(const unsigned int tag, const bool embedded_tags_flag, const unsigned char size,
				const unsigned char * const data);
		void add(const unsigned char tag[4], const bool embedded_tags_flag, const unsigned char size,
				const unsigned char * const data);
		/**
		 *	@brief
		 *		returns data without header that is associated with tag
		 *	@param tag
		 *		tag associated with data
		 *	@return
		 *		vector with data
		*/
		std::vector<unsigned char> getTagData(const unsigned int tag) const;
		std::vector<unsigned char> getTagData(const unsigned char tag[4]) const;
		/**
		 *	@brief
		 *		returns every vector with every element(header + data) in the object
		 *	@return
		 *		vector with header + data of every element
		*/
		std::vector<unsigned char> getAllData() const;

		static const unsigned int getTag(const unsigned char a, const unsigned char b,
				const unsigned char c, const unsigned char d);
		static const unsigned int getTag(const unsigned char tag[4]);
	private:
		/**
		 * structure representing node of Tlv Object
		*/
		struct TlvNode {
			/**
			 * 	@brief
			 * 		creates TlvNode object that has no children(because it has data in it)
			*/
			TlvNode(const unsigned int tag, const unsigned char size, const unsigned char * const data);
			/**
			 * 	@brief
			 * 		creates TlvNode object that has children so its size is 0
			*/
			TlvNode(const unsigned int tag, const unsigned char gsize);
			~TlvNode();

			TlvNode *next_sibling_; // pointer to next element
			TlvNode *first_child_; // pointer to the element embedded in this element
			const unsigned int tag_;
			const unsigned char size_; //size of data in this node
			const unsigned char global_size_; // size of data in this node and ith children
			unsigned char *data_;
		};

		void addNode(const unsigned int tag, const unsigned char size, const unsigned char * const data,
			const bool embedded_tags_flag, TlvNode **position);

		const TlvNode * const findNode(const TlvNode * const node, const unsigned int tag) const;
		void getFullBuffer(const TlvNode * const node, std::vector<unsigned char> &buffer) const;
		void pushSizeAndDataToBuffer(const TlvNode * const node, std::vector<unsigned char> &buffer) const;
		void pushTagToBuffer(const unsigned int tag, std::vector<unsigned char> &buffer) const;
		void pushDataToBuffer(const TlvNode * const node, std::vector<unsigned char> &buffer) const;
		void deleteNode(const TlvNode * const node);

		TlvNode *head_;
		TlvNode *last_;
		const int kEmbeddedTagsFlagPos = 4;
		const int kSizePos = 5;
		const int kHeaderOffset = 6;
		const unsigned int kDummyTag = 0;
		const unsigned char kHasChildren = 1;
		const unsigned char kHasNoChildren = 0;
};
