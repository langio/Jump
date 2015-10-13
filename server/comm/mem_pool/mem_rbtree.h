#ifndef MEM_RBTREE_V2_H_
#define MEM_RBTREE_V2_H_

#include <functional>
#include <algorithm>
#include <utility>
#include <string.h>
#include <vector>
#include <cassert>

//节点颜色
enum RB_TREE_COLOR
{
    //红节点
    RB_TREE_RED   = 0,
    //黑节点
    RB_TREE_BLACK = 1,
};

//RBTree的节点
template<class T>
struct RBTreeNode
{
    //无效节点
    const static size_t INVALID_POINT = static_cast<size_t>(-1);
    const static unsigned short MAGIC_FLAG = 0x9528;

public:
    //清除数据
    void Clear()
    {
        magic_flag_ = MAGIC_FLAG;
        use_flag_   = 0;
        parent_     = INVALID_POINT;
        left_       = INVALID_POINT;
        right_      = INVALID_POINT;
        color_      = RB_TREE_RED;
        size_       = 0;
        memset(&data_, 0, sizeof(data_));
    }

public:
    //魔数标志
    unsigned short  magic_flag_;
    //使用标志
    unsigned short  use_flag_;
    //父节点
    size_t          parent_;
    //左子树
    size_t          left_;
    //右子树
    size_t          right_;
    //颜色
    unsigned char   color_;
    //以该节点为根节点的子树的节点的个数
    size_t          size_;
    //数据
    T               data_;
};

//前导声明
template<class _value_type,
    class _key_type,
    class _extract_key,
    class _compare_key>
class MemRBTree;

//RBtree的迭代器
template <class _value_type,
    class _key_type,
    class _extract_key,
    class _compare_key>
class MemRBTreeIterator
{
    typedef RBTreeNode<_value_type>  tree_node;
    typedef MemRBTreeIterator<_value_type, _key_type, _extract_key, _compare_key> iterator;
    typedef MemRBTree<_value_type , _key_type, _extract_key, _compare_key> mem_rbtree;

protected:
    //序列号
    size_t          serial_;
    //RBtree的实例指针
    mem_rbtree*      rb_tree_;

public:
    MemRBTreeIterator(size_t seq, mem_rbtree* instance) : serial_(seq),
         rb_tree_(instance)
    {
    }

    MemRBTreeIterator() : serial_(tree_node::INVALID_POINT),
         rb_tree_(NULL)
    {
    }

    ~MemRBTreeIterator()
    {
    }

    //初始化
    void initialize(size_t seq, mem_rbtree* instance)
    {
        serial_     = seq;
        rb_tree_    = instance;
    }

    //保留序号就可以再根据模版实例化对象找到相应数据,不用使用指针
    size_t getserial() const
    {
        return serial_;
    }

    bool operator==(const iterator &x) const
    {
        return (serial_ == x.serial_ && rb_tree_ == x.rb_tree_ );
    }

    bool operator!=(const iterator &x) const
    {
        return !(*this == x);
    }

    _value_type& operator*() const
    {
        return *(operator->());
    }

    //在多线程的环境下提供这个运送符号是不安全的,没有加锁,上层自己保证
    _value_type* operator->() const
    {
        tree_node& node = rb_tree_->data_[serial_];
        assert(node.magic_flag_ == tree_node::MAGIC_FLAG);
        return &node.data_;
    }

    iterator& operator++()
    {
        increment();
        return *this;
    }

    iterator operator++(int)
    {
        iterator tmp = *this;
        increment();
        return tmp;
    }

    iterator& operator--()
    {
        decrement();
        return *this;
    }

    iterator operator--(int)
    {
        iterator tmp = *this;
        decrement();
        return tmp;
    }

