#ifndef LABA3_LIST_ITERATOR_H
#define LABA3_LIST_ITERATOR_H

#include "block.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>

template< typename T >
class BucketStorage;

template< typename T >
class list_iterator
{
  public:
	using iterator = list_iterator< T >;
	using iterator_const = list_iterator< const T >;
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = T;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;

  private:

  public:
	list_iterator(Node< T >* node) : node(node) {}
	Node< T >* node;
	friend class list_iterator< const T >;
	friend BucketStorage< T >;
	friend BucketStorage< std::remove_const_t< T > >;
	friend BucketStorage< const T >;

	operator list_iterator< const T >() const { return list_iterator< const T >(node); }

	list_iterator& operator++()
	{
		node = node->next;
		return *this;
	}

	list_iterator operator++(int)
	{
		list_iterator tmp = *this;
		++(*this);
		return tmp;
	}

	list_iterator& operator--()
	{
		node = node->prev;
		return *this;
	}

	list_iterator operator--(int)
	{
		list_iterator tmp = *this;
		--(*this);
		return tmp;
	}

	template< typename U >
	bool operator==(const list_iterator< U >& other) const
	{
		return static_cast< Node< T >* >(this->node) == (Node< T >*)other.node;
	}

	bool operator!=(const list_iterator& other) const { return !(*this == other); }

	friend bool operator>(const list_iterator& a, const list_iterator& b) { return a.node > b.node; }

	friend bool operator<(const list_iterator& a, const list_iterator& b) { return a.node < b.node; }

	friend bool operator<=(const list_iterator& a, const list_iterator& b) { return !(a > b); }

	friend bool operator>=(const list_iterator& a, const list_iterator& b) { return !(a < b); }

	T& operator*() const { return *node->data; }

	T* operator->() const { return node->data; }

	list_iterator& operator=(const list_iterator& it_2)
	{
		if (this != &it_2)
		{
			node = it_2.node;
		}
		return *this;
	}
};

#endif	  // LABA3_LIST_ITERATOR_H
