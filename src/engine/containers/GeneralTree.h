#ifndef GENERAL_TREE_IMPL_H
#define GENERAL_TREE_IMPL_H

#include "DLinkList.h"

namespace glare
{
    template<typename T, typename Alloc = default_allocator<T> >
    class GTree
    {
        struct GTreeNode;

        template<typename T, typename NODE>
        class gtree_iterator;

        template<typename T, typename NODE>
        class gtree_const_iterator;

    public:
        typedef T                   value_type;
        typedef value_type*         pointer;
        typedef const value_type*   const_pointer;
        typedef value_type&         reference;
        typedef const value_type&   const_reference;
        typedef std::size_t         size_type;
        typedef std::ptrdiff_t      difference_type;

        typedef GTreeNode           node_type;
        typedef node_type*          node_pointer;
        typedef const node_type*    const_node_pointer;
        typedef DLinkList<node_pointer> node_pointer_list;
        typedef gtree_iterator<value_type, node_type> iterator;
        typedef gtree_const_iterator<value_type, const node_type> const_iterator;
        typedef void (*process_data_cb)(value_type& vt);

        GTree() : m_root(nullptr), m_size(0), traversalFunc(preorder) {}
        ~GTree() { destroy(m_root); }

        size_t          size() {return m_size;}
        node_pointer    root() {return m_root;}
        bool            empty() { return (m_size == 0); }
        void            setPreOrderTraversal() { traversalFunc = preorder; }
        void            setPostOrderTraversal() { traversalFunc = postorder; }

        void            traverse(iterator itr, process_data_cb pFunc)
        {
            traversalFunc(*itr, pFunc);
        }

        void            addRoot(const_reference ref);
        void            addChild(iterator itr, const_reference ref);

        void            destroy(node_pointer ptr);

        size_t          count(node_pointer ptr);

    protected:

        static void preorder(node_pointer pNode, process_data_cb pFunc)
        {
            pFunc(pNode->m_data);
            node_pointer_list::iterator itr = pNode->m_children.begin();
            while(itr != pNode->m_children.end())
            {
                preorder(*itr, pFunc);
                ++itr;
            }
        }
        static void postorder(node_pointer pNode, process_data_cb pFunc)
        {
            node_pointer_list::iterator itr = pNode->m_children.begin();
            while(itr != pNode->m_children.end())
            {
                preorder(*itr, pFunc);
                ++itr;
            }
            pFunc(pNode->m_data);
        }

        struct GTreeNode
        {
            GTreeNode(const_reference ref) : m_data(ref), m_parent(nullptr) {}
            bool isExternal() { return (m_children.empty() == true); }
            bool isInternal() { return !isExternal(); }
            bool isRoot() { return (m_parent == nullptr); }

            value_type    m_data;
            node_pointer  m_parent;
            node_pointer_list  m_children;
        };

        template<typename T, typename NODE>
        struct gtree_iterator_base : public std::iterator<std::bidirectional_iterator_tag, T>
        {
            typedef NODE*                   node_pointer;
            typedef DLinkList<node_pointer> node_pointer_list;

            gtree_iterator_base(node_pointer ptr=0) : m_node(ptr) { resetChildItr(); }

            void resetChildItr() 
            {
                if (m_node)
                {
                    m_childItr = m_node->m_children.begin();
                }
                else
                {
                    m_childItr = node_pointer_list::iterator(nullptr);
                }
            }
            void root()
            {
                m_node = GTree<T>::root();
                resetChildItr();
            }
            void up()
            {
                if (m_node != nullptr)
                {
                    m_node = m_node->m_parent;
                }
                resetChildItr();
            }
            void down()
            {
                if (m_childItr != iterator(nullptr)) // To keep more flexible.
                {
                    m_node = *m_childItr;
                    resetChildItr();
                }
            }
            void childForth()
            {
                ++m_childItr;
                assert(m_childItr != m_node->m_children.end())
            }
            void childBack()
            {
                assert(m_childItr != m_node->m_children.begin())
                    --m_childItr;
            }
            bool isChildFirst()
            {
                return (m_childItr == m_node->m_children.begin());
            }
            bool isChildEnd()
            {
                return (m_childItr == m_node->m_children.end());
            }