    //用于实现operator++，找下一个节点
    void increment()
    {
        if (rb_tree_->data_[serial_].right_ != tree_node::INVALID_POINT )
        {
            //如果有右子节点，就向右走，然后一直沿左子树走到底即可
            serial_ = rb_tree_->data_[serial_].right_;
            while (rb_tree_->data_[serial_].left_ != tree_node::INVALID_POINT)
            {
                serial_ = rb_tree_->data_[serial_].left_;
            }
        }
        else
        {
            //如果没有右子节点，找到父节点，如果当前节点是某个右子节点，就一直上溯到不为右子节点为止
            size_t y = rb_tree_->data_[serial_].parent_;
            while (serial_ == rb_tree_->data_[y].right_)
            {
                serial_ = y;
                y = rb_tree_->data_[y].parent_;
            }

            //若此时的右子节点不等于父节点，则父节点即是，否则就是当前节点。到header节点就结束
            if (rb_tree_->data_[serial_].right_ != y)
            {
                serial_ = y;
            }
        }
    }

    //用于实现operator--，找上一个节点
    void decrement()
    {
        //如果是红节点，且父节点的的父节点等于自己。说明这个是头结点，亦是end()，再走就从头开始。
        if ( rb_tree_->data_[serial_].color_ == RB_TREE_RED &&
            rb_tree_->data_[rb_tree_->data_[serial_].parent_].parent_ == serial_ )
        {
            //右子节点即是
            serial_ = rb_tree_->data_[serial_].right_;
        }
        //如果有左子节点
        else if ( rb_tree_->data_[serial_].left_ != tree_node::INVALID_POINT )
        {
            //令y指向左子节点，找到y的右子节点，向右走到底即是
            size_t y = rb_tree_->data_[serial_].left_;
            while ( rb_tree_->data_[y].right_ != tree_node::INVALID_POINT )
            {
                y = rb_tree_->data_[y].right_;
            }
            serial_ = y;
        }
        else
        {
            //找出父节点，如果当前节点是个左子节点，就一直上溯，直到不再为左子节点，则其的父节点即是
            size_t y = rb_tree_->data_[serial_].parent_;
            while ( serial_ == rb_tree_->data_[y].left_ )
            {
                serial_ = y;
                y = rb_tree_->data_[y].parent_;
            }
            serial_ = y;
        }
    }
};


/*
 * @RBTreeHeader  红黑树的头
 */
struct RBTreeHeader
{
    //NODE结点个数
    size_t              num_of_node_;
    //free node的列表
    size_t              free_node_list_;
    //root节点
    size_t              root_node_;
    //最左节点
    size_t              left_node_;
    //最右节点
    size_t              right_node_;
    //空闲节点数
    size_t              sz_free_node_;
    //USE的NODE个数
    size_t              sz_use_node_;
    //脏标志位
    unsigned int        dirty_flag_;
};

//萃取器
template <class T>
struct mem_identity
{
    const T& operator()(const T& x) const
    {
        return x;
    }
};

/*
 * @MemRBTree 内存红黑树
 * 由于数据恢复的存在并不适用于海量数据的情况.
 */
template<class _value_type,
    class _key_type,
    class _extract_key = mem_identity<_value_type>,
    class _compare_key = std::less<_key_type> >
class MemRBTree
{
public:
    //数据类型
    typedef _value_type     value_type;

    //
    typedef std::vector<_value_type>    value_array;

    //节点数据类型
    typedef RBTreeNode<_value_type>     node_type;

    //定义自己
    typedef MemRBTree<_value_type, _key_type, _extract_key, _compare_key> self;

    //定义迭代器
    typedef MemRBTreeIterator<_value_type, _key_type, _extract_key, _compare_key> iterator;

    //迭代器友元
    friend class MemRBTreeIterator<_value_type, _key_type, _extract_key, _compare_key>;

public:
    //获取分配大小
    static size_t GetAllocSize(size_t node_num)
    {
        return sizeof(RBTreeHeader) + sizeof(node_type) * (node_num + 1);
    }

