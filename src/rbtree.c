#include "rbtree.h"

#if defined _RB_DEBUG
#include <assert.h>
#endif

static struct rb_node *
node_min(const struct rb_node *node)
{
    while (!node->_left->_isnil)
    {
        node = node->_left;
    }

    return (struct rb_node *)node;
}

static struct rb_node *
node_max(const struct rb_node *node)
{
    while (!node->_right->_isnil)
    {
        node = node->_right;
    }

    return (struct rb_node *)node;
}

static struct rb_node *
node_prev(const struct rb_node *node)
{
    if (node->_isnil)
    {
        node = node->_right;
    }
    else if (node->_left->_isnil)
    {
        struct rb_node *parent;

        while (!(parent = node->_parent)->_isnil && node == parent->_left)
        {
            node = parent;
        }

        if (!node->_isnil)
        {
            node = parent;
        }
    }
    else
    {
        node = node_max(node->_left);
    }

    return (struct rb_node *)node;
}

static struct rb_node *
node_next(const struct rb_node *node)
{
    if (node->_right->_isnil)
    {
        struct rb_node *parent;

        while (!(parent = node->_parent)->_isnil && node == parent->_right)
        {
            node = parent;
        }

        node = parent;
    }
    else
    {
        node = node_min(node->_right);
    }

    return (struct rb_node *)node;
}

static void
impl_rotate_left(struct _rb_impl *impl, struct rb_node *node)
{
    struct rb_node *root = node->_right;

    node->_right = root->_left;

    if (!root->_left->_isnil)
    {
        root->_left->_parent = node;
    }

    root->_parent = node->_parent;

    if (node == _RB_IMPL_ROOT(impl))
    {
        _RB_IMPL_ROOT(impl) = root;
    }
    else if (node == node->_parent->_left)
    {
        node->_parent->_left = root;
    }
    else
    {
        node->_parent->_right = root;
    }

    root->_left = node;
    node->_parent = root;
}

static void
impl_rotate_right(struct _rb_impl *impl, struct rb_node *node)
{
    struct rb_node *root = node->_left;

    node->_left = root->_right;

    if (!root->_right->_isnil)
    {
        root->_right->_parent = node;
    }

    root->_parent = node->_parent;

    if (node == _RB_IMPL_ROOT(impl))
    {
        _RB_IMPL_ROOT(impl) = root;
    }
    else if (node == node->_parent->_right)
    {
        node->_parent->_right = root;
    }
    else
    {
        node->_parent->_left = root;
    }

    root->_right = node;
    node->_parent = root;
}

static int
impl_comp(const struct _rb_impl *impl,
    const struct rb_node *n1, const struct rb_node *n2)
{
#define _IMPL_COMP(impl, n1, n2) \
    ((impl)->_comp(n1, n2, (impl)->_args))

    int cmpr = _IMPL_COMP(impl, n1, n2);

#if defined _RB_DEBUG
    assert(!cmpr || !_IMPL_COMP(impl, n2, n1) && "reflexivity detected");
#endif

    return cmpr;

#undef _IMPL_COMP
}

static int
rb_comp(const struct rb_tree *rb,
    const struct rb_node *n1, const struct rb_node *n2)
{
    return impl_comp(_RB_IMPL(rb), n1, n2);
}

static struct rb_node *
impl_lbnd(const struct _rb_impl *impl, const struct rb_node *val)
{
    const struct rb_node *parent = _RB_IMPL_HEAD(impl);
    const struct rb_node *node = parent->_parent;

    while (!node->_isnil)
    {
        if (impl_comp(impl, node, val))
        {
            node = node->_right;
        }
        else
        {
            parent = node;
            node = node->_left;
        }
    }

    return (struct rb_node *)parent;
}

static struct rb_node *
impl_ubnd(const struct _rb_impl *impl, const struct rb_node *val)
{
    const struct rb_node *parent = _RB_IMPL_HEAD(impl);
    const struct rb_node *node = parent->_parent;

    while (!node->_isnil)
    {
        if (impl_comp(impl, val, node))
        {
            parent = node;
            node = node->_left;
        }
        else
        {
            node = node->_right;
        }
    }

    return (struct rb_node *)parent;
}

