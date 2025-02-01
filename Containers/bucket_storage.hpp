#ifndef LABA3_BUCKET_STORAGE_HPP
#define LABA3_BUCKET_STORAGE_HPP

#include "block.h"
#include "list_iterator.h"
#include "list_stack.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <utility>

template< typename T >
class BucketStorage
{
	friend class Node< T >;
	friend class BlockNode< T >;

  public:
	using value_type = T;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using difference_type = ptrdiff_t;
	using const_pointer = const T*;
	using size_type = size_t;
	using iterator = list_iterator< T >;
	using const_iterator = list_iterator< const T >;

	explicit BucketStorage(size_t block_capacity = 64);
	BucketStorage(const BucketStorage< T >& other);
	BucketStorage(BucketStorage< T >&& other) noexcept;
	BucketStorage< T >& operator=(const BucketStorage< T >& other);
	BucketStorage< T >& operator=(BucketStorage< T >&& other) noexcept;

	size_t size() const noexcept;
	bool empty() const noexcept;
	void clear() noexcept;
	void compact_memory() noexcept;
	template< typename U >
	iterator insert_impl(U&& value);
	iterator insert(const value_type& value);
	iterator insert(value_type&& value);
	iterator erase(iterator it) noexcept;
	size_t capacity() const noexcept;
	void swap(BucketStorage& other) noexcept;
	void shrink_to_fit() noexcept;
	iterator get_to_distance(iterator it, const difference_type distance);

	iterator begin() noexcept;
	iterator end() noexcept;
	const_iterator begin() const noexcept;
	const_iterator end() const noexcept;
	const_iterator cbegin() const noexcept;
	const_iterator cend() const noexcept;

	~BucketStorage();

  private:
	size_t _size;
	size_t block_capacity;
	size_t total_capacity;
	using node = Node< T >;
	using block = BlockNode< T >;
	Stack_list< block > free_block;
	node* tail_node;
	void update_total_capacity();
};

template< typename T >
BucketStorage< T >::BucketStorage(size_t block_capacity) :
	block_capacity(block_capacity), _size(0), total_capacity(0), tail_node(nullptr)
{
	block* new_block = new block(block_capacity);
	tail_node = &(new_block->data[0]);
	tail_node->link(tail_node);
}

template< typename T >
bool BucketStorage< T >::empty() const noexcept
{
	return _size == 0;
}

template< typename T >
void BucketStorage< T >::clear() noexcept
{
	while (!free_block.empty())
	{
		delete free_block.top();
		free_block.pop();
	}
	while (_size != 0)
	{
		erase(begin());
	}
	_size = 0;
	total_capacity = 0;
}

template< typename T >
void BucketStorage< T >::update_total_capacity()
{
	size_t used_blocks = (_size + block_capacity - 1) / block_capacity;
	total_capacity = used_blocks * block_capacity;
	compact_memory();
}

