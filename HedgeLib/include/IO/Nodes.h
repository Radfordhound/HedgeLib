#ifndef HNODES_H_INCLUDED
#define HNODES_H_INCLUDED
#include "File.h"
#include <cstdint>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <algorithm>
#include <iostream> // TODO: REMOVE ME

namespace HedgeLib::IO
{
	template<typename T>
	class NodePointer
	{
		std::unique_ptr<std::uint8_t[]> nodeMem = nullptr;
		T* dataPtr = nullptr;

	public:
		inline NodePointer() noexcept
		{
			dataPtr = new T();
		}

		constexpr NodePointer(std::nullptr_t) noexcept :
			nodeMem(nullptr), dataPtr(nullptr) {}

		inline void ReadNode(const File& file,
			std::size_t size, std::size_t off)
		{
			if (size < sizeof(T))
				throw std::runtime_error("Could not create node; invalid size!");

			std::cout << "[ReadNode: Reading " << size << " bytes]" << std::endl;

			// Delete if necessary
			if (!nodeMem && dataPtr)
				delete dataPtr;

			// Read data into nodeMem
			nodeMem = std::make_unique<std::uint8_t[]>(size);
			dataPtr = reinterpret_cast<T*>(nodeMem.get() + off);

			if (!file.Read(dataPtr, size - off, 1))
				throw std::runtime_error("Could not read node!");
		}

		template<typename HeaderType>
		inline void CopyHeader(const HeaderType& nodeHeader)
		{
			// Copy nodeHeader into nodeMem
			const std::uint8_t* nodeHeaderPtr = reinterpret_cast<
				const std::uint8_t*>(&nodeHeader);

			std::copy(nodeHeaderPtr, nodeHeaderPtr +
				sizeof(nodeHeader), nodeMem.get());
		}

		template<typename HeaderType>
		inline void ReadNode(const File& file, const HeaderType& nodeHeader)
		{
			ReadNode(file, static_cast<std::size_t>(
				nodeHeader.Size()), sizeof(nodeHeader));

			CopyHeader(nodeHeader);
			dataPtr = reinterpret_cast<T*>(nodeMem.get() +
				nodeHeader.Origin);
		}

		template<template<typename> class OffsetType, typename HeaderType>
		inline void FixOffsets(const bool swapEndianness = false)
		{
			// Fix offsets
			HeaderType* headerPtr = reinterpret_cast<HeaderType*>(nodeMem.get());
			headerPtr->template FixOffsets<OffsetType>(swapEndianness);
		}

		inline NodePointer(const File& file,
			std::size_t size, std::size_t off)
		{
			ReadNode(file, size, off);
		}

		template<typename HeaderType>
		inline NodePointer(const File& file, const HeaderType& nodeHeader)
		{
			ReadNode(file, nodeHeader);
		}

		inline ~NodePointer() noexcept
		{
			std::cout << "[~NodePointer()]" << std::endl;
			if (!nodeMem && dataPtr)
				delete dataPtr;
		}

		inline T* Get() const noexcept
		{
			return dataPtr;
		}

		inline std::uint8_t* GetRawMemory() const noexcept
		{
			return nodeMem.get();
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