#ifndef HASH_MEM_POOL_H_
#define HASH_MEM_POOL_H_

#include "mem_pool/fixed_mem_pool.h"
#include "mem_pool/must_be_pod.h"
#include <sys/types.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>


template <typename KeyType, typename ValueType>
class HashMemPool
{
public:
    static const int OK = 0;
    static const int FAILED = -1;

    typedef struct HashNode
    {
        KeyType key;
        ValueType value;
        size_t next;
    } HashNode;

    typedef struct HashHeader
    {
        uint32_t bucket_num;
        uint32_t max_node;
        uint32_t node_num;
        size_t node_size;
    } HashHeader;
public:
    HashMemPool(): header_(NULL), buckets_(NULL) {memset(error_msg_, 0, sizeof(error_msg_));}
    ~HashMemPool() {}

    inline const char* GetErrorMsg() const
	{
		return error_msg_;
	}

    inline uint32_t GetAllocNodeNum() const
    {
        return header_->node_num;
    }

    inline size_t Ref(void* p) const
    {
        return mem_pool_.Ref(p);
    }

    inline void* Deref(size_t pos) const
    {
        return mem_pool_.Deref(pos);
    }

    int Init(void* mem, uint32_t bucket_num, uint32_t max_node, bool check_header = false)
    {
        must_be_pod<KeyType>();
        must_be_pod<ValueType>();

        char* p = (char*)mem;
        header_ = (HashHeader*)p;
        if (check_header)
        {
            if (header_->bucket_num != bucket_num || header_->max_node != max_node ||
                header_->node_size != sizeof(HashNode))
            {
            	snprintf(error_msg_, sizeof(error_msg_), "Init失败,check_header failed,"
                    "bucket_num %u vs %u, node_num %u vs %u, node_size %zd vs %zd",
                    header_->bucket_num, bucket_num,
                    header_->max_node,max_node,
                    header_->node_size,sizeof(HashNode));
                return FAILED;
            }
        }
        else
        {
            header_->bucket_num = bucket_num;
            header_->max_node = max_node;
            header_->node_num = 0;
            header_->node_size = sizeof(HashNode);
            snprintf(error_msg_, sizeof(error_msg_), "bucket_num:%u, max_node:%u,node_size：%zd",
                      bucket_num,max_node,header_->node_size);
        }
        p += sizeof(HashHeader);

        buckets_ = (size_t*)p;
        if (!check_header)
        {
            memset(buckets_, 0, sizeof(buckets_[0]) * header_->bucket_num);
        }
        p += sizeof(buckets_[0]) * header_->bucket_num;

        int ret = mem_pool_.Init(p, header_->max_node, sizeof(HashNode), check_header);
        if (ret != 0)
        {
        	snprintf(error_msg_, sizeof(error_msg_), "Init失败,mem_pool_.Init失败,ret=%d,error=%s",
                ret, mem_pool_.GetErrorMsg());
            return FAILED;
        }

        return OK;
    }

    ValueType* Alloc(const KeyType& key)
    {
        HashNode* node = GetNode(key);
        if (node)
        {
        	snprintf(error_msg_, sizeof(error_msg_), "Alloc失败,error=key已经存在");
            return NULL;
        }

        node = (HashNode*)mem_pool_.Alloc();
        if (!node)
        {
        	snprintf(error_msg_, sizeof(error_msg_), "Alloc失败,Alloc() return NULL,error=%s",
                mem_pool_.GetErrorMsg());
            return NULL;
        }

        memcpy(&node->key, &key, sizeof(KeyType));
        int index = key % header_->bucket_num;
        if (buckets_[index] == 0)
        {
            node->next = 0;
            buckets_[index] = mem_pool_.Ref(node);
        }
        else
        {
            node->next = buckets_[index];
            buckets_[index] = mem_pool_.Ref(node);
        }

        header_->node_num++;

        return &node->value;
    }

    ValueType* Insert(const KeyType& key, const ValueType* value)
    {
        ValueType* alloc = Alloc(key);
        if (!alloc)
        {
            return NULL;
        }

        memcpy(alloc, value, sizeof(ValueType));

        return alloc;
    }

