/******************************************************************************************
Description         : 拷贝自sail的zenlib，原名是zen_mmaplruhash，由于这个实现实际上只是基于已分配内存的，
                                                          而不管是不是共享内存，所以改了一下名字。
                                                          本文件实现了基于lru的hashtbale。
Modification History:
******************************************************************************************/
#ifndef MEM_LRUHASH_H_
#define MEM_LRUHASH_H_

#include "mem_predef.h"

//前导声明，供迭代器用
template <class _value_type,
    class _key_type,
    class _hash_fun,
    class _extract_key,
    class _equal_key,
    class _washout_fun >
class mem_lru_hashtable;

//LRU HASH 迭代器
template < class _value_type,
    class _key_type,
    class _hashfun,
    class _extract_key,
    class _equal_key ,
    class _washout_fun >
class _lru_hashtable_iterator
{
protected:
    //HASH TABLE的定义
    typedef mem_lru_hashtable<_value_type,
        _key_type,
        _hashfun,
        _extract_key,
        _equal_key,
        _washout_fun  > _lru_hashtable;

    //定义迭代器
    typedef _lru_hashtable_iterator<_value_type,
        _key_type,
        _hashfun,
        _extract_key,
        _equal_key,
        _washout_fun > iterator;

protected:
    //序列号
    size_t                 serial_;
    //
    _lru_hashtable        *lruht_instance_;

public:
    _lru_hashtable_iterator():
        serial_(0),
        lruht_instance_(NULL)
    {
    }

    _lru_hashtable_iterator(size_t serial,_lru_hashtable *lru_ht_inst):
        serial_(serial),
        lruht_instance_(lru_ht_inst)
    {
    }

    ~_lru_hashtable_iterator()
    {
    }

    _value_type& operator*() const
    {
        return lruht_instance_->value_base_ [serial_];
    }

    _value_type* operator->() const
    {
        return lruht_instance_->value_base_ + serial_;
    }

    //本来只提供前向迭代器，曾经以为使用可以使用LIST保证迭代的高效，发现不行，
    //可能要提供另外的函数
    //前向迭代器
    iterator &operator++()
    {
        size_t oldseq = serial_;
        serial_ = *(lruht_instance_->hash_index_base_ +serial_);

        //如果这个节点是末位的节点
        if (serial_ == _memory_base::_INVALID_POINT)
        {
            //顺着Index查询.
            size_t bucket = lruht_instance_->bkt_num_value(*(lruht_instance_->value_base_ +oldseq));
            //
            while(serial_ == _memory_base::_INVALID_POINT && ++bucket < lruht_instance_->capacity() )
            {
                serial_ = *(lruht_instance_->hash_factor_base_ + bucket);
            }
        }
        return *this;
    }

    //这个函数类似++，但是只在Hash数据的链表上游荡，所以性能更好
    //如果下一个数据是一样的KEY，那么就成为下个数据的迭代器。
    //否则成为end
    //其实也就比你自己做快一点点。
    iterator &goto_next_equal()
    {
        size_t oldseq =serial_;
        serial_ = *(lruht_instance_->hash_index_base_ +serial_);

        //如果这个节点不是末位的节点
        if (serial_ != _memory_base::_INVALID_POINT)
        {
            _extract_key get_key;
            _equal_key   equal_key;

            if( false == equal_key(get_key(*(lruht_instance_->value_base_ +oldseq)),
                get_key(*(lruht_instance_->value_base_ +serial_))) )
            {
                serial_ = _memory_base::_INVALID_POINT;
            }
        }
        else
        {
        }
        return *this;
    }

    //前向迭代器
    iterator operator++(int)
    {
        iterator tmp = *this;
        ++*this;
        return tmp;
    }
    //
    bool operator==(const iterator &it) const
    {
        if (lruht_instance_ == it.lruht_instance_ &&
            serial_ == it.serial_ )
        {
            return true;
        }
        return false;
    }
    //
    bool operator!=(const iterator &it) const
    {
        return !(*this == it);
    }

