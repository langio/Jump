#include "mem_pool/fixed_mem_pool.h"
#include <stdio.h>

int FixedMemPool::Init(void* mem, uint32_t max_node, size_t node_size, bool check_header)
{
    if (!mem)
    {
        return -1;
    }

    header_ = (MemHeader*)mem;

    if (!check_header)
    {
        // 初始化内存头
        header_->max_size = GetMaxSize(max_node, node_size);
        header_->max_node = max_node;
        header_->alloc_size = sizeof(MemHeader);
        header_->alloc_node = 0;
        header_->node_size = node_size;
        header_->block_size = GetBlockSize(node_size);
        header_->free_list = INVALID_POINTER;

        data_ = (char*)header_ + sizeof(MemHeader);
    }
    else
    {
        if (!CheckHeader(max_node, node_size, header_))
        {
            snprintf(error_msg_, sizeof(error_msg_), "内存池头信息验证错误");
            return -1;
        }

        data_ = (char*)header_ + sizeof(MemHeader);
    }

    return 0;
}

void* FixedMemPool::Alloc(bool zero)
{
    AllocBlock* p = GetFreeBlock();
    if (!p)
    {
        // 没有找到空闲块，需要从池中分配内存
        if (header_->alloc_size + header_->block_size <= header_->max_size)
        {
            p = (AllocBlock*)Deref(header_->alloc_size);
            header_->alloc_size += header_->block_size;
            ++header_->alloc_node;
        }
        else
        {
            snprintf(error_msg_, sizeof(error_msg_), "Alloc错误,空间已满");
        }
    }
    else if (!IsValidBlock(p))
    {
        snprintf(error_msg_, sizeof(error_msg_), "Alloc错误,空闲块非法%p,"
            "header=%p,max_size=%zu,alloc_size=%zu,block_size=%zu", p, header_, header_->max_size,
            header_->alloc_size, header_->block_size);
        p = NULL;
    }

    if (!p) return NULL;

    if (zero)
    {
        memset(p, 0, header_->block_size);
    }

    p->next = 0;

    return p->node;
}

int FixedMemPool::Free(void* node)
{
    AllocBlock* block = GetBlock(node);

    if (IsUsedBlock(block))
    {
        LinkFreeBlock(block);
    }
    else
    {
        snprintf(error_msg_, sizeof(error_msg_), "Free释放指针错误,指针%p为非法指针,"
            "header=%p,max_size=%zu,alloc_size=%zu,block_size=%zu", node, header_,
            header_->max_size, header_->alloc_size, header_->block_size);
        return -1;
    }

    return 0;
}

int FixedMemPool::Clear()
{
    int max_node = header_->max_node;
    int node_size = header_->node_size;
    return Init(header_, max_node, node_size);
}