        protected:
            friend class GTree<T, Alloc>;

            node_pointer m_node;
            typename node_pointer_list::iterator m_childItr;
        };

        template<typename T, typename NODE>
        class gtree_iterator : public gtree_iterator_base<T, NODE>
        {
        public:
            gtree_iterator& operator++(/*Pre-increment*/) {down(); return *this;}
            gtree_iterator operator++(int /*Post-increment*/) {iterator tmp(*this); down(); return tmp;}

            gtree_iterator& operator--(/*Pre-decrement*/) {up(); return *this;}
            gtree_iterator operator--(int /*Post-decrement*/) {iterator tmp(*this); up(); return tmp;}

            bool operator==(const gtree_iterator& rhs) const {return m_node == rhs.m_node;}
            bool operator!=(const gtree_iterator& rhs) const {return m_node != rhs.m_node;}

            reference operator*() { return ptr->m_data; }
            pointer operator->() { return &(ptr->m_data); }

            gtree_iterator(node_pointer ptr=0) : gtree_iterator_base(ptr) { }
        };

        template<typename T, typename NODE>
        class gtree_const_iterator : public gtree_iterator_base<T, NODE>
        {
        public:
            gtree_const_iterator& operator++(/*Pre-increment*/) {down(); return *this;}
            gtree_const_iterator operator++(int /*Post-increment*/) {iterator tmp(*this); down(); return tmp;}

            gtree_const_iterator& operator--(/*Pre-decrement*/) {up(); return *this;}
            gtree_const_iterator operator--(int /*Post-decrement*/) {iterator tmp(*this); up(); return tmp;}

            bool operator==(const gtree_const_iterator& rhs) const {return m_node == rhs.m_node;}
            bool operator!=(const gtree_const_iterator& rhs) const {return m_node != rhs.m_node;}

            const_reference operator*() { return ptr->m_data; }
            const_pointer operator->() { return &(ptr->m_data); }

            gtree_const_iterator(node_pointer ptr=0) : gtree_iterator_base(ptr) { }
        };

        node_pointer constructNode(const_reference ref)
        {
            node_pointer ptr = m_nodeAllocator.allocate(1);
            m_nodeAllocator.construct(ptr, ref);
            return ptr;
        }
        void destroyNode(node_pointer ptr)
        {
            m_nodeAllocator.destroy(ptr); 
            m_nodeAllocator.deallocate(ptr, 1);
        }

        node_pointer m_root;
        size_t  m_size;
        void (*traversalFunc)(node_pointer pNode, process_data_cb pFunc);

        // Alloc m_allocator;
        typename Alloc::template rebind<node_type>::other m_nodeAllocator;
    }; // End Class

    template<typename T, typename Alloc>
    void GTree<T, Alloc>::destroy(node_pointer pNode)
    {
        typename node_pointer_list::iterator itr = pNode->m_children.begin();
        while(itr != pNode->m_children.end())
        {
            node_pointer ptr = *itr;
            itr = pNode->m_children.erase(itr);  // returns next
            destroy(ptr);
        }
        destroyNode(pNode);
    }

    template<typename T, typename Alloc>
    size_t GTree<T, Alloc>::count(node_pointer pNode)
    {
        size_t c = 1; // count current node.
        node_pointer_list::iterator itr = pNode->m_children.begin();
        for (; itr != pNode->m_children.end(); ++itr)
        {
            c += count(*itr); // count children nodes.
        }
        return c;
    }

    template<typename T, typename Alloc>
    void GTree<T, Alloc>::addRoot(const_reference ref)
    {
        node_pointer node = constructNode(ref);
        node->m_parent = nullptr; // 'cos new root node.

        if (m_root)
        {
            m_root->m_parent = node;
            node->m_children.push_back(m_root);
        }

        m_root = node;
        ++m_size;
    }

    template<typename T, typename Alloc>
    void GTree<T, Alloc>::addChild(iterator itr, const_reference ref)
    {
        node_pointer node = constructNode(ref);
        node->m_parent = itr.m_node
            itr.m_childItr.push_back(node);
        ++m_size;
    }
} // End of namespace.

#endif
