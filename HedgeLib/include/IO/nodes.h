#ifndef HNODES_H_INCLUDED
#define HNODES_H_INCLUDED
#include "file.h"
#include <cstdint>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <iostream> // TODO: REMOVE ME

namespace HedgeLib::IO
{
	template<typename T>
	class NodePointer
	{
		std::uint8_t* nodeMem = nullptr;
		T* nodePtr = nullptr;

		inline void ReadNode(const File& file, std::size_t size, std::size_t off)
		{
			if (size < sizeof(T))
				throw std::runtime_error("Could not create node; invalid size!");

			std::cout << "[ReadNode: Reading " << size << " bytes]" << std::endl;

			// Delete if necessary
			if (!nodeMem && nodePtr)
				delete nodePtr;

			// Read data into nodeMem
			nodeMem = new std::uint8_t[size];
			nodePtr = reinterpret_cast<T*>(nodeMem + off);

			if (!file.Read(nodePtr, size - off, 1))
			{
				delete[] nodeMem;
				throw std::runtime_error("Could not read node!");
			}

			nodePtr = reinterpret_cast<T*>(nodeMem);
		}

	public:
		inline NodePointer() noexcept
		{
			nodePtr = new T();
		}

		constexpr NodePointer(std::nullptr_t) noexcept :
			nodeMem(nullptr), nodePtr(nullptr) {}

		inline void ReadNode(const File& file, std::size_t size)
		{
			ReadNode(file, size, 0);
		}

		template<typename HeaderType>
		inline void ReadNode(const File& file, const HeaderType& nodeHeader)
		{
			ReadNode(file, static_cast<std::size_t>(
				nodeHeader.Size()), sizeof(nodeHeader));

			// Copy nodeHeader into nodeMem
			const std::uint8_t* nodeHeaderPtr = reinterpret_cast<
				const std::uint8_t*>(&nodeHeader);

			std::copy(nodeHeaderPtr, nodeHeaderPtr +
				sizeof(nodeHeader), &nodeMem[0]);
		}

		inline NodePointer(const File& file, std::size_t size)
		{
			ReadNode(file, size);
		}

		template<typename HeaderType>
		inline NodePointer(const File& file, const HeaderType& nodeHeader)
		{
			ReadNode(file, nodeHeader);
		}

		inline ~NodePointer() noexcept
		{
			std::cout << "[~NodePointer()]" << std::endl;
			if (!nodeMem && nodePtr)
				delete nodePtr;
		}

		inline T* Get() const noexcept
		{
			return nodePtr;
		}

		inline operator T*() const noexcept
		{
			return this->Get();
		}

		inline T& operator* () const noexcept
		{
			return *(this->Get());
		}

		inline T* operator-> () const noexcept
		{
			return this->Get();
		}

		inline T& operator[] (const int index) const noexcept
		{
			return (this->Get())[index];
		}
	};
}
#endif