    //
    int Init(char* mem, size_t node_num, bool restore = true)
    {
        //脏数据恢复
        value_array va;

        //
        header_         = (RBTreeHeader*)mem;
        data_           = (node_type*)(mem + sizeof(RBTreeHeader));

        if (restore)
        {
            //比对节点数
            if (header_->num_of_node_ != node_num)
            {
                return -1;
            }

            //脏标志，需要重新建树
            //这样做有个前提:数据量不是很大,否则内存都不一定能申请到.
            if (header_->dirty_flag_)
            {
                va.reserve(header_->sz_use_node_);

                for (size_t i = 0; i < node_num; ++i)
                {
                    if (0 == data_[i].use_flag_)
                    {
                        continue;
                    }

                    va.push_back(value(i));
                }

                assert(va.size() == header_->sz_use_node_);

                restore = false;
            }
        }

        if (!restore)
        {
            //节点数
            header_->num_of_node_       = node_num;
            clear();
        }

        for (size_t i = 0; i < va.size(); ++i)
        {
            iterator iter = insert_equal(va[i]);
            if (iter == end())
            {
                return -2;
            }
        }

        return 0;
    }

    void clear()
    {
        header_->free_node_list_    = 0;
        header_->root_node_         = node_type::INVALID_POINT;
        header_->sz_free_node_      = header_->num_of_node_;
        header_->sz_use_node_       = 0;
        header_->dirty_flag_        = 0;

        //清理头节点
        node(header())->Clear();
        right(header()) = header();
        left(header())  = header();

        //构造空闲列表
        for (size_t i = 0; i < header_->num_of_node_; ++i)
        {
            node(i)->Clear();
            right(i)        = i + 1;
        }
        right(header_->num_of_node_ - 1) = node_type::INVALID_POINT;
    }

    //找到第一个节点
    iterator begin()
    {
        return iterator(leftmost(), this);
    };

    //容器应该是前闭后开的,头节点视为最后一个index
    iterator end()
    {
        return iterator(header_->num_of_node_, this);
    }

    //所有节点都在free链上即是空
    bool empty()
    {
        return (header_->sz_free_node_ == header_->num_of_node_);
    }

    //在插入数据前调用,这个函数检查
    bool full()
    {
        return (header_->sz_free_node_ == 0);
    }

    size_t size() const
    {
        return header_->sz_use_node_;
    }

    size_t capacity() const
    {
        return header_->num_of_node_;
    }

    //空闲的节点个数
    size_t sizefree()
    {
        return header_->sz_free_node_;
    }

public:
    //允许重复key插入的插入函数
    iterator insert_equal(const _value_type& v)
    {
        size_t y = header();
        size_t x = root();
        while (x != node_type::INVALID_POINT)
        {
            y = x;
            x = _compare_key()(_extract_key()(v), key(x)) ? left(x) : right(x);
        }
        return InsertNode(x, y, v);
    }

    //重复key插入则失败的插入函数，Map、Sap用这个
    std::pair<iterator, bool> insert_unique(const _value_type& v)
    {
        size_t y    = header();
        size_t x    = root();
        bool comp   = true;
        while (x != node_type::INVALID_POINT)
        {
            y = x;
            comp = _compare_key()(_extract_key()(v), key(x));
            x = comp ? left(x) : right(x);
        }

        iterator j = iterator(y, this);
        if (comp)
        {
            //v<x，--j<x, v>=--j
            if (j == begin())
            {
                return std::pair<iterator, bool>(InsertNode(x, y, v), true);
            }
            else
            {
                --j;
            }
        }

        if (_compare_key()(key(j.getserial()), _extract_key()(v)))
        {
            return std::pair<iterator, bool>(InsertNode(x, y, v), true);
        }

        return std::pair<iterator, bool>(j, false);
    }