    //保留序号就可以再根据模版实例化对象找到相应数据,不用使用指针
    size_t getserial() const
    {
        return serial_;
    }
};


//头部，LRU_HASH的头部结构，放在LRUHASH内存的前面
class _lru_hashhead
{
protected:
    _lru_hashhead():
        size_of_mem_(0),
        num_of_node_(0),
        sz_freenode_(0),
        sz_usenode_(0),
        sz_useindex_(0)
    {
    }
    ~_lru_hashhead()
    {
    }

public:
    //内存区的长度
    size_t           size_of_mem_;

    //NODE,INDEX结点个数,INDEX的个数和NODE的节点个数为1:1,
    size_t           num_of_node_;

    //FREE的NODE个数
    size_t           sz_freenode_;
    //USE的NODE个数
    size_t           sz_usenode_;

    //使用的INDEX个数,可以了解实际开链的负载比率
    size_t           sz_useindex_;
};


//淘汰函数
template < class _value_type>
class _default_washout_fun
{
public:
    void operator()(_value_type & /*da*/)
    {
        return;
    }
};

//lru hash表
template <class _value_type,
    class _key_type,
    class _hash_fun = mem_hash<_key_type>,
    class _extract_key=mem_identity<_value_type>,
    class _equal_key = std::equal_to<_key_type> ,
    class _washout_fun = _default_washout_fun<_value_type> >
class mem_lru_hashtable :public  _memory_base
{
public:
    //定义迭代器
    typedef _lru_hashtable_iterator<_value_type,
        _key_type,
        _hash_fun,
        _extract_key,
        _equal_key,
        _washout_fun > iterator;

    friend class _lru_hashtable_iterator<_value_type,
        _key_type,
        _hash_fun,
        _extract_key,
        _equal_key,
        _washout_fun >;

protected:
    //
    static const size_t  LIST_ADD_NODE_NUMBER = 2;

protected:
    //
    _lru_hashhead         *lru_hash_head_;

    //Hash因子的BASE
    size_t                *hash_factor_base_;
    //hash链的索引
    size_t                *hash_index_base_;

    //LIST的索引
    _mem_list_index      *lst_index_base_;
    //FREE节点链表的开始
    _mem_list_index      *lst_free_node_;
    //USE节点链表的开始
    _mem_list_index      *lst_use_node_;

    //优先级的数据指针
    unsigned long         *priority_base_;
    //数据区指针
    _value_type           *value_base_;


protected:

    mem_lru_hashtable():
        lru_hash_head_(NULL),
        hash_factor_base_(NULL),
        hash_index_base_(NULL),
        lst_index_base_(NULL),
        lst_free_node_(NULL),
        lst_use_node_(NULL),
        priority_base_(NULL),
        value_base_(NULL)
    {
    }

public:

    ~mem_lru_hashtable()
    {
    }

protected:

    //返回大于N的一个质数,(最后一个例外),使用质数作为取模的
    static size_t get_next_prime(const size_t n)
    {
        const size_t num_primes_list =50;
        //考虑到实际情况,增加了很多100万以下的质数,
        const size_t primes_list[num_primes_list] = __MEM_PRIME_LIST_BODY;

        size_t num_primes = num_primes_list;
        size_t i=0;
        //找呀找
        for(; i < num_primes ; ++i)
        {
            if(primes_list[i] >= n)
            {
                break;
            }
        }
        //如果大于最后一个调整一下,
        return (i==num_primes)?primes_list[i-1]:primes_list[i];
    }


public:

