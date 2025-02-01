#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

#ifndef LABA3_LIST_STACK_H
#define LABA3_LIST_STACK_H

template< typename T >
class Stack_list
{
  private:
	struct Node
	{
		T data;
		Node* next;
	};
	using pointer = std::unique_ptr< T >;
	Node* root;
	size_t stack_size;

	void clear()
	{
		while (root)
		{
			Node* temp = root;
			root = root->next;
			delete temp;
		}
		stack_size = 0;
	}

  public:
	Stack_list() : root(nullptr), stack_size(0) {}
	~Stack_list() { clear(); }
	Stack_list(const Stack_list& other) : root(nullptr), stack_size(0)
	{
		if (other.root)
		{
			Node* current = other.root;
			Node* prev = nullptr;
			while (current)
			{
				Node* new_node = new Node{ current->data, nullptr };
				if (prev)
				{
					prev->next = new_node;
				}
				else
				{
					root = new_node;
				}
				prev = new_node;
				current = current->next;
			}
			stack_size = other.stack_size;
		}
	}
	Stack_list& operator=(const Stack_list& other)
	{
		if (this == &other)
		{
			return *this;
		}
		clear();
		if (other.root)
		{
			Node* current = other.root;
			Node* prev = nullptr;
			while (current)
			{
				Node* new_node = new Node{ current->data, nullptr };
				if (prev)
				{
					prev->next = new_node;
				}
				else
				{
					root = new_node;
				}
				prev = new_node;
				current = current->next;
			}
			stack_size = other.stack_size;
		}
		return *this;
	}
	Node* top() { return root; }
	T pop()
	{
		if (!root)
		{
			throw std::out_of_range("empty");
		}
		Node* pop_node = root;
		T a = pop_node->data;
		root = pop_node->next;
		delete pop_node;
		--stack_size;
		return a;
	}
	void push(const T& value)
	{
		Node* new_node = new Node{ value, root };
		root = new_node;
		++stack_size;
	}
	size_t size() const { return stack_size; }
	bool empty() const { return size() == 0; }
};

#endif	  // LABA3_LIST_STACK_H