static struct rb_pair
impl_eqrange(const struct _rb_impl *impl, const struct rb_node *val)
{
    struct rb_pair pr;

    const struct rb_node *node = _RB_IMPL_ROOT(impl);
    const struct rb_node *begin = _RB_IMPL_HEAD(impl);
    const struct rb_node *end = _RB_IMPL_HEAD(impl);

    while (!node->_isnil)
    {
        if (impl_comp(impl, node, val))
        {
            node = node->_right;
        }
        else
        {
            if (end->_isnil && impl_comp(impl, val, node))
            {
                end = node;
            }
            begin = node;
            node = node->_left;
        }
    }
    node = end->_isnil ? _RB_IMPL_ROOT(impl) : end->_left;
    while (!node->_isnil)
    {
        if (impl_comp(impl, val, node))
        {
            end = node;
            node = node->_left;
        }
        else
        {
            node = node->_right;
        }
    }

    pr.first = (struct rb_node *)begin, pr.second = (struct rb_node *)end;
    return pr;
}

static struct rb_node *
rb_erase_node(struct rb_tree *rb, struct rb_node *node)
{
#define SWAP_COLOR(c1, c2) \
    do { \
        char __tmp = (c1); \
        (c1) = (c2); \
        (c2) = (c1); \
    } while (0)

    struct _rb_impl *impl = _RB_IMPL(rb);

    struct rb_node *fixnode;
    struct rb_node *fixparent;
    struct rb_node *erased = node;
    struct rb_node *pnode = erased;

    node = node_next(node);

    if (pnode->_left->_isnil)
    {
        fixnode = pnode->_right;
    }
    else if (pnode->_right->_isnil)
    {
        fixnode = pnode->_left;
    }
    else
    {
        pnode = node;
        fixnode = pnode->_right;
    }

    if (pnode == erased)
    {
        fixparent = erased->_parent;
        if (!fixnode->_isnil)
        {
            fixnode->_parent = fixparent;
        }
        if (_RB_IMPL_ROOT(impl) == erased)
        {
            _RB_IMPL_ROOT(impl) = fixnode;
        }
        else if (fixparent->_left == erased)
        {
            fixparent->_left = fixnode;
        }
        else
        {
            fixparent->_right = fixnode;
        }

        if (_RB_IMPL_LMST(impl) == erased)
        {
            _RB_IMPL_LMST(impl) = fixnode->_isnil ?
                fixparent : node_min(fixnode);
        }
        if (_RB_IMPL_RMST(impl) == erased)
        {
            _RB_IMPL_RMST(impl) = fixnode->_isnil ?
                fixparent : node_max(fixnode);
        }
    }
    else
    {
        erased->_left->_parent = pnode;
        pnode->_left = erased->_left;

        if (pnode == erased->_right)
        {
            fixparent = pnode;
        }
        else
        {
            fixparent = pnode->_parent;

            if (!fixnode->_isnil)
            {
                fixnode->_parent = fixparent;
            }

            fixparent->_left = fixnode;
            pnode->_right = erased->_right;
            erased->_right->_parent = pnode;
        }
        if (_RB_IMPL_ROOT(impl) == erased)
        {
            _RB_IMPL_ROOT(impl) = pnode;
        }
        else if (erased->_parent->_left == erased)
        {
            erased->_parent->_left = pnode;
        }
        else
        {
            erased->_parent->_right = pnode;
        }

        pnode->_parent = erased->_parent;
        SWAP_COLOR(pnode->_color, erased->_color);
    }

    if (erased->_color == _RB_BLACK)
    {
        for (; fixnode != _RB_IMPL_ROOT(impl)
            && fixnode->_color == _RB_BLACK;
            fixparent = fixnode->_parent)
        {
            if (fixnode == fixparent->_left)
            {
                pnode = fixparent->_right;

                if (pnode->_color == _RB_RED)
                {
                    pnode->_color = _RB_BLACK;
                    fixparent->_color = _RB_RED;
                    impl_rotate_left(impl, fixparent);
                    pnode = fixparent->_right;
                }

                if (pnode->_isnil)
                {
                    fixnode = fixparent;
                }
                else if (pnode->_left->_color == _RB_BLACK
                    && pnode->_right->_color == _RB_BLACK)
                {
                    pnode->_color = _RB_RED;
                    fixnode = fixparent;
                }
                else
                {
                    if (pnode->_right->_color == _RB_BLACK)
                    {
                        pnode->_left->_color = _RB_BLACK;
                        pnode->_color = _RB_RED;
                        impl_rotate_right(impl, pnode);
                        pnode = fixparent->_right;
                    }

                    pnode->_color = fixparent->_color;
                    fixparent->_color = _RB_BLACK;
                    pnode->_right->_color = _RB_BLACK;
                    impl_rotate_left(impl, fixparent);
                    break;
                }
            }
            else
            {
                pnode = fixparent->_left;

                if (pnode->_color == _RB_RED)
                {
                    pnode->_color = _RB_BLACK;
                    fixparent->_color = _RB_RED;
                    impl_rotate_right(impl, fixparent);
                    pnode = fixparent->_left;
                }

                if (pnode->_isnil)
                {
                    fixnode = fixparent;
                }
                else if (pnode->_right->_color == _RB_BLACK
                    && pnode->_left->_color == _RB_BLACK)
                {
                    pnode->_color = _RB_RED;
                    fixnode = fixparent;
                }
                else
                {
                    if (pnode->_left->_color == _RB_BLACK)
                    {
                        pnode->_right->_color = _RB_BLACK;
                        pnode->_color = _RB_RED;
                        impl_rotate_left(impl, pnode);
                        pnode = fixparent->_left;
                    }

                    pnode->_color = fixparent->_color;
                    fixparent->_color = _RB_BLACK;
                    pnode->_left->_color = _RB_BLACK;
                    impl_rotate_right(impl, fixparent);
                    break;
                }
            }
        }

        fixnode->_color = _RB_BLACK;
    }
    --rb->size;

    return node;

#undef SWAP_COLOR
}