    //通过迭代器删除一个节点
    void erase(const iterator &pos)
    {
        RBTreeBalanceForErase(pos.getserial(), root(), leftmost(), rightmost());
        FreeNode(pos.getserial());
    }

    //通过起始迭代器删除一段节点
    size_t erase(iterator __first, iterator __last)
    {
        size_t num = 0;
        if (__first == begin() && __last == end())
        {
            num = size();
            clear();
        }
        else
        {
            while (__first != __last)
            {
                ++num;
                erase(__first++);
            }
        }
        return num;
    }

    //通过key删除节点，Map和Set用
    size_t erase_unique(const _key_type& k)
    {
        iterator it = find(k);
        if (it != end()) {
            erase(it);
            return 1;
        }
        return 0;
    }

    //通过value删除节点，Map和Set用
    size_t erase_unique_value(const _value_type& v)
    {
        _extract_key get_key;
        return erase_unique(get_key(v));
    }

    //通过key删除节点，Multimap和Multiset用
    size_t erase_equal(const _key_type& k)
    {
        iterator it_l = lower_bound(k);
        iterator it_u = upper_bound(k);
        return erase(it_l, it_u);
    }

    //通过值删除节点，Multimap和Multiset用
    size_t erase_equal_value(const _value_type& v)
    {
        _extract_key get_key;
        return erase_equal(get_key(v));
    }

    //找到第一个key值相同的节点
    iterator lower_bound(const _key_type& k)
    {
        size_t y = header();
        size_t x = root();
        while (x != node_type::INVALID_POINT)
        {
            if (!_compare_key()(key(x), k) )
            {
                y = x;
                x = left(x);
            }
            else
            {
                x = right(x);
            }
        }
        return iterator(y, this);
    }

    //找到最后一个key值相同的节点
    iterator upper_bound(const _key_type& k)
    {
        size_t y = header();
        size_t x = root();
        while (x != node_type::INVALID_POINT)
        {
            if (_compare_key()(k, key(x)))
            {
                y = x;
                x = left(x);
            }
            else
            {
                x = right(x);
            }
        }
        return iterator(y, this);
    }

    std::pair<iterator, iterator> equal_range(const _key_type& k)
    {
        return std::pair<iterator, iterator>(lower_bound(k), upper_bound(k));
    }

    //找到迭代器相同的节点
    bool find_if_exist(const iterator &iter)
    {
        iterator begin, end;
        begin = lower_bound(key(iter.getserial()));
        end = upper_bound(key(iter.getserial()));

        iterator t = begin;
        for(; t != end; ++t)
        {
            if(t == iter)
            {
                return true;
            }
        }

        if(t == iter)
        {
            return true;
        }

        return false;
    }
    //找key相同的节点
    iterator find(const _key_type& k)
    {
        size_t y = header();
        size_t x = root();

        while (x != node_type::INVALID_POINT)
        {
            if (!_compare_key()(key(x), k))
            {
                y = x;
                x = left(x);
            }
            else
            {
                x = right(x);
            }
        }

        iterator j = iterator(y, this);
        return (j == end() || _compare_key()(k, key(j.getserial()))) ? end() : j;
    }

    //找value相同的节点
    iterator find_value(const _value_type& v)
    {
        _extract_key get_key;
        return find(get_key(v));
    }

    //找value相同的节点，如未找到则插入
    _value_type& find_and_insert_if_noexist(const _value_type& v)
    {
        iterator iter = find_value(v);
        if (iter == end())
        {
            std::pair<iterator, bool> pair_iter = insert_equal(v);
            return (*(pair_iter.first));
        }

        return *iter;
    }

    int get_order(const iterator &iter, bool ignore_check = false)
    {
        if(ignore_check || find_if_exist(iter))
        {
            return GetIthnum(iter.getserial());
        }
        else
        {
            return 0;
        }
    }

