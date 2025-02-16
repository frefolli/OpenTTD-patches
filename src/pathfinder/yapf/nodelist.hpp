/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file nodelist.hpp List of nodes used for the A-star pathfinder. */

#ifndef NODELIST_HPP
#define NODELIST_HPP

#include "../../core/arena_alloc.hpp"
#include "../../misc/hashtable.hpp"
#include "../../misc/binaryheap.hpp"

/**
 * Hash table based node list multi-container class.
 *  Implements open list, closed list and priority queue for A-star pathfinder.
 */
template <class Titem>
class NodeList {
public:
	using Item = Titem;
	using Key = typename Titem::Key;

protected:
	BumpAllocContainer<Titem, 4096> items; ///< Here we store full item data (Titem).
	HashTable<Titem> open_nodes;           ///< Hash table of pointers to open item data.
	HashTable<Titem> closed_nodes;         ///< Hash table of pointers to closed item data.
	CBinaryHeapT<Titem> open_queue;        ///< Priority queue of pointers to open item data.
	Titem *new_node;                       ///< New open node under construction.

public:
	/** default constructor */
	NodeList() : open_queue(2048)
	{
		this->new_node = nullptr;
	}

	/** return number of open nodes */
	inline int OpenCount()
	{
		return (int)this->open_nodes.Count();
	}

	/** return number of closed nodes */
	inline int ClosedCount()
	{
		return (int)this->closed_nodes.Count();
	}

	/** return the total number of nodes. */
	inline int TotalCount()
	{
		return this->items.Length();
	}

	/** allocate new data item from items */
	inline Titem &CreateNewNode()
	{
		if (this->new_node == nullptr) this->new_node = this->items.New();
		return *this->new_node;
	}

	/** Notify the nodelist that we don't want to discard the given node. */
	inline void FoundBestNode(Titem &item)
	{
		/* for now it is enough to invalidate this->new_node if it is our given node */
		if (&item == this->new_node) {
			this->new_node = nullptr;
		}
		/* TODO: do we need to store best nodes found in some extra list/array? Probably not now. */
	}

	/** insert given item as open node (into this->open_nodes and this->open_queue) */
	inline void InsertOpenNode(Titem &item)
	{
		dbg_assert(this->closed_nodes.Find(item.GetKey()) == nullptr);
		this->open_nodes.Push(item);
		this->open_queue.Include(&item);
		if (&item == this->new_node) {
			this->new_node = nullptr;
		}
	}

	/** return the best open node */
	inline Titem *GetBestOpenNode()
	{
		if (!this->open_queue.IsEmpty()) {
			return this->open_queue.Begin();
		}
		return nullptr;
	}

	/** remove and return the best open node */
	inline Titem *PopBestOpenNode()
	{
		if (!this->open_queue.IsEmpty()) {
			Titem *item = this->open_queue.Shift();
			this->open_nodes.Pop(*item);
			return item;
		}
		return nullptr;
	}

	inline void DequeueBestOpenNode()
	{
		dbg_assert(!this->open_queue.IsEmpty());
		this->open_queue.Shift();
	}

	inline void ReenqueueOpenNode(Titem &item)
	{
		this->open_queue.Include(&item);
	}

	inline Titem &PopAlreadyDequeuedOpenNode(const Key &key)
	{
		return this->open_nodes.Pop(key);
	}

	/** return the open node specified by a key or nullptr if not found */
	inline Titem *FindOpenNode(const Key &key)
	{
		return this->open_nodes.Find(key);
	}

	/** remove and return the open node specified by a key */
	inline Titem &PopOpenNode(const Key &key)
	{
		Titem &item = this->open_nodes.Pop(key);
		size_t index = this->open_queue.FindIndex(item);
		this->open_queue.Remove(index);
		return item;
	}

	/** close node */
	inline void InsertClosedNode(Titem &item)
	{
		dbg_assert(this->open_nodes.Find(item.GetKey()) == nullptr);
		this->closed_nodes.Push(item);
	}

	/** return the closed node specified by a key or nullptr if not found */
	inline Titem *FindClosedNode(const Key &key)
	{
		return this->closed_nodes.Find(key);
	}

	/** Helper for creating output of this array. */
	template <class D>
	void Dump(D &dmp) const
	{
		dmp.WriteStructT("data", &this->items);
	}
};

#endif /* NODELIST_HPP */