    //内存区的构成为 define区,index区,data区,返回所需要的长度,
    //注意返回的是实际INDEX长度,会取一个质数
    static size_t getallocsize(size_t &numnode)
    {
        numnode = get_next_prime(numnode);
        size_t sz_alloc =  0;
        //
        sz_alloc += sizeof(_lru_hashhead);
        sz_alloc += sizeof(size_t) * numnode;
        sz_alloc += sizeof(size_t) * numnode;
        sz_alloc += sizeof(size_t) * numnode;
        //
        sz_alloc += sizeof(_mem_list_index)*(numnode + LIST_ADD_NODE_NUMBER);
        sz_alloc += sizeof(unsigned long) *(numnode);
        sz_alloc += sizeof(_value_type)* (numnode);
        return sz_alloc;
    }

    //
    static mem_lru_hashtable< _value_type,
        _key_type,
        _hash_fun,
        _extract_key,
        _equal_key,
        _washout_fun >*
        initialize(size_t &numnode,char *pmem,bool brestore = false)
    {
        assert(pmem!=NULL && numnode >0 );
        //调整
        numnode = get_next_prime(numnode);

        _lru_hashhead *hashhead =  reinterpret_cast< _lru_hashhead* >(pmem);

        //如果是恢复,数据都在内存中,
        if(brestore == true)
        {
            //检查一下恢复的内存是否正确,
            if(getallocsize(numnode) != hashhead->size_of_mem_ ||
                numnode != hashhead->num_of_node_ )
            {
                return NULL;
            }
        }

        //初始化尺寸
        hashhead->size_of_mem_ = getallocsize(numnode) ;
        hashhead->num_of_node_ = numnode;

        mem_lru_hashtable< _value_type,_key_type ,_hash_fun,_extract_key,_equal_key,_washout_fun >* instance
            = new mem_lru_hashtable< _value_type,_key_type ,_hash_fun, _extract_key,_equal_key,_washout_fun>();

        instance->mem_base_ = pmem;
        char *tmp_base = instance->mem_base_;
        instance->lru_hash_head_ = reinterpret_cast<_lru_hashhead*>(tmp_base);
        tmp_base = tmp_base + sizeof(_lru_hashhead);
        instance->hash_factor_base_ = reinterpret_cast<size_t*>(tmp_base);
        tmp_base = tmp_base + sizeof(size_t) * numnode;
        instance->hash_index_base_ = reinterpret_cast<size_t*>(tmp_base);

        tmp_base = tmp_base + sizeof(size_t) * numnode;
        instance->lst_index_base_ = reinterpret_cast<_mem_list_index*>(tmp_base);
        tmp_base = tmp_base + sizeof(_mem_list_index)*(numnode + LIST_ADD_NODE_NUMBER);
        instance->lst_use_node_ = instance->lst_index_base_ + numnode;
        instance->lst_free_node_ = instance->lst_index_base_ + numnode + 1;

        instance->priority_base_ = reinterpret_cast<unsigned long*>(tmp_base);
        tmp_base = tmp_base + sizeof(unsigned long)*(numnode );
        instance->value_base_ = reinterpret_cast<_value_type*>(tmp_base);

        if (brestore == false)
        {
            //清理初始化所有的内存,所有的节点为FREE
            instance->clear();
        }

        //打完收工
        return instance;
    }

    //清理初始化所有的内存,所有的节点为FREE
    void clear()
    {
        //处理关键Node,以及相关长度,开始所有的数据是free.
        lru_hash_head_->sz_freenode_ = lru_hash_head_->num_of_node_;
        lru_hash_head_->sz_usenode_ = 0;
        lru_hash_head_->sz_useindex_ = 0;

        //将两个队列都清理为NULL,让指针都指向自己,这儿有一点小技巧,
        //你可以将其视为将双向链表的头指针,(其实也是尾指针).
        lst_use_node_->idx_next_ = lru_hash_head_->num_of_node_ ;
        lst_use_node_->idx_prev_ = lru_hash_head_->num_of_node_ ;

        lst_free_node_->idx_next_ = lru_hash_head_->num_of_node_ +1;
        lst_free_node_->idx_prev_ = lru_hash_head_->num_of_node_ +1;



        _mem_list_index *pindex = lst_index_base_;

        //初始化free数据区
        for (size_t i=0; i<lru_hash_head_->num_of_node_ ;++i )
        {

            //
            hash_factor_base_[i] = _INVALID_POINT;
            hash_index_base_[i] =_INVALID_POINT;
            priority_base_[i]=0;

            pindex->idx_next_ = (i+1) ;
            pindex->idx_prev_ = (i-1) ;
            //将所有的数据用FREENODE串起来
            if(0 == i)
            {
                pindex->idx_prev_ = lst_free_node_->idx_next_;
                lst_free_node_->idx_next_ = 0;
            }
            if(i == lru_hash_head_->num_of_node_ - 1)
            {
                pindex->idx_next_ = lst_free_node_->idx_prev_;
                lst_free_node_->idx_prev_ = lru_hash_head_->num_of_node_ - 1;
            }
            pindex++;
        }
    }

protected:

    //分配一个NODE,将其从FREELIST中取出
    size_t create_node()
    {
        //如果没有空间可以分配
        if (lru_hash_head_->sz_freenode_ == 0)
        {
            return _INVALID_POINT;
        }
        //从链上取1个下来
        size_t node = lst_free_node_->idx_next_;

        lst_free_node_->idx_next_ = (lst_index_base_+ node)->idx_next_;
        //lst_free_node_->idx_next_已经向后调整一个位置了
        (lst_index_base_ + lst_free_node_->idx_next_)->idx_prev_ = (lst_index_base_+ node)->idx_prev_;

        //注意num_of_node_的位置是usenode
        lst_index_base_[node].idx_next_ = lst_use_node_->idx_next_;
        lst_index_base_[node].idx_prev_ = lst_index_base_[lst_use_node_->idx_next_].idx_prev_;
        lst_index_base_[lst_use_node_->idx_next_].idx_prev_ = node;
        lst_use_node_->idx_next_ = node;

        lru_hash_head_->sz_usenode_  ++;
        lru_hash_head_->sz_freenode_ --;

        //检查你干错事情么没有
        assert(lru_hash_head_->sz_usenode_ + lru_hash_head_->sz_freenode_ == lru_hash_head_->num_of_node_);
        return node;
    }

    //释放一个NODE,将其归还给FREELIST,单向链表就是简单
    void destroy_node(size_t pos)
    {
        size_t freenext = lst_free_node_->idx_next_;

        size_t pos_next = lst_index_base_[pos].idx_next_ ;
        size_t pos_prev = lst_index_base_[pos].idx_prev_ ;
        lst_index_base_[pos_next].idx_prev_ = pos_prev;
        lst_index_base_[pos_prev].idx_next_ = pos_next;

        (lst_index_base_ + pos)->idx_next_ = freenext;
        (lst_index_base_ + pos)->idx_prev_ = (lst_index_base_ + freenext)->idx_prev_;

        lst_free_node_->idx_next_ = pos;

        (lst_index_base_ + freenext)->idx_prev_ = pos;
        lru_hash_head_->sz_usenode_  --;
        lru_hash_head_->sz_freenode_ ++;

        hash_index_base_[pos] = _INVALID_POINT;

        assert(lru_hash_head_->sz_usenode_ + lru_hash_head_->sz_freenode_ == lru_hash_head_->num_of_node_);
    }


    //从value中取值
    size_t bkt_num_value(const _value_type & obj) const
    {
        _extract_key get_key;
        return static_cast<size_t>(bkt_num_key(get_key(obj)));
    }

    //为什么不能重载上面的函数,自己考虑一下,
    //重载的话，如果_value_type和_key_type一样，就等着哭吧 ---inmore
    size_t bkt_num_key(const _key_type & key) const
    {
        _hash_fun hash_fun;
        return static_cast<size_t>(hash_fun(key) % lru_hash_head_->num_of_node_);
    }

public:


    //得到开始的迭代器的位置
    iterator begin()
    {
        for (size_t i = 0; i < lru_hash_head_->num_of_node_; ++i)
        {
            if (*(hash_factor_base_ +i) != _INVALID_POINT)
            {
                return iterator(*(hash_factor_base_ +i), this);
            }
        }
        return end();
    }

    //得到结束位置
    iterator end()
    {
        return iterator(_INVALID_POINT,this);
    }
    //当前使用的节点数量
    size_t size() const
    {
        return lru_hash_head_->sz_usenode_;
    }
    //得到容量
    size_t capacity() const
    {
        return lru_hash_head_->num_of_node_;
    }
    //
    bool empty() const
    {
        return (lru_hash_head_->sz_freenode_ == lru_hash_head_->num_of_node_);
    }
    //是否空间已经满了
    bool full() const
    {
        return (lru_hash_head_->sz_freenode_ == 0);
    }

    //插入节点
    std::pair<iterator, bool> insert(const _value_type& val,
        unsigned long priority = time(NULL))
    {
        size_t idx = bkt_num_value(val);
        size_t first = hash_factor_base_[idx];

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        while (first != _INVALID_POINT )
        {

            //如果找到相同的Key函数
            if(equal_key((get_key(value_base_[first])),(get_key(val))) ==true )
            {
                return std::pair<iterator, bool>(iterator(first, this), false);
            }
            first = hash_index_base_[ first ];
        }
        //没有找到,插入新数据
        size_t newnode = create_node();

        //空间不足,
        if (newnode == _INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        //放入链表中
        hash_index_base_[newnode]  = hash_factor_base_[idx];
        hash_factor_base_[idx] = newnode;
        value_base_[newnode] = val;
        priority_base_[newnode] = priority;

        return std::pair<iterator, bool>(iterator(newnode,this), true);
    }

    //插入节点,允许相等
    std::pair<iterator, bool> insert_equal(const _value_type& val,
        unsigned long priority = time(NULL))
    {
        size_t idx = bkt_num_value(val);
        size_t first = hash_factor_base_[idx];

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key函数
            if(equal_key((get_key(value_base_[first])),(get_key(val))) ==true )
            {
                break;
            }
            first = hash_index_base_[ first ];
        }
        //没有找到,插入新数据
        size_t newnode = create_node();

        //空间不足,
        if (newnode == _INVALID_POINT)
        {
            return std::pair<iterator, bool>(iterator(_INVALID_POINT, this), false);
        }

        //没有找到相同KEY的数据
        if (first == _INVALID_POINT)
        {
            //放入链表的首部就可以了
            hash_index_base_[newnode]  = hash_factor_base_[idx];
            hash_factor_base_[idx] = newnode;
        }
        //如果找到了相同的KEY节点
        else
        {
            //放到这个节点的后面
            hash_index_base_ [newnode] = hash_index_base_[first];
            hash_index_base_[first] = newnode;
        }
        value_base_[newnode] = val;
        priority_base_[newnode] = priority;

        return std::pair<iterator, bool>(iterator(newnode, this), true);
    }

    //查询相应的Key是否有,返回迭代器
    iterator find(const _key_type& key)
    {
        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        while (first != _INVALID_POINT && !equal_key(get_key(value_base_[first]), key))
        {
            first = hash_index_base_[ first ];
        }
        return iterator(first,this);
    }
    //
    iterator find_value(const _value_type& val)
    {
        _extract_key get_key;
        return find(get_key(val));
    }

