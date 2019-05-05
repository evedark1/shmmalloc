#ifndef _RBTREE_H_
#define _RBTREE_H_

#include <stdint.h>
#include <string.h>

// user should define follow marco
#ifndef RB_POINTER
#define RB_POINTER uint16_t
#define RB_VALUE uint16_t
#define RB_NULL 0
extern struct rbtree_node *get_node(RB_POINTER p);
#define RB_GET_NODE(p) get_node(p)
extern int compare(RB_VALUE a, RB_VALUE b);
#define RB_COMPARE(a, b) compare(a, b)
#endif

#define RED 0
#define BLACK 1
 
struct rbtree_node{
	RB_POINTER s;
	RB_VALUE v;
	RB_POINTER p;
	RB_POINTER left;
	RB_POINTER right;
	uint8_t color;
};
 
struct rbtree{
	RB_POINTER root;
	uint32_t size;
};
 
static struct rbtree_node *rbtree_get_prev_node(struct rbtree *root, struct rbtree_node *other)
{
	RB_POINTER node = root->root;
	struct rbtree_node *prev = NULL;
	while(node != RB_NULL) {
		struct rbtree_node *pnode = RB_GET_NODE(node);
		prev = pnode;
		if(RB_COMPARE(pnode->v, other->v) < 0) {
			node = pnode->right;
		}else{
			node = pnode->left;
		}
	}
	return prev;
}

static struct rbtree_node *rbtree_min_node(struct rbtree_node *node)
{
	while(node->left != RB_NULL)
		node = RB_GET_NODE(node->left);
	return node;
}

static struct rbtree_node *rbtree_max_node(struct rbtree_node *node)
{
	while(node->right != RB_NULL)
		node = RB_GET_NODE(node->right);
	return node;
}

static void rbtree_left_rotate(struct rbtree *root, struct rbtree_node *node)
{
	struct rbtree_node *x, *p;
	x = RB_GET_NODE(node->right);
	p = RB_GET_NODE(node->p);
 
	node->right = x->left;
	if(node->right != RB_NULL) {
		struct rbtree_node *y = RB_GET_NODE(node->right);
		y->p = node->s;
	}

	node->p = x->s;
	x->left = node->s;
	if(p == NULL) {
		root->root = x->s;
		x->p = RB_NULL;
	} else {
		if(p->left == node->s) {
			p->left = x->s;
		} else {
			p->right = x->s;
		}
		x->p = p->s;
	}
}

static void rbtree_right_rotate(struct rbtree *root, struct rbtree_node *node)
{
	struct rbtree_node *x, *p;
	x = RB_GET_NODE(node->left);
	p = RB_GET_NODE(node->p);
 
	node->left = x->right;
	if(node->left != RB_NULL) {
		struct rbtree_node *y = RB_GET_NODE(node->left);
		y->p = node->s;
	}
 
	node->p = x->s;
	x->right = node->s;
	if(p == NULL) {
		root->root = x->s;
		x->p = RB_NULL;
	} else {
		if(p->left == node->s) {
			p->left = x->s;
		} else {
			p->right = x->s;
		}
		x->p = p->s;
	}
}

static void rbtree_fix_insert(struct rbtree *root, struct rbtree_node *node)
{
	struct rbtree_node *p;
	while((p == RB_GET_NODE(node->p)) && p->color == RED) {
		struct rbtree_node *pp = RB_GET_NODE(p->p);
		if(p->s == pp->left) {
			struct rbtree_node *uncle = RB_GET_NODE(pp->right);
			if(uncle && uncle->color == RED) {
				uncle->color = BLACK;
				p->color = BLACK;
				pp->color = RED;
				node = pp;
			} else {
				if(node->s == p->right) {
					rbtree_left_rotate(root, p);
					node = p;
					p = RB_GET_NODE(node->p);
				}
				rbtree_right_rotate(root, pp);
				uint8_t c = p->color;
				p->color = pp->color;
				pp->color = c;
				node = p;
			}
		} else {
			struct rbtree_node *uncle = RB_GET_NODE(pp->left);
			if(uncle && uncle->color == RED) {
				uncle->color = BLACK;
				p->color = BLACK;
				pp->color = RED;
				node = pp;
			} else {
				if(node->s == p->left) {
					rbtree_right_rotate(root, p);
					node = p;
					p = RB_GET_NODE(node->p);
				}
				rbtree_left_rotate(root, pp);
				uint8_t c = p->color;
				p->color = pp->color;
				pp->color = c;
				node = p;
			}
		}
	}
	if(node->p == RB_NULL)
		node->color = BLACK;
}

