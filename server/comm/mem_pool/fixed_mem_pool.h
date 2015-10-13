#ifndef FIXED_MEM_POOL_H_
#define FIXED_MEM_POOL_H_

#include <stdint.h>
#include <stddef.h>
#include <string.h>

// 双向链表结构定义
typedef struct LinkedList
{
    size_t next;
    size_t prev;
    LinkedList(): next(0), prev(0) {}
} LinkedList;

class FixedMemPool
{

public:
    static const size_t INVALID_POINTER = static_cast<size_t> (-1);

    typedef struct AllocBlock
    {
        /*
         * next说明：
         * 0 则表示为已分配使用的Block
         * INVALID_POINTER 表示空闲链结尾
         * 否则 next 值用来链接空闲链，表示下一个空闲节点
         */
        size_t next;
        char node[0];
    } AllocBlock;

    static const size_t NODE_OFFSET = offsetof(AllocBlock, node);

public:
    typedef struct MemHeader
    {
        size_t max_size; // 最大可分配大小
        uint32_t max_node; // 最大节点数
        size_t alloc_size; // 已分配大小
        uint32_t alloc_node; // 已分配节点数
        size_t node_size; // 节点实际大小
        size_t block_size; // 分配块大小
        size_t free_list; // 空闲链
    } MemHeader;

    FixedMemPool() :
        header_(NULL), data_(NULL)
    {
        memset(error_msg_, 0, sizeof(error_msg_));
    }

    virtual ~FixedMemPool()
    {
    }

    inline size_t Ref(void* p) const
    {
        return (size_t)((intptr_t)p - (intptr_t)(header_));
    }

    inline void* Deref(size_t pos) const
    {
        return ((char*)header_ + pos);
    }

    static size_t GetMaxSize(uint32_t max_node, size_t node_size)
    {
        return sizeof(MemHeader) + max_node * GetBlockSize(node_size);
    }

    size_t GetMaxSize()
    {
        return GetMaxSize(header_->max_node, header_->node_size);
    }

    inline const MemHeader* Header() const
    {
        return header_;
    }

    inline const char* GetErrorMsg() const
    {
        return error_msg_;
    }

    //计算内存的使用率,百分比
    inline int GetMemUtilization() const
    {
        return (header_->alloc_node * 100) / header_->max_node;
    }

    inline int GetAllocNodeCount() const
    {
        return header_->alloc_node;
    }

    int Init(void* mem, uint32_t max_node, size_t node_size, bool check_header = false);

    void* Alloc(bool zero = true);

    int Free(void* node);

    int Clear();

protected:
    static size_t GetBlockSize(size_t node_size)
    {
        // 分配块大小按照8字节对齐
        return ((node_size + NODE_OFFSET + 7) & ~7);
    }

    size_t GetBlockSize()
    {
        return GetBlockSize(header_->node_size);
    }

    static bool CheckHeader(uint32_t max_node, size_t node_size, MemHeader* header)
    {
        size_t max_size = GetMaxSize(max_node, node_size);
        // 验证内存头部信息是否正确
        if ((header->node_size != node_size) || (header->block_size != GetBlockSize(node_size))
            || (header->max_node != max_node) || (header->max_size != max_size)
            || (sizeof(MemHeader) + header->alloc_node * header->block_size != header->alloc_size)
            || (header->alloc_size > max_size))
        {
            return false;
        }

        return true;
    }


    bool IsValidBlock(const AllocBlock* block) const
    {
        return !(block < data_ || (size_t)block + header_->block_size > (size_t)header_
            + header_->alloc_size || Ref(const_cast<AllocBlock*> (block)) % header_->block_size
            != sizeof(MemHeader) % header_->block_size);
    }

    bool IsUsedBlock(const AllocBlock* block) const
    {
        return (IsValidBlock(block) && block->next == 0);
    }

    void LinkFreeBlock(AllocBlock* p)
    {
        p->next = header_->free_list;
        header_->free_list = Ref(p);
    }

    AllocBlock* GetFreeBlock()
    {
        AllocBlock* p = NULL;
        if (header_->free_list == INVALID_POINTER)
        {
            p = NULL;
        }
        else
        {
            p = (AllocBlock*)Deref(header_->free_list);
            header_->free_list = p->next;
        }

        return p;
    }

public:
    AllocBlock* GetBlock(const void* node) const
    {
        return (AllocBlock*)((char*)node - NODE_OFFSET);
    }

    const AllocBlock* GetFirstBlock() const
    {
        const AllocBlock* block = (AllocBlock*)data_;
        return (IsValidBlock(block) ? block : NULL);
    }

    const AllocBlock* GetNextBlock(const AllocBlock* block) const
    {
        if (!block) return GetFirstBlock();

        const AllocBlock* next = (AllocBlock*)((char*)block + header_->block_size);
        return (IsValidBlock(next) ? next : NULL);
    }

    const AllocBlock* GetNextUsedBlock(const AllocBlock* block = NULL) const
    {
        while ((block = GetNextBlock(block)))
        {
        	if (block->next == 0)
        	{
        		return block;
        	}
        }

        return NULL;
    }


protected:
    MemHeader* header_;
    void* data_;
    char error_msg_[256];
};


#endif /* FIXED_MEM_POOL_H_ */