    ValueType* Get(const KeyType& key)
    {
        HashNode* node = GetNode(key);
        if (node)
        {
            return &node->value;
        }

        return NULL;
    }

    int Free(const KeyType& key)
    {
        int index = key % header_->bucket_num;
        HashNode* prev_node = NULL;
        HashNode* node = NULL;

        size_t p = buckets_[index];
        while (p)
        {
            node = (HashNode*)mem_pool_.Deref(p);
            if (node->key == key)
            {
                if (prev_node)
                {
                    prev_node->next = node->next;
                }
                else
                {
                    buckets_[index] = node->next;
                }

                header_->node_num--;

                if (mem_pool_.Free(node) != 0)
                {
                	snprintf(error_msg_, sizeof(error_msg_), "Free失败,error=%s", mem_pool_.GetErrorMsg());
                    return FAILED;
                }
                return OK;
            }
            prev_node = node;
            p = node->next;
        }

        snprintf(error_msg_, sizeof(error_msg_), "Free失败,key not found");
        return FAILED;
    }

    void GetValueList(const KeyType& key, std::vector<ValueType *>& val_list) const
    {
        val_list.clear();
        int index = key % header_->bucket_num;
        HashNode* node = NULL;
        size_t p = buckets_[index];
        while (p)
        {
            node = (HashNode*)mem_pool_.Deref(p);
            val_list.push_back(&node->value);
            p = node->next;
        }
    }

    static size_t GetMaxSize(uint32_t bucket_num, uint32_t max_node)
    {
        return sizeof(HashHeader) + sizeof(buckets_[0]) * bucket_num +
            FixedMemPool::GetMaxSize(max_node, sizeof(HashNode));
    }

    size_t GetMaxSize()
    {
        return GetMaxSize(header_->bucket_num, header_->max_node);
    }

    const HashHeader* GetHeader() const
    {
        return header_;
    }

    // 遍历接口，遍历首次调用传NULL。以后每次传上次得到的HashNode
    const HashNode* GetNextNode(const HashNode* node = NULL)
    {
        const FixedMemPool::AllocBlock* block = NULL;
        if (node)
        {
            block = mem_pool_.GetBlock(node);
        }

        block = mem_pool_.GetNextUsedBlock(block);

        if (!block) return NULL;
        else return (const HashNode *)block->node;
    }

    int GetMemUtilization() const
    {
        return mem_pool_.GetMemUtilization();
    }

    int Init(key_t shm_key, uint32_t bucket_num, uint32_t max_node, bool &is_exist)
    {
        size_t shm_size = GetMaxSize(bucket_num, max_node);
        is_exist = true;
        int shm_id = shmget(shm_key, shm_size, 0644);
        if (shm_id == -1)
        {
            is_exist = false;
            shm_id = shmget(shm_key, shm_size, 0644 | IPC_CREAT | IPC_EXCL);
            if (shm_id == -1)
            {
                //既然获取不了该共享内存，但是又不能创建共享内存，那么只能返回错误了
                printf("获取共享内存失败:%s\n", strerror(errno));
                return FAILED;
            }
        }

        void *mem_begin = shmat(shm_id, NULL, 0);
        if (mem_begin == (void *) -1)
        {
            printf("连接共享内存失败:%s\n", strerror(errno));
            return FAILED;
        }

        return Init(mem_begin, bucket_num, max_node, is_exist);
    }

protected:
    HashNode* GetNode(const KeyType& key)
    {
        int index = key % header_->bucket_num;
        HashNode* node = NULL;

        size_t p = buckets_[index];
        while (p)
        {
            node = (HashNode*)mem_pool_.Deref(p);
            if (node->key == key)
            {
                return node;
            }
            p = node->next;
        }

        return NULL;
    }

protected:
    HashHeader* header_;
    size_t* buckets_;
    FixedMemPool mem_pool_;

    char error_msg_[256];
};


#endif /* HASH_MEM_POOL_H_ */