    iterator get_iter_by_order(int order)
    {
        size_t x = GetIthnode(order);

        if(x == node_type::INVALID_POINT)
        {
            return end();
        }
        else
        {
            iterator j = iterator(x, this);
            return j;
        }
    }

    size_t get_total_node_num()
    {
        if (root() == node_type::INVALID_POINT)
        {
            return 0;
        }
        else
        {
            return size(root());
        }
    }


protected:
    //
    iterator InsertNode(size_t x, size_t parent_pos, const _value_type& v)
    {
        size_t curr_pos = AllocNode();
        if (curr_pos == node_type::INVALID_POINT)
        {
            return end();
        }

        SetDirtyFlag();

        if (parent_pos == header() || x != node_type::INVALID_POINT || _compare_key()(_extract_key()(v), key(parent_pos)))
        {
            left(parent_pos) = curr_pos;
            if ( parent_pos == header() )
            {
                root()      = curr_pos;
                rightmost() = curr_pos;
            }
            else if (parent_pos == leftmost())
            {
                leftmost() = curr_pos;
            }
        }
        else
        {
            right(parent_pos) = curr_pos;
            if (parent_pos == rightmost())
            {
                rightmost() = curr_pos;
            }
        }

        parent(curr_pos)        = parent_pos;
        left(curr_pos)          = node_type::INVALID_POINT;
        right(curr_pos)         = node_type::INVALID_POINT;
        node(curr_pos)->data_   = v;
        useflag(curr_pos)       = 1;
        size(curr_pos)          = 1;

        //往前回溯，路径上节点的size都+1
        size_t y = parent_pos;
        while (y != header())
        {
            size(y) += 1;
            y = parent(y);
        }

        RBTreeBalance(curr_pos, parent(header()));

        ClearDiryFlag();

        return iterator(curr_pos, this);
    }

    void RBTreeBalance(size_t pos, size_t& root)
    {
        color(pos) = RB_TREE_RED;
        while( pos != root && color(parent(pos)) == RB_TREE_RED )
        {
            if ( parent(pos) == left(parent(parent(pos))) )
            {
                size_t right_pos = right(parent(parent(pos)));
                if ( right_pos != node_type::INVALID_POINT && color(right_pos) == RB_TREE_RED )
                {
                    color(parent(pos))          = RB_TREE_BLACK;
                    color(right_pos)            = RB_TREE_BLACK;
                    color(parent(parent(pos)))  = RB_TREE_RED;
                    pos = parent(parent(pos));
                }
                else
                {
                    if ( pos == right(parent(pos)) )
                    {
                        pos = parent(pos);
                        RBTreeRotateLeft(pos, root);
                    }
                    color(parent(pos))          = RB_TREE_BLACK;
                    color(parent(parent(pos)))  = RB_TREE_RED;
                    RBTreeRotateRight(parent(parent(pos)), root);
                }
            }
            else
            {
                size_t left_pos = left(parent(parent(pos)));
                if (left_pos != node_type::INVALID_POINT && color(left_pos) == RB_TREE_RED)
                {
                    color(parent(pos))          = RB_TREE_BLACK;
                    color(left_pos)             = RB_TREE_BLACK;
                    color(parent(parent(pos)))  = RB_TREE_RED;
                    pos = parent(parent(pos));
                }
                else
                {
                    if (pos == left(parent(pos)))
                    {
                        pos = parent(pos);
                        RBTreeRotateRight(pos, root);
                    }
                    color(parent(pos))          = RB_TREE_BLACK;
                    color(parent(parent(pos)))  = RB_TREE_RED;
                    RBTreeRotateLeft(parent(parent(pos)), root);
                }
            }
        }

        color(root) = RB_TREE_BLACK;
    }