static struct rb_node *
impl_node_insert(struct rb_tree *rb,
    struct rb_node *node, struct rb_node *pos, int addleft)
{
    struct _rb_impl *impl = _RB_IMPL(rb);

    node->_parent = pos;

    if (pos == _RB_IMPL_HEAD(impl))
    {
        _RB_IMPL_ROOT(impl) = node;
        _RB_IMPL_LMST(impl) = node;
        _RB_IMPL_RMST(impl) = node;
    }
    else if (addleft)
    {
        pos->_left = node;

        if (pos == _RB_IMPL_LMST(impl))
        {
            _RB_IMPL_LMST(impl) = node;
        }
    }
    else
    {
        pos->_right = node;

        if (pos == _RB_IMPL_RMST(impl))
        {
            _RB_IMPL_RMST(impl) = node;
        }
    }

    for (struct rb_node *pnode = node; pnode->_parent->_color == _RB_RED; )
    {
        if (pnode->_parent == pnode->_parent->_parent->_left)
        {
            pos = pnode->_parent->_parent->_right;

            if (pos->_color == _RB_RED)
            {
                pnode->_parent->_color = _RB_BLACK;
                pos->_color = _RB_BLACK;
                pnode->_parent->_parent->_color = _RB_RED;
                pnode = pnode->_parent->_parent;
            }
            else
            {
                if (pnode == pnode->_parent->_right)
                {
                    pnode = pnode->_parent;
                    impl_rotate_left(impl, pnode);
                }

                pnode->_parent->_color = _RB_BLACK;
                pnode->_parent->_parent->_color = _RB_RED;
                impl_rotate_right(impl, pnode->_parent->_parent);
            }
        }
        else
        {
            pos = pnode->_parent->_parent->_left;

            if (pos->_color == _RB_RED)
            {
                pnode->_parent->_color = _RB_BLACK;
                pos->_color = _RB_BLACK;
                pnode->_parent->_parent->_color = _RB_RED;
                pnode = pnode->_parent->_parent;
            }
            else
            {
                if (pnode == pnode->_parent->_left)
                {
                    pnode = pnode->_parent;
                    impl_rotate_right(impl, pnode);
                }

                pnode->_parent->_color = _RB_BLACK;
                pnode->_parent->_parent->_color = _RB_RED;
                impl_rotate_left(impl, pnode->_parent->_parent);
            }
        }
    }

    _RB_IMPL_ROOT(impl)->_color = _RB_BLACK;
    ++rb->size;

    return node;
}

static struct rb_node *
rb_insert_node(struct rb_tree *rb, struct rb_node *node, int left, int *out)
{
    struct _rb_impl *impl = _RB_IMPL(rb);

    struct rb_node *position = _RB_IMPL_HEAD(impl);
    struct rb_node *res = position->_parent;

    int addleft = 1;

    *out = 1;

    while (!res->_isnil)
    {
        position = res;

        if (left)
        {
            addleft = !impl_comp(impl, res, node);
        }
        else
        {
            addleft = impl_comp(impl, node, res);
        }

        res = addleft ? res->_left : res->_right;
    }

    if (impl->_multi)
    {
        return impl_node_insert(rb, node, position, addleft);
    }
    else
    {
        struct rb_node *pos = position;

        if (!addleft)
        {
        }
        else if (pos == _RB_IMPL_LMST(impl))
        {
            return impl_node_insert(rb, node, position, 1);
        }
        else
        {
            pos = node_prev(pos);
        }

        if (impl_comp(impl, pos, node))
        {
            return impl_node_insert(rb, node, position, addleft);
        }
        else
        {
            *out = 0;

            return pos;
        }
    }
}

