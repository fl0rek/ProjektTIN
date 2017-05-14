#include <cstring>
#include <iostream>
#include "TlvObject.h"

using namespace std;

Tlv::TlvNode::TlvNode(const unsigned int tag, const unsigned char size, const unsigned char * const data) : 
	tag_(tag), size_(size), global_size_(size), next_sibling_(nullptr), first_child_(nullptr)
{
	data_ = new unsigned char[size_]; 
	memcpy(data_, data, size_);
}	

Tlv::TlvNode::~TlvNode()
{
	delete[] data_;
}

Tlv::TlvNode::TlvNode(const unsigned int tag, const unsigned char gsize) : tag_(tag), next_sibling_(nullptr),
       	first_child_(nullptr), size_(0), global_size_(gsize), data_(nullptr)
{}	

Tlv::Tlv() : head_(nullptr), last_(nullptr)
{}

Tlv::Tlv(const unsigned char * const data, const unsigned char size) : head_(nullptr), last_(nullptr)
{
	add(DUMMY_TAG, 1, size, data);
	TlvNode * old_head = head_;
	head_ = head_->first_child_;
	delete old_head;
}

Tlv::~Tlv()
{
	deleteNode(head_);
}

void Tlv::deleteNode(const TlvNode * const node)
{
	if(!node)
	{
		return; 
	}
	
	if(node->first_child_)
	{
		deleteNode(node->first_child_);
	}

	if(node->next_sibling_)
	{
		deleteNode(node->next_sibling_);
	}

	delete node;
}

void Tlv::add(const unsigned int tag, const bool embedded_tags_flag, const unsigned char size, 
		const unsigned char * const data) 
{
	if(head_ == nullptr)
	{
		addNode(tag, size, data, embedded_tags_flag, &head_);
		last_ = head_;
	}
	else
	{
		addNode(tag, size, data, embedded_tags_flag, &(last_->next_sibling_));
		last_ = last_->next_sibling_;
	}

}

inline const unsigned int Tlv::getTag(const unsigned char a, const unsigned char b, 
		const unsigned char c) const
{
	return static_cast<int>(a<<16 | b<<8 | c);
}

void Tlv::addNode(const unsigned int tag, const unsigned char size, const unsigned char * const data, 
		const bool embedded_tags_flag, TlvNode **position)
{
	if(embedded_tags_flag)
	{
		*position = new TlvNode(tag, size);
		int offset = 0;
		TlvNode **new_position = &((*position)->first_child_);

		while(offset < size)
		{
			unsigned int new_tag = getTag(data[offset], data[offset + 1], data[offset + 2]); // first 3 chars are tag
			addNode(new_tag, data[offset + SIZE_POS], data + HEADER_OFFSET + offset,
				data[EMBEDDED_TAGS_FLAG_POS], new_position);

			new_position = &((*new_position)->next_sibling_);
			offset += data[offset + SIZE_POS] + HEADER_OFFSET;
		}

	}
	else
	{
		*position = new TlvNode(tag, size, data);	
	}
}

vector<unsigned char> Tlv::findTagData(const unsigned int tag) const
{
	const TlvNode * const parent = findNode(head_, tag);
       	vector<unsigned char> buffer;

	if(!parent)
		return buffer;

	// adding data without header of the parent tag
	if(parent->first_child_)  
	{
		getFullBuffer(parent->first_child_, buffer);
	}
	else
	{
		pushDataToBuffer(parent, buffer);
	}

	return buffer;
}

vector<unsigned char> Tlv::getAllData() const
{
	vector<unsigned char> buffer;

	if(!head_)
		return buffer;
	
	getFullBuffer(head_, buffer); 

	return buffer;
}

void Tlv::pushDataToBuffer(const TlvNode * const node, vector<unsigned char> &buffer) const
{
	int size = node->size_;
	for(int i = 0; i < size; ++i)
	{
		buffer.push_back((node->data_)[i]);
	}

}
void Tlv::pushToBuffer(const TlvNode * const node, vector<unsigned char> &buffer) const
{
	int size = node->size_;
	buffer.push_back(node->global_size_);
	pushDataToBuffer(node, buffer);
}

void Tlv::pushTagToBuffer(const unsigned int tag, vector<unsigned char> &buffer) const
{
	buffer.push_back(static_cast<unsigned char>(((tag >> (16)) & 0xff))); // get 3 bytes from int 
	buffer.push_back(static_cast<unsigned char>(((tag >> (8)) & 0xff)));
	buffer.push_back(static_cast<unsigned char>((tag & 0xff)));
}

void Tlv::getFullBuffer(const TlvNode * const node, vector<unsigned char> &buffer) const
{
	pushTagToBuffer(node->tag_, buffer);

	if(node->first_child_)
	{
		buffer.push_back(HAS_CHILDREN);	
		pushToBuffer(node, buffer);
		getFullBuffer(node->first_child_, buffer);
	}
	else
	{
		buffer.push_back(HAS_NO_CHILDREN);	
		pushToBuffer(node, buffer);
	}

	if(node->next_sibling_)
	{
		getFullBuffer(node->next_sibling_, buffer);
	}

}

const Tlv::TlvNode * const Tlv::findNode(const TlvNode * const node, const unsigned int tag) const
{
	const TlvNode *return_node;

	if(!node)
	{
		return nullptr;
	}

	if(tag == node->tag_)
	{
		return node;
	}

	return_node = findNode(node->first_child_, tag);
	if(return_node) 
	{
		return return_node;	
	}	

	return_node = findNode(node->next_sibling_, tag);
	if(return_node) 
	{
		return return_node;
	}

	return nullptr;
}
