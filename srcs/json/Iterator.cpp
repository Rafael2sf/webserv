#include "Iterator.hpp"
#include "Node.hpp"

namespace JSON
{
	Iterator::Iterator( void )
	{}

	Iterator::Iterator( Node * p )
	: node(p)
	{}

	Iterator::Iterator( Iterator const& other )
	: node(other.node)
	{
		*this = other;
	}

	Iterator &
	Iterator::operator=( Iterator const& rhs )
	{
		node = rhs.node;
		return *this;
	}

	Node &
	Iterator::operator*( void )
	{
		return *node;
	}

	Node *
	Iterator::operator->( void )
	{
		return node;
	}

	Iterator	&
	Iterator::operator++( void )
	{
		Object *	obj;
		Array *		arr;

		switch (node->type())
		{
			case array:
				arr = dynamic_cast<Array *>(node);
				if (arr->impl.size() == 0)
					node = _findNextFromParent(node->getParent());
				else
					node =  (*arr->impl.begin());
				break ;
			case object:
				obj = dynamic_cast<Object *>(node);
				if (obj->impl.size() == 0)
					node = _findNextFromParent(node->getParent());
				else
					node = (*obj->impl.begin());
				break ;
			default:
				node = _findNextFromParent(node->getParent());
				break ;
		}
		return *this;
	}

	Iterator 
	Iterator::operator++( int )
	{
		Iterator	it(*this);

		++(*this);
		return (it);
	}

	Iterator	&
	Iterator::operator--( void )
	{
		return *this;
	}

	Iterator
	Iterator::operator--( int )
	{
		Iterator	it(*this);

		--(*this);
		return (it);
	}

	Node * Iterator::_findNextFromParent( Node * parent )
	{
		Object * tmp;
		Array * arr;
		std::multiset<Node *>::iterator it;
		std::vector<Node *>::iterator it2;

		if (!parent)
			return node;
		if (parent->type() == array)
		{
			arr = dynamic_cast<Array *>(parent);
			it2 = std::find(arr->impl.begin(), arr->impl.end(), node);
			if (it2 == arr->impl.end() || it2 == --arr->impl.end())
			{
				node = parent;
				node = _findNextFromParent(node->getParent());
			}
			else
				node = *(++it2);
		}
		else
		{
			tmp = dynamic_cast<Object *>(parent);
			it = tmp->impl.find(node);
			if (it == tmp->impl.end() || it == --tmp->impl.end())
			{
				node = parent;
				node = _findNextFromParent(node->getParent());
			}
			else
				node = *++it;
		}
		return node;
	}

	// TO DO
	void Iterator::skip( void )
	{
		if (!node->getParent())
			return ;
		_findNextFromParent(node->getParent());
		// if (node->type() == array)
		// {
		// 	if (dynamic_cast<Array *>(node)->impl.empty())
		// 		return ;
		// 	node = *--dynamic_cast<Array *>(node)->impl.end();
		// }
		// else if (node->type() == object)
		// {
		// 	if (dynamic_cast<Object *>(node)->impl.empty())
		// 		return ;
		// 	node = *--dynamic_cast<Object *>(node)->impl.end();
		// }
	}

	bool Iterator::operator==( Iterator const&  rhs ) const
	{
		return node == rhs.node;
	}

	bool Iterator::operator!=( Iterator const&  rhs ) const
	{
		return node != rhs.node;
	}

	constIterator::constIterator( void )
	{}

	constIterator::constIterator( Node * p )
	: node(p)
	{}

	constIterator::constIterator( Iterator const& other )
	: node(other.node)
	{
		this->node = other.node;
	}

	constIterator::constIterator( constIterator const& other )
	: node(other.node)
	{
		*this = other;
	}

	constIterator &
	constIterator::operator=( constIterator const& rhs )
	{
		node = rhs.node;
		return *this;
	}

	Node const&
	constIterator::operator*( void ) const
	{
		return *node;
	}

	Node const*
	constIterator::operator->( void ) const
	{
		return node;
	}

	constIterator	&
	constIterator::operator++( void )
	{
		Object *	obj;
		Array *		arr;

		switch (node->type())
		{
			case array:
				arr = dynamic_cast<Array *>(node);
				if (arr->impl.size() == 0)
					node = _findNextFromParent(node->getParent());
				else
					node =  (*arr->impl.begin());
				break ;
			case object:
				obj = dynamic_cast<Object *>(node);
				if (obj->impl.size() == 0)
					node = _findNextFromParent(node->getParent());
				else
					node = (*obj->impl.begin());
				break ;
			default:
				node = _findNextFromParent(node->getParent());
				break ;
		}
		return *this;
	}

	constIterator 
	constIterator::operator++( int )
	{
		constIterator	it(*this);

		++(*this);
		return (it);
	}

	constIterator	&
	constIterator::operator--( void )
	{
		return *this;
	}

	constIterator
	constIterator::operator--( int )
	{
		constIterator	it(*this);

		--(*this);
		return (it);
	}

	Node * constIterator::_findNextFromParent( Node * parent )
	{
		Object * tmp;
		Array * arr;
		std::multiset<Node *>::iterator it;
		std::vector<Node *>::iterator it2;

		if (!parent)
			return node;
		if (parent->type() == array)
		{
			arr = dynamic_cast<Array *>(parent);
			it2 = std::find(arr->impl.begin(), arr->impl.end(), node);
			if (it2 == arr->impl.end() || it2 == --arr->impl.end())
			{
				node = parent;
				node = _findNextFromParent(node->getParent());
			}
			else
				node = *(++it2);
		}
		else
		{
			tmp = dynamic_cast<Object *>(parent);
			it = tmp->impl.find(node);
			if (it == tmp->impl.end() || it == --tmp->impl.end())
			{
				node = parent;
				node = _findNextFromParent(node->getParent());
			}
			else
				node = *++it;
		}
		return node;
	}

	// TO DO
	void constIterator::skip( void )
	{
		if (!node->getParent())
			return ;
		if (node->type() == array)
		{
			if (dynamic_cast<Array *>(node)->impl.empty())
				return ;
			node = *--dynamic_cast<Array *>(node)->impl.end();
		}
		else if (node->type() == object)
		{
			if (dynamic_cast<Object *>(node)->impl.empty())
				return ;
			node = *--dynamic_cast<Object *>(node)->impl.end();
		}
	}

	bool constIterator::operator==( constIterator const&  rhs ) const
	{
		return node == rhs.node;
	}

	bool constIterator::operator!=( constIterator const&  rhs ) const
	{
		return node != rhs.node;
	}
}
