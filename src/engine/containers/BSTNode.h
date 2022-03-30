#ifndef _BINARY_TREE_NODE_
#define _BINARY_TREE_NODE_

#include "GlareCoreUtility.h"

namespace glare
{
    template<typename T>
    struct BasicNode
    {
    public:
        typedef BasicNode<T>            node_type;
        typedef node_type*              node_pointer;
        typedef const node_type*        const_node_pointer;
        typedef node_type&              node_reference;
        typedef const node_type&        const_node_reference;
        typedef T                       data_type;


        BasicNode(const T& obj) : m_left(NULL)
                                , m_right(NULL)
                                , m_data(obj) 
        {}

        BasicNode* m_left;
        BasicNode* m_right;

        T m_data;
    };

    // ---------------------------------------------------------------------------------------------------------
    // BINARY SEARCH TREE NODE
    // ---------------------------------------------------------------------------------------------------------

    template<typename keyType, typename ValueType, typename NodeType = BSTNode<keyType, ValueType> >
    class BSTNode
    {
    public:
        typedef BSTNode<keyType, ValueType, NodeType> selftype;
        typedef NodeType                            node_type;
        typedef node_type*                          node_pointer;
        typedef const node_type*                    const_node_pointer;
        typedef GLARE_PAIR<keyType, ValueType>      value_type;

        typedef ValueType                           ValueType;
        typedef keyType                             KeyType;

    public:
        BSTNode(): m_left(nullptr)
                 , m_right(nullptr)
                 , m_pair()
        {}

        explicit BSTNode(const value_type& obj): m_left(nullptr)
                                              , m_right(nullptr)
                                              , m_pair(obj)
        {}

        BSTNode(const BSTNode& obj): m_left(nullptr)
                                   , m_right(nullptr)
                                   , m_pair(obj.m_pair)
        {}

        BSTNode& operator = (const BSTNode& right)
        {
            if (&right != this)
            {
                copyData(right);
            }
            return *this;
        }

        const keyType&      key() const { return m_pair.first; }
        const ValueType&    value() const { return m_pair.second; }
        ValueType&          value() { return m_pair.second; }
        void                copyData(const selftype& _other) { m_pair = _other.m_pair; }
        value_type&         getData() { return m_pair; }
        const value_type&   getData() const { return m_pair; }

        node_pointer        left() { return m_left; }
        node_pointer        right() { return m_right; }

        const node_pointer  left() const { return m_left; }
        const node_pointer  right() const { return m_right; }

    private:
        value_type m_pair;

    public:
        node_type* m_left;
        node_type* m_right;
    };

} // namespace
#endif
