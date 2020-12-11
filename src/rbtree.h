#ifndef __RBTREE__
#define __RBTREE__

#include <stddef.h>

#if defined _DEBUG && !defined _RB_RELEASE
#define _RB_DEBUG
#endif

struct rb_node
{
    struct rb_node *_parent;
    struct rb_node *_left;
    struct rb_node *_right;
    char            _color;
    char            _isnil;
};

#if !defined RB_CONV
#define RB_CONV(type, ptr, name) \
    ((type *)((char *)&(ptr)->_left - offsetof(type, name._left)))
#endif

typedef int(*rb_compare_f)(const struct rb_node *, const struct rb_node *, void *);

struct _rb_impl
{
    struct rb_node _head;
    rb_compare_f   _comp;
    void *         _args;
    int            _multi;
};

struct rb_pair
{
    struct rb_node *first, *second;
};

struct rb_tree
{
    struct _rb_impl _impl;
    size_t          size;
};

#define _RB_RED   0
#define _RB_BLACK 1

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

#define RB_INIT(p, multi, comp, args) \
    { _RB_IMPL_INIT(_RB_IMPL(p), multi, comp, args),0 }

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
