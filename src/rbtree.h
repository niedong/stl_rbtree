/*
 * Copyright (c) 2020 niedong
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __RBTREE__
#define __RBTREE__

#include <stddef.h>

/*
 * The _RB_DEBUG flag will enable extra operation checks, while
 * _RB_RELEASE flag will disable them.
 */
#if defined _DEBUG && !defined _RB_RELEASE && !defined _RB_DEBUG
#define _RB_DEBUG
#endif

/*
 * A member that starts with an underscore '_'(e.g. _parent) is
 * considered as a protected member. You should not use them
 * directly, since they are handled during internal operation.
 * 
 * Undefined result may occur if you modify them externally.
 */

struct rb_node
{
    struct rb_node *_parent;  // ptr to parent
    struct rb_node *_left;    // ptr to left child
    struct rb_node *_right;   // ptr to right child
    char            _color;   // the color
    char            _isnil;   // there are no NULL ptr, only nil node
};

#if !defined RB_CONV
// the container access macro
#define RB_CONV(type, ptr, name) \
    ((type *)((char *)&(ptr)->_left - offsetof(type, name._left)))
#endif

/*
 * To make rbtree capable of storing multiple key-equivalent values,
 * the compare function must return a strictly less order of two nodes.
 * 
 * Extra argument can be provided if needed.
 */
typedef int(*rb_compare_f)(const struct rb_node *, const struct rb_node *, void *);

struct _rb_impl
{
    struct rb_node _head;  // head node
    rb_compare_f   _comp;  // user's compare function
    void *         _args;  // user's extra argument
    int            _multi; // multi or not
};

struct rb_pair
{
    struct rb_node *first, *second;
};

struct rb_tree
{
    struct _rb_impl _impl;
    size_t          size;  // public member, size of the tree
};

// red node
#define _RB_RED   0
// black node
#define _RB_BLACK 1

// ****** The following macro are used internally ******

#define _RB_IMPL_HEAD(impl) (&(impl)->_head)
#define _RB_IMPL_ROOT(impl) (_RB_IMPL_HEAD(impl)->_parent)
#define _RB_IMPL_LMST(impl) (_RB_IMPL_HEAD(impl)->_left)
#define _RB_IMPL_RMST(impl) (_RB_IMPL_HEAD(impl)->_right)
#define _RB_IMPL(p)         (&(p)->_impl)
#define _RB_HEAD(p)         _RB_IMPL_HEAD(_RB_IMPL(p))
#define _RB_ROOT(p)         _RB_IMPL_ROOT(_RB_IMPL(p))
#define _RB_LMST(p)         _RB_IMPL_LMST(_RB_IMPL(p))
#define _RB_RMST(p)         _RB_IMPL_RMST(_RB_IMPL(p))

#define _RB_IMPL_HEAD_INIT(head) \
    { head,head,head,_RB_BLACK,1 }

#define _RB_IMPL_INIT(impl, multi, comp, args) \
    { _RB_IMPL_HEAD_INIT(_RB_IMPL_HEAD(impl)),comp,args,multi }

// ****** End of internal macro ******

/*
 * rbtree init macro. This supports direct initialization
 * for on-stack, static or global rbtree variable.
 * 
 * e.g.
 * 
 * struct rb_tree myTree = RB_INIT(&myTree, ...);
 */
#define RB_INIT(p, multi, comp, args) \
    { _RB_IMPL_INIT(_RB_IMPL(p), multi, comp, args),0 }

/*
 * For rb_tree traversal, you can write:
 * 
 * for (struct rb_node *it = rb_lmst(tr); it != rb_head(tr); it = rb_next(it))
 * {
 *     Do something with 'it'...
 * }
 * 
 * Or in a reverse order:
 * 
 * for (struct rb_node *it = rb_rmst(tr); it != rb_head(tr); it = rb_prev(it))
 * {
 *     Do something with 'it'...
 * }
 * 
 * Notice that although rb_prev/rb_next runs O(logn) on average, but for rb_tree
 * traversal, access on each node runs amortized O(1). Thus the whole traversal
 * runs in O(n).
 */

#ifdef __cplusplus
extern "C" {
#endif

struct rb_node *rb_lmst(const struct rb_tree *rb);
struct rb_node *rb_rmst(const struct rb_tree *rb);
struct rb_node *rb_head(const struct rb_tree *rb);
struct rb_node *rb_prev(const struct rb_node *node);
struct rb_node *rb_next(const struct rb_node *node);

void rb_init(struct rb_tree *rb, int multi, rb_compare_f comp, void *args);

void rb_clear(struct rb_tree *rb);

struct rb_pair rb_eqrange(const struct rb_tree *rb, const struct rb_node *val);

struct rb_node *rb_insert(struct rb_tree *rb, struct rb_node *node, int *out);
struct rb_node *rb_erase(struct rb_tree *rb, struct rb_node *node);
struct rb_node *rb_erase_range(struct rb_tree *rb, struct rb_node *begin, struct rb_node *end);

size_t rb_erase_val(struct rb_tree *rb, const struct rb_node *val);

size_t rb_dist(const struct rb_tree *rb, const struct rb_node *begin, const struct rb_node *end);
size_t rb_vcnt(const struct rb_tree *rb, const struct rb_node *val);

struct rb_node *rb_find(const struct rb_tree *rb, const struct rb_node *val);

struct rb_node *rb_lbnd(const struct rb_tree *rb, const struct rb_node *val);
struct rb_node *rb_ubnd(const struct rb_tree *rb, const struct rb_node *val);

#ifdef __cplusplus
}
#endif

#endif
