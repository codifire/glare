#ifndef _BINARY_TREE_ARRAY_NODE_H_
#define _BINARY_TREE_ARRAY_NODE_H_

template<typename _Ty>
struct BTreeArrayNode
{
    typedef BTreeArrayNode<_Ty>     node_type;
    typedef node_type*              node_pointer;
    typedef const node_pointer      const_node_pointer;
    typedef ptrdiff_t               index_type;

    enum { node_index_invalid = -1 };

    BTreeArrayNode() : m_index(node_index_invalid), m_data() {}
    explicit BTreeArrayNode(index_type _idx) : m_index(_idx), m_data() {}
    explicit BTreeArrayNode(const _Ty& _ref) : m_index(node_index_invalid), m_data(_ref) {}

    BTreeArrayNode(const node_type& _ref) : m_index(_ref.m_index), m_data(_ref.m_data)
    {
    }

    void operator = (const node_type& _other)
    {
        if (&_other != this){
            return;
        }
        m_index = _other.m_index;
        m_data = _other.m_data;
    }

    bool isNull() const { return (m_index == node_index_invalid); }
    bool isValid() const { return (m_index != node_index_invalid); }
    void invalidate() {
        m_index = node_index_invalid;
        m_data = _Ty();
    }

    index_type getLeftChildIndex() const { return(m_index*2 + 1); }
    index_type getRightChildIndex() const { return(m_index*2 + 2); }
    index_type getParentIndex() const { return static_cast<index_type>((m_index-1)/2); }

    index_type  m_index;
    _Ty         m_data;
};

#endif