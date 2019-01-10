#ifndef HNODES_H_INCLUDED
#define HNODES_H_INCLUDED
#include "file.h"
#include <cstdint>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <iostream> // TODO: REMOVE ME

namespace HedgeLib::IO
{
	// A node is any blob of data that goes like this:

	//struct NodeHeader
	//{
	//	// Returns the size of the node in its entirety, including this header.
	//	std::uint32_t Size();
	//};

	//struct Node
	//{
	//	NodeHeader Header;
	//	[Whatever data you want]
	//};

	template<class T>
	void DestroyNode(T* node)
	{
		std::cout << "[DestroyNode: Freeing " <<
			node->Header.Size() << " bytes]" << std::endl;

		if (node->Header.Size())
		{
			delete[] reinterpret_cast<std::uint8_t*>(node);
		}
		else
		{
			delete node;
		}
	}

	template<typename T>
	struct NodeDeleter
	{
		void operator()(T* b) { DestroyNode<T>(b); }
	};

	template <typename T, typename deleter = NodeDeleter<T>>
	using NodePointer = std::unique_ptr<T, deleter>;

	template<typename T, typename deleter = NodeDeleter<T>>
	NodePointer<T, deleter> CreateNode(const std::uint32_t size = sizeof(T))
	{
		if (size < sizeof(T))
			throw std::runtime_error("Could not create node; invalid size!");

		std::cout << "[CreateNode: Allocating " << size << " bytes]" << std::endl;

		// Allocate memory for node (using sort of a "C++ malloc")
		std::uint8_t* nodeMem = new std::uint8_t[size];

		// Do a placement new on the allocated memory which calls the
		// constructor, therefore initializing the node
		T* node = new (nodeMem) T();

		// Set node size and return node
		*reinterpret_cast<std::uint32_t*>(nodeMem + node->Header.SizeOffset) = size;
		return NodePointer<T, deleter>(node);
	}

	template<typename HeaderType, typename T, typename deleter = NodeDeleter<T>>
	NodePointer<T, deleter> ReadNode(const HedgeLib::IO::File& file,
		const HeaderType& nodeHeader)
	{
		if (nodeHeader.Size() < sizeof(T))
			throw std::runtime_error("Could not create node; invalid size!");

		std::cout << "[ReadNode: Reading " << nodeHeader.Size() <<
			" bytes]" << std::endl;

		std::uint8_t* nodeMem = new std::uint8_t[nodeHeader.Size()];
		if (!file.Read(nodeMem + sizeof(nodeHeader),
			nodeHeader.Size() - sizeof(nodeHeader), 1))
		{
			delete[] nodeMem;
			throw std::runtime_error("Could not read node!");
		}

		const std::uint8_t* nodePtr = reinterpret_cast<
			const std::uint8_t*>(&nodeHeader);

		std::copy(nodePtr, nodePtr + sizeof(nodeHeader), &nodeMem[0]);
		return NodePointer<T, deleter>(reinterpret_cast<T*>(nodeMem));
	}

	template<typename HeaderType, typename T, typename deleter = NodeDeleter<T>>
	NodePointer<T, deleter> ReadNode(const HedgeLib::IO::File& file)
	{
		HeaderType nodeHeader = {};
		if (!file.Read(&nodeHeader, sizeof(nodeHeader), 1))
			throw std::runtime_error("Could not read node header!");

		return ReadNode(file, nodeHeader);
	}
}
#endif