static void rbtree_fix_delete(struct rbtree *root, struct rbtree_node *node)
{
	while(node->p != RB_NULL && node->color == BLACK) {
		struct rbtree_node *p = RB_GET_NODE(node->p);
		if(p->left == node->s) {
			struct rbtree_node *w = RB_GET_NODE(p->right);
			if(w->color == RED) {
				w->color = BLACK;
				p->color = RED;
				rbtree_left_rotate(root, p);
				w = RB_GET_NODE(p->right);
			}

			struct rbtree_node *wl = RB_GET_NODE(w->left);
			struct rbtree_node *wr = RB_GET_NODE(w->right);
			if(wl->color == BLACK && wr->color == BLACK) {
				w->color = RED;
				node = p;
			} else {
				if(wr->color == BLACK) {
					wl->color = BLACK;
					w->color = RED;
					rbtree_right_rotate(root, w);
					w = RB_GET_NODE(p->right);
					wr = RB_GET_NODE(w->right);
				}
				w->color = p->color;
				p->color = BLACK;
				wr->color = BLACK;
				rbtree_left_rotate(root, p);
				break;
			}
		} else {
			struct rbtree_node *w = RB_GET_NODE(p->left);
			if(w->color == RED){
				w->color = BLACK;
				p->color = RED;
				rbtree_right_rotate(root, p);
				w = RB_GET_NODE(p->left);
			}

			struct rbtree_node *wl = RB_GET_NODE(w->left);
			struct rbtree_node *wr = RB_GET_NODE(w->right);
			if(wl->color == BLACK && wr->color == BLACK) {
				w->color = RED;
				node = p;
			}else{
				if(wl->color == BLACK){
					wr->color = BLACK;
					w->color = RED;
					rbtree_left_rotate(root, w);
					w = RB_GET_NODE(p->left);
					wl = RB_GET_NODE(w->left);
				}
				w->color = p->color;
				p->color = BLACK;
				wl->color = BLACK;
				rbtree_right_rotate(root, p);
				break;
			}
		}
	}
	node->color = BLACK;
}

// rbtree api
static void rbtree_node_init(struct rbtree_node *node)
{
	node->color = RED;
	node->p = RB_NULL;
	node->left = RB_NULL;
	node->right = RB_NULL;
}

static void rbtree_init(struct rbtree *root)
{
	root->root = RB_NULL;
	root->size = 0;
}

static struct rbtree_node *rbtree_min(struct rbtree *root)
{
	struct rbtree_node *node = NULL;
	if(root->root != RB_NULL) {
		node = rbtree_min_node(RB_GET_NODE(root->root));
	}
	return node;
}

static struct rbtree_node *rbtree_max(struct rbtree *root)
{
	struct rbtree_node *node = NULL;
	if(root->root != RB_NULL) {
		node = rbtree_max_node(RB_GET_NODE(root->root));
	}
	return node;
}
 
static struct rbtree_node *rbtree_next(struct rbtree *root, struct rbtree_node *node)
{
    struct rbtree_node *r = NULL;
    if(node->right != RB_NULL) {
        node = RB_GET_NODE(node->right);
        while(node->left != RB_NULL)
            node = RB_GET_NODE(node->left);
        r = node;
    } else {
        while(node->p != RB_NULL) {
            struct rbtree_node *p = RB_GET_NODE(node->p);
            if(node->s == p->right)
                node = p;
            else {
                r = p;
                break;
            }
        }
    }
    return r;
}

static struct rbtree_node *rbtree_prev(struct rbtree *root, struct rbtree_node *node)
{
	// TODO: finish this
}

static struct rbtree_node *rbtree_find(struct rbtree *root, RB_VALUE val)
{
	struct rbtree_node *r = NULL;
	RB_POINTER node = root->root;
	while(node != RB_NULL) {
		struct rbtree_node *pnode = RB_GET_NODE(node);
		int diff = RB_COMPARE(pnode->v, val);
		if(diff < 0) {
			node = pnode->right;
		} else if(diff > 0) {
			node = pnode->left;
		} else { // diff == 0
			r = pnode;
			break;
		}
	}
	return r;
}

static struct rbtree_node *rbtree_lower(struct rbtree *root, RB_VALUE val)
{
	RB_POINTER node = root->root;
	struct rbtree_node *prev = NULL;
	uint32_t prev_bigger = 0;

	while(node != RB_NULL) {
		struct rbtree_node *pnode = RB_GET_NODE(node);
		if(RB_COMPARE(pnode->v, val) < 0) {
			if(prev_bigger)
				break;
			node = pnode->right;
		}else{
			node = pnode->left;
			prev_bigger = 1;
		}
		prev = pnode;
	}
	return prev_bigger ? prev : NULL;
}

static void rbtree_insert(struct rbtree *root, struct rbtree_node *new_node)
{ 
	rbtree_node_init(new_node);
    root->size++;

	if(root->root == RB_NULL) {
		new_node->color = BLACK;
		root->root = new_node->s;
        return;
    }

	struct rbtree_node *node = rbtree_get_prev_node(root, new_node);
	if(RB_COMPARE(node->v, new_node->v) < 0) {
		node->right = new_node->s;
	} else {
		node->left = new_node->s;
	}
	new_node->p = node->s;
	rbtree_fix_insert(root, new_node);
}

static void rbtree_delete(struct rbtree *root, struct rbtree_node *node)
{
    root->size--;

	struct rbtree_node *x, *y;
	if(node->left == RB_NULL) {
		x = RB_GET_NODE(node->right);
		y = node;
	} else if (node->right == RB_NULL) {
		x = RB_GET_NODE(node->left);
		y = node;
	} else {
		y = rbtree_min_node(RB_GET_NODE(node->right));
		if(y->left != RB_NULL)
            x = RB_GET_NODE(y->left);
		else
            x = RB_GET_NODE(y->right);
	}

	x->p = y->p;
	if(y->p == RB_NULL) {
        root->root = x->s;
	} else {
		struct rbtree_node *p = RB_GET_NODE(y->p);
		if(y->s == p->left) {
			p->left = x->s;
		} else {
			p->right = x->s;
		}
	}
	
	if(y != node) {
		node->v = y->v;
	}

	if(y->color == BLACK)
		rbtree_fix_delete(root, x);
}

#endif
