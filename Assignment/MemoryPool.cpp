#pragma once
#include <cstddef>
#include <vector>
#include <stack>

template <typename T>
class MemoryPool {
private:
    std::vector<T*> blocks;
    std::stack<T*> freeList;
    size_t blockSize;

    void allocateBlock() {
        T* block = static_cast<T*>(::operator new(sizeof(T) * blockSize));
        blocks.push_back(block);
        for (size_t i = 0; i < blockSize; ++i)
            freeList.push(block + i);
    }

public:
    explicit MemoryPool(size_t blockSize_ = 1024)
        : blockSize(blockSize_) {
        allocateBlock();
    }

    ~MemoryPool() {
        for (T* block : blocks)
            ::operator delete(block);
    }

    template <typename... Args>
    T* allocate(Args&&... args) {
        if (freeList.empty())
            allocateBlock();

        T* obj = freeList.top();
        freeList.pop();
        new (obj) T(std::forward<Args>(args)...);
        return obj;
    }

    void deallocate(T* obj) {
        if (!obj)
            return;
        obj->~T();
        freeList.push(obj);
    }
};
