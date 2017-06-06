#include "Tlv.h"
#include "Exceptions.h"

#include <cstring>
#include <iostream>

using namespace std;

Tlv::TlvNode::TlvNode(const unsigned int tag, const unsigned char size, const unsigned char * const data) :
	tag_(tag), size_(size), global_size_(size), next_sibling_(nullptr), first_child_(nullptr) {

	data_ = new unsigned char[size_];
	memcpy(data_, data, size_);
}

Tlv::TlvNode::~TlvNode() {
	delete[] data_;
}

Tlv::TlvNode::TlvNode(const unsigned int tag, const unsigned char gsize) : tag_(tag), size_(0), global_size_(gsize),
	data_(nullptr), next_sibling_(nullptr), first_child_(nullptr)
{}

Tlv::Tlv() : head_(nullptr), last_(nullptr)
{}

Tlv::Tlv(const unsigned char * const data, const unsigned char size) : head_(nullptr), last_(nullptr) {
	add(kDummyTag, 1, size, data);
	TlvNode * old_head = head_;
	head_ = head_->first_child_;
	delete old_head; // dummy node
}

Tlv::~Tlv() {
	deleteNode(head_);
}

void Tlv::deleteNode(const TlvNode * const node) {
	if(!node) {
		return;
	}

	if(node->first_child_) {
		deleteNode(node->first_child_);
	}

	if(node->next_sibling_) {
		deleteNode(node->next_sibling_);
	}

	delete node;
}

bool Tlv::isDataProper(const unsigned char size, const unsigned char * const data) const {
	int i = 0;

	while(i + kHeaderOffset < size) {
		if(data[i + kEmbeddedTagsFlagPos]) {
			if(!isDataProper(data[i + kSizePos], data + i + kHeaderOffset))
				return false;
		}

		if(i + data[i + kSizePos] + kHeaderOffset > size)
			return false;
		else if (i + data[i + kSizePos] + kHeaderOffset == size)
			return true;

		i += kHeaderOffset + data[i + kSizePos];

	}

	return false;
}

void Tlv::add(const unsigned char tag[4], const bool embedded_tags_flag, const unsigned char size,
		const unsigned char * const data) {
	add(Tlv::getTag(tag), embedded_tags_flag, size, data);
}

void Tlv::add(const unsigned int tag, const bool embedded_tags_flag, const unsigned char size,
		const unsigned char * const data) {
	if(embedded_tags_flag && !isDataProper(size, data)) // if there are no embedded tags then data is just plain data without tags
		throw TlvException("Bad tlv input");

	if(head_ == nullptr) {
		addNode(tag, size, data, embedded_tags_flag, &head_);
		last_ = head_;
	}
	else {
		addNode(tag, size, data, embedded_tags_flag, &(last_->next_sibling_));
		last_ = last_->next_sibling_;
	}

}

inline unsigned int Tlv::getTag(const unsigned char a, const unsigned char b,
		const unsigned char c, const unsigned char d) {
	return static_cast<int>(a<<24 | b<<16 | c<<8 | d);
}

inline unsigned int Tlv::getTag(const unsigned char tag[4]) {
	return static_cast<int>(tag[0]<<24 | tag[1]<<16 | tag[2]<<8 | tag[3]);
}


void Tlv::addNode(const unsigned int tag, const unsigned char size, const unsigned char * const data,
		const bool embedded_tags_flag, TlvNode **position) {
	if(embedded_tags_flag) {
		*position = new TlvNode(tag, size);
		int offset = 0;
		TlvNode **new_position = &((*position)->first_child_);

		while(offset < size) {
			unsigned int new_tag = getTag(data[offset], data[offset + 1], data[offset + 2], data[offset + 3]); // first 4 chars are tag
			addNode(new_tag, data[offset + kSizePos], data + kHeaderOffset + offset,
				data[kEmbeddedTagsFlagPos], new_position);

			new_position = &((*new_position)->next_sibling_);
			offset += data[offset + kSizePos] + kHeaderOffset;
		}

	}
	else {
		*position = new TlvNode(tag, size, data);
	}
}

vector<unsigned char> Tlv::getTagData(const unsigned char tag[4]) const {
	return getTagData(Tlv::getTag(tag));
}

vector<unsigned char> Tlv::getTagData(const unsigned int tag) const {
	const TlvNode * const parent = findNode(head_, tag);
	vector<unsigned char> buffer;

	if(!parent)
		return buffer;

	// adding data without header of the parent tag
	if(parent->first_child_) {
		getFullBuffer(parent->first_child_, buffer);
	}
	else {
		pushDataToBuffer(parent, buffer);
	}

	return buffer;
}

vector<unsigned char> Tlv::getAllData() const {
	vector<unsigned char> buffer;

	if(!head_)
		return buffer;

	getFullBuffer(head_, buffer);

	return buffer;
}

//pushes only data to buffer, usefull when I use getTagData
void Tlv::pushDataToBuffer(const TlvNode * const node, vector<unsigned char> &buffer) const {
	int size = node->size_;
	buffer.reserve(buffer.size() + size);
	copy(&((node->data_)[0]), &((node->data_)[size]), back_inserter(buffer));
}
//pushes size and calls pushDataToBuffer to push data, usefull when I use getAllData
void Tlv::pushSizeAndDataToBuffer(const TlvNode * const node, vector<unsigned char> &buffer) const {
	buffer.push_back(node->global_size_);
	pushDataToBuffer(node, buffer);
}

void Tlv::pushTagToBuffer(const unsigned int tag, vector<unsigned char> &buffer) const {
	buffer.push_back(static_cast<unsigned char>(((tag >> (24)) & 0xff))); // get 4 bytes from int
	buffer.push_back(static_cast<unsigned char>(((tag >> (16)) & 0xff)));
	buffer.push_back(static_cast<unsigned char>(((tag >> (8)) & 0xff)));
	buffer.push_back(static_cast<unsigned char>((tag & 0xff)));
}

void Tlv::getFullBuffer(const TlvNode * const node, vector<unsigned char> &buffer) const {
	pushTagToBuffer(node->tag_, buffer);

	if(node->first_child_) {
		buffer.push_back(kHasChildren);
		pushSizeAndDataToBuffer(node, buffer);
		getFullBuffer(node->first_child_, buffer);
	}
	else {
		buffer.push_back(kHasNoChildren);
		pushSizeAndDataToBuffer(node, buffer);
	}

	if(node->next_sibling_) {
		getFullBuffer(node->next_sibling_, buffer);
	}

}

bool Tlv::isTagPresent(const unsigned int tag) const 
{
	if(findNode(head_, tag) != nullptr)
		return true;
	else 
		return false;
}

bool Tlv::isTagPresent(const unsigned char tag[4]) const
{
	return isTagPresent(getTag(tag));
}

const Tlv::TlvNode * Tlv::findNode(const TlvNode * const node, const unsigned int tag) const {
	const TlvNode *return_node;

	if(!node) {
		return nullptr;
	}

	if(tag == node->tag_) {
		return node;
	}

	return_node = findNode(node->first_child_, tag);

	if(return_node) {
		return return_node;
	}

	return_node = findNode(node->next_sibling_, tag);
	if(return_node) {
		return return_node;
	}

	return nullptr;
}