template< typename T >
template< typename U >
typename BucketStorage< T >::iterator BucketStorage< T >::insert_impl(U&& value)
{
	compact_memory();

	if (free_block.empty())
	{
		block* new_block = new block(block_capacity);
		node* new_node = &(new_block->data[0]);

		try
		{
			new_node->set_data(std::forward< U >(value));
			tail_node->prev->link(new_node);
			new_node->link(tail_node);

			++new_block->block_size;
			++_size;
			total_capacity += block_capacity;
			update_total_capacity();
		} catch (...)
		{
			delete new_block;
			throw;
		}
	}
	else
	{
		node* empty_node = free_block.top()->data.stack_element.pop();
		tail_node->prev->link(empty_node);
		empty_node->link(tail_node);

		empty_node->set_data(std::forward< U >(value));
		++_size;
		++free_block.top()->data.block_size;
	}

	return iterator(tail_node->prev);
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::insert(const value_type& value)
{
	return insert_impl(value);
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::insert(value_type&& value)
{
	return insert_impl(std::move(value));
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::erase(iterator it) noexcept
{
	node* current_node = const_cast< node* >(it.node);
	if (current_node == tail_node)
		return it;

	iterator next_it = iterator(current_node->next);
	if (current_node->prev)
	{
		current_node->prev->link(current_node->next);
	}
	current_node->next->prev = current_node->prev;
	--current_node->block_ptr->block_size;
	--_size;

	if (current_node->block_ptr->block_size == 0)
	{
		total_capacity = block_capacity * 2;
		compact_memory();
	}

	current_node->~node();
	current_node->block_ptr = nullptr;

	return next_it;
}

template< typename T >
size_t BucketStorage< T >::size() const noexcept
{
	return _size;
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::begin() noexcept
{
	return iterator(tail_node->next);
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::end() noexcept
{
	return iterator(tail_node);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::begin() const noexcept
{
	return const_iterator(tail_node->next);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::end() const noexcept
{
	return const_iterator(tail_node);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::cbegin() const noexcept
{
	return const_iterator((Node< const value_type >*)tail_node->next);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::cend() const noexcept
{
	return const_iterator((Node< const value_type >*)tail_node);
}

template< typename T >
size_t BucketStorage< T >::capacity() const noexcept
{
	return total_capacity;
}

template< typename T >
void BucketStorage< T >::swap(BucketStorage& other) noexcept
{
	std::swap(_size, other._size);
	std::swap(block_capacity, other.block_capacity);
	std::swap(total_capacity, other.total_capacity);
	std::swap(free_block, other.free_block);
	std::swap(tail_node, other.tail_node);
}

template< typename T >
void BucketStorage< T >::shrink_to_fit() noexcept
{
	BucketStorage< T > temporary_copy = *this;
	swap(temporary_copy);
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::get_to_distance(iterator it, const difference_type distance)
{
	if (distance < 0)
	{
		for (size_t i = 0; i < static_cast< size_t >(-distance); ++i, ++it)
			;
	}
	else
	{
		for (size_t i = 0; i < static_cast< size_t >(distance); ++i, ++it)
			;
	}
	return it;
}

template< typename T >
void BucketStorage< T >::compact_memory() noexcept
{
	while (!free_block.empty())
	{
		if (free_block.top()->data.block_size == 0)
		{
			delete free_block.top();
			free_block.pop();
			total_capacity -= block_capacity;
		}
		else
		{
			break;
		}
	}
}

template< typename T >
BucketStorage< T >::BucketStorage(const BucketStorage< T >& other) :
	block_capacity(other.block_capacity), _size(0), total_capacity(0), tail_node(nullptr)
{
	if (other.empty())
	{
		return;
	}

	block* new_block = new block(other.block_capacity);
	tail_node = &(new_block->data[0]);
	tail_node->link(tail_node);

	try
	{
		node* current = other.tail_node->next;
		while (current != other.tail_node)
		{
			insert(*current->data);
			current = current->next;
		}
	} catch (...)
	{
		delete new_block;
		tail_node = nullptr;
		throw;
	}
}

template< typename T >
BucketStorage< T >::BucketStorage(BucketStorage< T >&& other) noexcept :
	_size(other._size), block_capacity(other.block_capacity), total_capacity(other.total_capacity),
	free_block(std::move(other.free_block)), tail_node(other.tail_node)
{
	other._size = 0;
	other.block_capacity = 0;
	other.total_capacity = 0;
	other.tail_node = nullptr;
}

template< typename T >
BucketStorage< T >& BucketStorage< T >::operator=(const BucketStorage< T >& other)
{
	if (this != &other)
	{
		clear();
		block_capacity = other.block_capacity;
		_size = 0;
		total_capacity = 0;
		tail_node = nullptr;

		if (!other.empty())
		{
			block* new_block = new block(other.block_capacity);
			tail_node = &(new_block->data[0]);
			tail_node->link(tail_node);

			try
			{
				node* current = other.tail_node->next;
				while (current != other.tail_node)
				{
					insert(*current->data);
					current = current->next;
				}
			} catch (...)
			{
				delete new_block;
				tail_node = nullptr;
				throw;
			}
		}
	}
	return *this;
}

template< typename T >
BucketStorage< T >& BucketStorage< T >::operator=(BucketStorage< T >&& other) noexcept
{
	if (this != &other)
	{
		clear();
		block_capacity = other.block_capacity;
		_size = other._size;
		total_capacity = other.total_capacity;
		free_block = std::move(other.free_block);
		tail_node = other.tail_node;

		other._size = 0;
		other.block_capacity = 0;
		other.total_capacity = 0;
		other.tail_node = nullptr;
	}
	return *this;
}

template< typename T >
BucketStorage< T >::~BucketStorage()
{
	clear();
	if (tail_node)
	{
		delete[] tail_node;
	}
	while (!free_block.empty())
	{
		delete free_block.top();
		free_block.pop();
	}
}

#endif	  // LABA3_BUCKET_STORAGE_HPP