static void
node_init(struct rb_node *node, struct rb_node *head)
{
    node->_parent = head;
    node->_left = head;
    node->_right = head;
    node->_color = _RB_RED;
    node->_isnil = 0;
}

static void
head_init(struct rb_node *head)
{
    head->_parent = head;
    head->_left = head;
    head->_right = head;
    head->_color = _RB_BLACK;
    head->_isnil = 1;
}

static void
impl_init(struct _rb_impl *impl, int multi, rb_compare_f comp, void *args)
{
    head_init(_RB_IMPL_HEAD(impl));

    impl->_multi = multi;
    impl->_comp = comp;
    impl->_args = args;
}

struct rb_node *
rb_lmst(const struct rb_tree *rb)
{
    return _RB_LMST(rb);
}

struct rb_node *
rb_rmst(const struct rb_tree *rb)
{
    return _RB_RMST(rb);
}

struct rb_node *
rb_head(const struct rb_tree *rb)
{
    return (struct rb_node *)_RB_HEAD(rb);
}

struct rb_node *
rb_prev(const struct rb_node *node)
{
    return node_prev(node);
}

struct rb_node *
rb_next(const struct rb_node *node)
{
    return node_next(node);
}

void
rb_init(struct rb_tree *rb, int multi, rb_compare_f comp, void *args)
{
    impl_init(_RB_IMPL(rb), multi, comp, args);

    rb->size = 0;
}

void
rb_clear(struct rb_tree *rb)
{
    head_init(_RB_HEAD(rb));

    rb->size = 0;
}

struct rb_node *
rb_insert(struct rb_tree *rb, struct rb_node *node, int *out)
{
#ifdef _RB_DEBUG
    assert(out && "not a legal, writable address");
#endif

    node_init(node, _RB_HEAD(rb));

    return rb_insert_node(rb, node, 0, out);
}

struct rb_pair
rb_eqrange(const struct rb_tree *rb, const struct rb_node *val)
{
    return impl_eqrange(_RB_IMPL(rb), val);
}

struct rb_node *
rb_lbnd(const struct rb_tree *rb, const struct rb_node *val)
{
    return impl_lbnd(_RB_IMPL(rb), val);
}

struct rb_node *
rb_ubnd(const struct rb_tree *rb, const struct rb_node *val)
{
    return impl_ubnd(_RB_IMPL(rb), val);
}

struct rb_node *
rb_find(const struct rb_tree *rb, const struct rb_node *val)
{
    struct rb_node *fr = rb_lbnd(rb, val);

    return fr == rb_head(rb) || rb_comp(rb, val, fr) ?
        rb_head(rb) : fr;
}

struct rb_node *
rb_erase(struct rb_tree *rb, struct rb_node *node)
{
    return rb_erase_node(rb, node);
}

struct rb_node *
rb_erase_range(struct rb_tree *rb,
    struct rb_node *begin, struct rb_node *end)
{
    if (begin == rb_lmst(rb) && end == rb_head(rb))
    {
        rb_clear(rb);

        return rb_lmst(rb);
    }
    else
    {
        while (begin != end)
        {
            begin = rb_erase(rb, begin);
        }

        return begin;
    }
}

size_t
rb_erase_rgcnt(struct rb_tree *rb,
    struct rb_node *begin, struct rb_node *end)
{
    size_t dist = 0;

    if (begin == rb_lmst(rb) && end == rb_head(rb))
    {
        dist = rb->size;

        rb_clear(rb);
    }
    else
    {
        while (begin != end)
        {
            begin = rb_erase(rb, begin);
            ++dist;
        }
    }

    return dist;
}

size_t
rb_erase_val(struct rb_tree *rb, const struct rb_node *val)
{
    struct rb_pair pr = rb_eqrange(rb, val);
    
    return rb_erase_rgcnt(rb, pr.first, pr.second);
}

size_t
rb_dist(const struct rb_tree *rb,
    const struct rb_node *begin, const struct rb_node *end)
{
    size_t dist = 0;

    if (begin == _RB_LMST(rb) && end == rb_head(rb))
    {
        dist = rb->size;
    }
    else
    {
        while (begin != end)
        {
            begin = node_next(begin);
            ++dist;
        }
    }

    return dist;
}

size_t
rb_vcnt(const struct rb_tree *rb, const struct rb_node *val)
{
    struct rb_pair pr = rb_eqrange(rb, val);

    return rb_dist(rb, pr.first, pr.second);
}