    //得到某个KEY的元素个数，有点相当于查询操作
    size_t count(const _key_type& key)
    {
        size_t equal_count =0;
        size_t idx = bkt_num_key(key);
        //从索引中找到第一个
        size_t first = hash_factor_base_[ idx];

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if(equal_key(get_key(value_base_[first]), key) ==true )
            {
                ++equal_count;
            }
            first = hash_index_base_[ first ];
        }
        return equal_count;
    }

    //得到某个VALUE的元素个数，有点相当于查询操作
    size_t count_value(const _value_type& val)
    {
        _extract_key get_key;
        return count( get_key(val));
    }

    bool erase(const _key_type& key)
    {
        size_t idx = bkt_num_key(key);
        //从索引中找到第一个
        size_t first = hash_factor_base_[ idx];
        size_t prev = first;

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if(equal_key(get_key(value_base_[first]), key) ==true )
            {
                if(first == hash_factor_base_[ idx])
                {
                    hash_factor_base_[ idx] = hash_index_base_[first];
                }
                else
                {
                    hash_index_base_[prev] = hash_index_base_[first];
                }
                //回收空间
                destroy_node(first);

                return true;
            }
            prev = first;
            first = hash_index_base_[ first ];
        }
        return false;
    }



    //使用迭代器删除,尽量高效所以不用简化写法
    bool erase(const iterator &it)
    {
        _extract_key get_key;
        size_t idx =bkt_num_key(get_key(*it));
        size_t first = hash_factor_base_[ idx];
        size_t prev = first;
        size_t itseq = it.getserial();
        //
        while (first != _INVALID_POINT )
        {
            if (first == itseq )
            {
                if(first == hash_factor_base_[ idx])
                {
                    hash_factor_base_[ idx] = hash_index_base_[first];
                }
                else
                {
                    hash_index_base_[prev] = hash_index_base_[first];
                }
                //回收空间
                destroy_node(first);

                return true;
            }
            prev = first;
            first = hash_index_base_[ first ];
        }
        return false;
    }

    //删除某个值
    bool erase_value(const _value_type& val )
    {
        _extract_key get_key;
        return erase( get_key(val));
    }


    //删除所有相等的KEY的数据,和insert_equal配对使用，返回删除了几个数据
    size_t erase_equal(const _key_type& key)
    {
        size_t erase_count =0;
        size_t idx = bkt_num_key(key);
        //从索引中找到第一个
        size_t first = hash_factor_base_[ idx];
        size_t prev = first;

        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //循环查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if(equal_key(get_key(value_base_[first]), key) ==true )
            {
                if(first == hash_factor_base_[ idx])
                {
                    hash_factor_base_[ idx] = hash_index_base_[first];
                }
                else
                {
                    hash_index_base_[prev] = hash_index_base_[first];
                }
                //删除的情况下prev不用调整，first向后移动
                size_t del_pos = first;
                first = hash_index_base_[ first ];
                //回收空间
                destroy_node(del_pos);
                ++erase_count;
            }
            else
            {
                //如果已经删除过，退出循环，因为所有的KEY相同的东东挂在一起，删除也是一起删除了.
                if (erase_count > 0)
                {
                    break;
                }
                prev = first;
                first = hash_index_base_[ first ];
            }
        }
        return erase_count;
    }
    //删除所有相等的每个数据(还是要KEY相等),和insert_equal配对使用，返回删除了几个数据
    bool erase_equal_value(const _value_type& val )
    {
        _extract_key get_key;
        return erase_equal( get_key(val));
    }


    //激活,将激活的数据挂到LIST的最开始,淘汰使用expire,disuse
    bool active(const _key_type &key, unsigned long priority = time(NULL))
    {
        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if(equal_key(get_key(value_base_[first]), key) ==true )
            {
                priority_base_[first] = priority;

                size_t first_prv = lst_index_base_[first].idx_prev_;
                size_t first_nxt = lst_index_base_[first].idx_next_;
                //从原来的地方取下来
                lst_index_base_[first_prv].idx_next_ = lst_index_base_[first].idx_next_;
                lst_index_base_[first_nxt].idx_prev_ = lst_index_base_[first].idx_prev_;

                //放如头部
                lst_index_base_[first].idx_next_ = lst_use_node_->idx_next_;
                lst_index_base_[lst_use_node_->idx_next_].idx_prev_ = first;
                lst_index_base_[first].idx_prev_ = lru_hash_head_->num_of_node_;
                lst_use_node_->idx_next_ = first;

                return true;

            }
            first = hash_index_base_[ first ];
        }
        return false;
    }



    //通过VALUE激活，同时讲值替换成最新的数据VALUE
    bool active_value(const _value_type &val, unsigned long priority = time(NULL))
    {
        _extract_key get_key;
        _equal_key   equal_key;

        _key_type key = get_key(val);
        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[idx];
        //使用量函数对象,一个类单独定义一个是否更好?



        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if(equal_key(get_key(value_base_[first]), key) ==true )
            {
                priority_base_[first] = priority;
                value_base_[first] = val;
                size_t first_prv = lst_index_base_[first].idx_prev_;
                size_t first_nxt = lst_index_base_[first].idx_next_;
                //从原来的地方取下来
                lst_index_base_[first_prv].idx_next_ = lst_index_base_[first].idx_next_;
                lst_index_base_[first_nxt].idx_prev_ = lst_index_base_[first].idx_prev_;

                //放如头部
                lst_index_base_[first].idx_next_ = lst_use_node_->idx_next_;
                lst_index_base_[lst_use_node_->idx_next_].idx_prev_ = first;
                lst_index_base_[first].idx_prev_ = lru_hash_head_->num_of_node_;
                lst_use_node_->idx_next_ = first;

                return true;

            }
            first = hash_index_base_[ first ];
        }
        return false;
    }

    //激活所有相同的KEY,将激活的数据挂到LIST的最开始,淘汰使用expire
    size_t active_equal(const _key_type &key, unsigned long priority = time(NULL))
    {
        size_t active_count =0;

        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if(equal_key(get_key(value_base_[first]), key) ==true )
            {
                priority_base_[first] = priority;

                size_t first_prv = lst_index_base_[first].idx_prev_;
                size_t first_nxt = lst_index_base_[first].idx_next_;
                //从原来的地方取下来
                lst_index_base_[first_prv].idx_next_ = lst_index_base_[first].idx_next_;
                lst_index_base_[first_nxt].idx_prev_ = lst_index_base_[first].idx_prev_;

                //放如头部
                lst_index_base_[first].idx_next_ = lst_use_node_->idx_next_;
                lst_index_base_[first].idx_prev_ = lst_index_base_[lst_use_node_->idx_next_].idx_prev_;
                lst_index_base_[lst_use_node_->idx_next_].idx_prev_ = first;
                lst_use_node_->idx_next_ = first;
            }
            else
            {
                //如果已经有数据，退出循环，因为所有的KEY相同的东东挂在一起，删除也是一起删除了.
                if (active_count > 0)
                {
                    break;
                }
            }
            first = hash_index_base_[ first ];

        }
        return active_count;
    }

    //激活所有相同的KEY，这个函数有点别扭。我不喜欢，因为VALUE没有（没法）跟新
    //size_t active_value_equal(const _value_type &val, unsigned long priority = time(NULL))
    //{
    //    return active_equal(_extract_key(val),priority);
    //}

    //淘汰过期的数据,假设LIST中间的数据是按照过期实际排序的
    //小于等于这个优先级的数据将被淘汰
    size_t expire(unsigned long expire_time)
    {
        //从尾部开始检查，
        size_t list_idx = lst_use_node_->idx_prev_;
        size_t expire_num = 0;

        while(list_idx != lru_hash_head_->num_of_node_)
        {
            //小于等于
            if(priority_base_[list_idx] <= expire_time)
            {
                size_t del_iter = list_idx;
                _washout_fun wash_fun;
                wash_fun(value_base_[del_iter]);
                ++expire_num;
                //
                iterator iter_tmp(del_iter,this);
                erase(iter_tmp);
            }
            else
            {
                break;
            }
            //如果删除了，还是检查第一个
            list_idx = lst_use_node_->idx_prev_;
        }
        return expire_num;
    }

    //希望淘汰掉disuse_num个数据，
    //如果disuse_eaqul == ture，则删除和最后删除的那个优先级相等的所有元素
    //disuse_eaqul可以保证数据的整体淘汰，避免一个KEY的部分数据在内存，一部分不在
    size_t disuse(size_t disuse_num,bool disuse_eaqul)
    {
        //从尾部开始检查，
        size_t list_idx = lst_use_node_->idx_prev_;
        size_t fact_del_num = 0;
        unsigned long disuse_priority = 0;
        for(size_t i=0;i<disuse_num && list_idx != lru_hash_head_->num_of_node_;++i)
        {
            size_t del_iter = list_idx;
            _washout_fun wash_fun;
            wash_fun(value_base_[del_iter]);
            ++fact_del_num;
            //
            iterator iter_tmp(del_iter,this);
            erase(iter_tmp);
            disuse_priority = priority_base_[list_idx];
            //
            //如果删除了，还是检查第一个
            list_idx = lst_use_node_->idx_prev_;
        }

        if( true == disuse_eaqul && disuse_num > 0)
        {
            fact_del_num+=expire(disuse_priority);
        }
        return fact_del_num;
    }

    //标注，重新给一个数据打一个优先级标签，淘汰使用函数washout
    bool mark(const _key_type &key, unsigned long priority)
    {
        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if(equal_key(get_key(value_base_[first]), key) ==true )
            {
                priority_base_[first] = priority;

                return true;
            }
            first = hash_index_base_[ first ];
        }
        return false;
    }

    //根据value将优先级跟新，重新给一个数据打一个优先级标签，同时将值替换，
    bool mark_value(const _value_type &val, unsigned long priority )
    {
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        _key_type key= get_key(val);


        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];

        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if(equal_key(get_key(value_base_[first]), key) ==true )
            {
                priority_base_[first] = priority;
                value_base_[first] = val;
                return true;
            }
            first = hash_index_base_[ first ];
        }
        return false;
    }

    //标注所有相等的数据，重新给一个数据打一个优先级标签，淘汰使用函数washout
    bool mark_equal(const _key_type &key, unsigned long priority)
    {
        size_t mark_count =0;

        size_t idx = bkt_num_key(key);
        size_t first = hash_factor_base_[ idx];
        //使用量函数对象,一个类单独定义一个是否更好?
        _extract_key get_key;
        _equal_key   equal_key;

        //在列表中间查询
        while (first != _INVALID_POINT )
        {
            //如果找到相同的Key
            if(equal_key(get_key(value_base_[first]), key) ==true )
            {
                priority_base_[first] = priority;
            }
            else
            {
                //如果已经有数据，退出循环，因为所有的KEY相同的东东挂在一起，删除也是一起删除了.
                if (mark_count > 0)
                {
                    break;
                }
            }
            first = hash_index_base_[ first ];

        }
        return mark_count;
    }

    //淘汰优先级过低的数据,LIST中间的数据是是乱序的也可以.只淘汰num_wash数量的数据
    //但限制淘汰是从头部开始，感觉不是太好。
    void washout(unsigned long wash_priority,size_t num_wash)
    {
        size_t list_idx = lst_use_node_->idx_next_;
        size_t num_del = 0;
        //不为NULL，而且删除的个数没有达到，从头部开始是否好呢?我不确定，打算在提供一个函数
        while ( list_idx != lru_hash_head_->num_of_node_ && num_del < num_wash)
        {
            //如果优先级小于淘汰系数
            if(priority_base_[list_idx] < wash_priority)
            {
                ++num_del;
                size_t del_iter = list_idx;
                list_idx = lst_index_base_[list_idx].idx_next_;
                //
                _washout_fun wash_fun;
                wash_fun(value_base_[del_iter]);
                //
                iterator iter_tmp(del_iter,this);
                erase(iter_tmp);
            }
            else
            {
                continue;
            }
        }
    }
};

#endif /* MEM_LRUHASH_H_ */
