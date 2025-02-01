#ifndef LABA3_BLOCK_H
#define LABA3_BLOCK_H

#include "list_stack.h"

#include <cstdint>
#include <cstring>
#include <utility>

template< typename T >
class BlockNode;

template< typename T >
struct Node
{
	Node* prev = nullptr;
	Node* next = nullptr;
	BlockNode< T >* block_ptr = nullptr;
	T* data = nullptr;

	Node() = default;

	void link(Node* other)
	{
		this->next = other;
		other->prev = this;
	}

	void set_block(BlockNode< T >* block) { block_ptr = block; }

	void set_data(T&& node_data)
	{
		if (data)
		{
			*data = std::move(node_data);
		}
		else
		{
			data = new T(std::move(node_data));
		}
	}
	void set_data(const T& node_data)
	{
		if (data)
		{
			*data = node_data;
		}
		else
		{
			data = new T(node_data);
		}
	}
	~Node()
	{
		delete data;
		data = nullptr;
	}
};

template< typename T >
struct BlockNode
{
	size_t block_size;
	size_t block_capacity;
	Node< T >* data;
	Stack_list< Node< T >* > stack_element;
	BlockNode< T >* next;

	BlockNode(size_t block_capacity) :
		block_size(0), block_capacity(block_capacity), data(new Node< T >[block_capacity]), next(nullptr)
	{
		for (size_t i = 0; i < block_capacity; ++i)
		{
			data[i].set_block(this);
			stack_element.push(&data[i]);
		}
	}

	void destroy_data()
	{
		for (size_t i = 0; i < block_capacity; ++i)
		{
			data[i].~Node();
		}
		delete[] data;
		data = nullptr;
	}

	~BlockNode() { destroy_data(); }
};

#endif	  // LABA3_BLOCK_H