    //左旋函数
    //参数1：左旋节点
    //参数2：根节点
    void RBTreeRotateLeft(size_t x, size_t& root)
    {
        size_t y = right(x);
        right(x) = left(y);
        if ( left(y) != node_type::INVALID_POINT)
        {
            parent(left(y)) = x;
        }
        parent(y) = parent(x);

        if ( x == root )
        {
            root = y;
        }
        else if ( x == left(parent(x)) )
        {
            left(parent(x)) = y;
        }
        else
        {
            right(parent(x)) = y;
        }
        left(y) = x;
        parent(x) = y;

        size(x) = 1;
        if(left(x) != node_type::INVALID_POINT)
        {
            size(x) += size(left(x));
        }
        if(right(x) != node_type::INVALID_POINT)
        {
            size(x) += size(right(x));
        }

        size(y) = 1;
        if(left(y) != node_type::INVALID_POINT)
        {
            size(y) += size(left(y));
        }
        if(right(y) != node_type::INVALID_POINT)
        {
            size(y) += size(right(y));
        }
    }

    //右旋函数
    //参数1：右旋节点
    //参数2：根节点
    void RBTreeRotateRight(size_t x, size_t& root)
    {
        size_t y = left(x);
        left(x) = right(y);
        if ( right(y) != node_type::INVALID_POINT )
        {
            parent(right(y)) = x;
        }
        parent(y) = parent(x);

        if ( x == root )
        {
            root = y;
        }
        else if ( x == right(parent(x)) )
        {
            right(parent(x)) = y;
        }
        else
        {
            left(parent(x)) = y;
        }
        right(y)    = x;
        parent(x)   = y;

        size(x) = 1;
        if(left(x) != node_type::INVALID_POINT)
        {
            size(x) += size(left(x));
        }
        if(right(x) != node_type::INVALID_POINT)
        {
            size(x) += size(right(x));
        }

        size(y) = 1;
        if(left(y) != node_type::INVALID_POINT)
        {
            size(y) += size(left(y));
        }
        if(right(y) != node_type::INVALID_POINT)
        {
            size(y) += size(right(y));
        }
    }

    //删除时的树形调整，让其符合RBTree要求
    size_t RBTreeBalanceForErase(size_t cur_pos, size_t& root, size_t& leftmost, size_t& rightmost)
    {
        SetDirtyFlag();

        size_t y        = cur_pos;
        size_t x        = node_type::INVALID_POINT;
        size_t x_parent = node_type::INVALID_POINT;

        //根据当前节点的子节点情况，决定如何调整树
        if (left(y) == node_type::INVALID_POINT)
        {
            x = right(y);
        }
        else if (right(y) == node_type::INVALID_POINT)
        {
            x = left(y);
        }
        else
        {
            y = minimum(right(y));
            x = right(y);
        }

        //当前节点的左右子树都存在，将右子树的最左节点提升到当前节点的位置
        if (y != cur_pos)
        {
            parent(left(cur_pos)) = y;
            left(y) = left(cur_pos);
            if (y != right(cur_pos))
            {
                //将右子树作为左子树挂到父节点上。是minimum节点，所以一定无左子树。
                x_parent = parent(y);
                if (x != node_type::INVALID_POINT)
                {
                    parent(x) = parent(y);
                }
                left(parent(y)) = x;
                right(y) = right(cur_pos);
                parent(right(cur_pos)) = y;
            }
            else
            {
                x_parent = y;
            }

            if ( root == cur_pos )
            {
                root = y;
            }
            else if (left(parent(cur_pos)) == cur_pos)
            {
                left(parent(cur_pos)) = y;
            }
            else
            {
                right(parent(cur_pos)) = y;
            }

            parent(y) = parent(cur_pos);
            unsigned char  c = color(y);
            color(y) = color(cur_pos);
            color(cur_pos) = c;
            size(y) = size(cur_pos);
            y = cur_pos;
        }
        else
        {
            //直接将子节点替换当前节点
            x_parent = parent(y);
            if (x != node_type::INVALID_POINT)
            {
                parent(x) = parent(y);
            }

            if (root == cur_pos)
            {
                root = x;
            }
            else
            {
                if (left(parent(cur_pos)) == cur_pos)
                {
                    left(parent(cur_pos)) = x;
                }
                else
                {
                    right(parent(cur_pos)) = x;
                }
            }

            size(y) = size(cur_pos);

            //如果最左、最右节点发生变化，做对应的调整
            if (leftmost == cur_pos)
            {
                if (right(cur_pos) == node_type::INVALID_POINT)
                {
                    leftmost = parent(cur_pos);
                }
                else
                {
                    leftmost = minimum(x);
                }
            }
            if (rightmost == cur_pos)
            {
                if (left(cur_pos) == node_type::INVALID_POINT)
                {
                    rightmost = parent(cur_pos);
                }
                else
                {
                    rightmost = maximum(x);
                }
            }
        }


        //往上回溯， 节点size-1
        size_t z = x_parent;
        while (z != header())
        {
            size(z) -= 1;
            z = parent(z);
        }

        //重新平衡
        if (color(y) != RB_TREE_RED )
        {
            while (x != root && (x == node_type::INVALID_POINT || color(x) == RB_TREE_BLACK))
            {
                if (x == left(x_parent))
                {
                    size_t w = right(x_parent);
                    if (color(w) == RB_TREE_RED)
                    {
                        color(w)        = RB_TREE_BLACK;
                        color(x_parent) = RB_TREE_RED;
                        RBTreeRotateLeft(x_parent, root);
                        w               = right(x_parent);
                    }
                    if ((left(w) == node_type::INVALID_POINT || color(left(w)) == RB_TREE_BLACK) &&
                        (right(w) == node_type::INVALID_POINT || color(right(w)) == RB_TREE_BLACK ))
                    {
                        color(w)    = RB_TREE_RED;
                        x           = x_parent;
                        x_parent    = parent(x_parent);
                    }
                    else
                    {
                        if ( right(w) == node_type::INVALID_POINT || color(right(w)) == RB_TREE_BLACK)
                        {
                            if (left(w) != node_type::INVALID_POINT)
                            {
                                color(left(w)) = RB_TREE_BLACK;
                            }
                            color(w)    = RB_TREE_RED;
                            RBTreeRotateRight(w, root);
                            w           = right(x_parent);
                        }
                        color(w)        = color(x_parent);
                        color(x_parent) = RB_TREE_BLACK;
                        if (right(w) != node_type::INVALID_POINT)
                        {
                            color(right(w)) = RB_TREE_BLACK;
                        }
                        RBTreeRotateLeft(x_parent, root);
                        break;
                    }
                }
                else
                {
                    size_t w = left(x_parent);
                    if (color(w) == RB_TREE_RED)
                    {
                        color(w)        = RB_TREE_BLACK;
                        color(x_parent) = RB_TREE_RED;
                        RBTreeRotateRight(x_parent, root);
                        w = left(x_parent);
                    }
                    if ((right(w) == node_type::INVALID_POINT || color(right(w)) == RB_TREE_BLACK) &&
                        (left(w) == node_type::INVALID_POINT || color(left(w)) == RB_TREE_BLACK))
                    {
                        color(w)    = RB_TREE_RED;
                        x           = x_parent;
                        x_parent    = parent(x_parent);
                    }
                    else
                    {
                        if (left(w) == node_type::INVALID_POINT || color(left(w)) == RB_TREE_BLACK)
                        {
                            if (right(w) != node_type::INVALID_POINT)
                            {
                                color(right(w)) = RB_TREE_BLACK;
                            }
                            color(w)    = RB_TREE_RED;
                            RBTreeRotateLeft(w, root);
                            w           = left(x_parent);
                        }
                        color(w)        = color(x_parent);
                        color(x_parent) = RB_TREE_BLACK;
                        if (left(w) != node_type::INVALID_POINT)
                        {
                            color(left(w)) = RB_TREE_BLACK;
                        }
                        RBTreeRotateRight(x_parent, root);
                        break;
                    }
                }
            }

            if (x != node_type::INVALID_POINT)
            {
                color(x) = RB_TREE_BLACK;
            }
        }

        ClearDiryFlag();

        return y;
    }

    //根据顺序统计量 找到该节点
    size_t GetIthnode(int num)
    {
        size_t x = root();
        int r = 0;

        if (num <= 0) //输入非法
        {
            return node_type::INVALID_POINT;
        }

        if (x == node_type::INVALID_POINT)
        {
            return node_type::INVALID_POINT;
        }

        if(num > (int)size(x))
        {
            return node_type::INVALID_POINT;
        }
        while (x != node_type::INVALID_POINT)
        {
            r = 1;
            if (left(x) != node_type::INVALID_POINT)
            {
                r += size(left(x));
            }
            if (r == num)
            {
                return x;
            }
            else if (r > num)
            {
                x = left(x);
            }
            else
            {
                x = right(x);
                num -= r;
            }
        }
        return node_type::INVALID_POINT;  //表示没有找到
    }

    //根据指定节点，获取顺序统计量, 这里必须保证pos能找到
    int GetIthnum(size_t pos)
    {
        int num = 1;
        size_t x = pos;
        size_t y = 0;
        //初始化
        if (left(x) != node_type::INVALID_POINT)
        {
            num += size(left(x));
        }

        while (x != root())
        {
            y = parent(x);
            //如果x是其父亲的右子树
            if (x == right(y))
            {
                num += 1;
                //其兄弟不为空
                if (left(y) != node_type::INVALID_POINT)
                {
                    num += size(left(y));
                }
            }
            x = y;
        }
        return num;
    }

protected:
    //
    void SetDirtyFlag() {header_->dirty_flag_ = 1;}
    void ClearDiryFlag() {header_->dirty_flag_ = 0;}
    //
    size_t& header()         {return header_->num_of_node_;}
    size_t& root()           {return parent(header());}
    size_t& leftmost()       {return left(header());}
    size_t& rightmost()      {return right(header());}
    size_t& left(size_t pos)       {return data_[pos].left_;}
    size_t& right(size_t pos)      {return data_[pos].right_;}
    size_t& parent(size_t pos)     {return data_[pos].parent_;}
    size_t& size(size_t pos)       {return data_[pos].size_;}
    unsigned char& color(size_t pos)     {return data_[pos].color_;}
    const _value_type& value(size_t pos) {return data_[pos].data_; }
    const _key_type& key(size_t pos)     {return _extract_key()(value(pos));}
    node_type* node(size_t pos) {return &data_[pos];}
    unsigned short& useflag(size_t pos) { return data_[pos].use_flag_; }
    //取极小值
    size_t minimum(size_t pos)
    {
        while (left(pos) != node_type::INVALID_POINT)
        {
            pos = left(pos);
        }

        return pos;
    }
    //取极大值
    size_t maximum(size_t pos)
    {
        while (right(pos) != node_type::INVALID_POINT)
        {
            pos = right(pos);
        }

        return pos;
    }

protected:
    //
    size_t AllocNode()
    {
        if (header_->sz_free_node_ == 0)
        {
            return node_type::INVALID_POINT;
        }

        size_t pos                  = header_->free_node_list_;
        header_->free_node_list_    = right(pos);
        header_->sz_use_node_++;
        header_->sz_free_node_--;

        return pos;
    }
    //
    void FreeNode(size_t pos)
    {
        node(pos)->Clear();
        right(pos)                  = header_->free_node_list_;
        header_->free_node_list_    = pos;
        header_->sz_free_node_++;
        header_->sz_use_node_--;
    }


protected:
    //
    RBTreeHeader*       header_;
    //
    node_type*       data_;
};


#endif /* MEM_RBTREE_V2_H_ */
