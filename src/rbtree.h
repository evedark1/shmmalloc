#ifndef _RBTREE_H_
#define _RBTREE_H_

#include <stdint.h>
#include <string.h>

// user should define follow marco
#ifndef RB_POINTER
#define RB_POINTER uint16_t
#define RB_KEY uint16_t
#define RB_NULL 0
extern struct rbtree_node *get_node(RB_POINTER p);
#define RB_GET_NODE(p) get_node(p)
extern int compare(RB_KEY a, RB_KEY b);
#define RB_COMPARE(a, b) compare(a, b)
#endif

#define RB_RED 0
#define RB_BLACK 1
 
struct rbtree_node{
	RB_POINTER s;
	RB_POINTER p;
	RB_POINTER left;
	RB_POINTER right;
    RB_KEY v;
	uint8_t color;
};
 
struct rbtree{
	RB_POINTER root;
	uint32_t size;
};
 
static inline struct rbtree_node *rbtree_check_node(RB_POINTER p)
{
	if(p == RB_NULL)
		return NULL;
	return RB_GET_NODE(p);
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
	p = rbtree_check_node(node->p);
 
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
	p = rbtree_check_node(node->p);

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
	while((p = rbtree_check_node(node->p)) && p->color == RB_RED) {
		struct rbtree_node *pp = RB_GET_NODE(p->p);
		if(p->s == pp->left) {
			struct rbtree_node *uncle = rbtree_check_node(pp->right);
			if(uncle && uncle->color == RB_RED) {
				uncle->color = RB_BLACK;
				p->color = RB_BLACK;
				pp->color = RB_RED;
				node = pp;
			} else {
				if(node->s == p->right) {
					rbtree_left_rotate(root, p);
					struct rbtree_node *t = node;
					node = p;
					p = t;
				}
				p->color = RB_BLACK;
				pp->color = RB_RED;
				rbtree_right_rotate(root, pp);
			}
		} else {
			struct rbtree_node *uncle = rbtree_check_node(pp->left);
			if(uncle && uncle->color == RB_RED) {
				uncle->color = RB_BLACK;
				p->color = RB_BLACK;
				pp->color = RB_RED;
				node = pp;
			} else {
				if(node->s == p->left) {
					rbtree_right_rotate(root, p);
					struct rbtree_node *t = node;
					node = p;
					p = t;
				}
				p->color = RB_BLACK;
				pp->color = RB_RED;
				rbtree_left_rotate(root, pp);
			}
		}
	}
	if(node->p == RB_NULL)
		node->color = RB_BLACK;
}

static void rbtree_fix_delete(struct rbtree *root, struct rbtree_node *node)
{
	while(node->p != RB_NULL && node->color == RB_BLACK) {
		struct rbtree_node *p = RB_GET_NODE(node->p);
		if(p->left == node->s) {
			struct rbtree_node *w = RB_GET_NODE(p->right);
			if(w->color == RB_RED) {
				w->color = RB_BLACK;
				p->color = RB_RED;
				rbtree_left_rotate(root, p);
				w = RB_GET_NODE(p->right);
			}

			struct rbtree_node *wl = rbtree_check_node(w->left);
			struct rbtree_node *wr = rbtree_check_node(w->right);
			if((wl == NULL || wl->color == RB_BLACK) && (wr == NULL || wr->color == RB_BLACK)) {
				w->color = RB_RED;
				node = p;
			} else {
				if(wr == NULL || wr->color == RB_BLACK) {
					wl->color = RB_BLACK;
					w->color = RB_RED;
					rbtree_right_rotate(root, w);
					w = RB_GET_NODE(p->right);
					wr = RB_GET_NODE(w->right);
				}
				w->color = p->color;
				p->color = RB_BLACK;
				wr->color = RB_BLACK;
				rbtree_left_rotate(root, p);
				break;
			}
		} else {
			struct rbtree_node *w = RB_GET_NODE(p->left);
			if(w->color == RB_RED){
				w->color = RB_BLACK;
				p->color = RB_RED;
				rbtree_right_rotate(root, p);
				w = RB_GET_NODE(p->left);
			}

			struct rbtree_node *wl = rbtree_check_node(w->left);
			struct rbtree_node *wr = rbtree_check_node(w->right);
			if((wl == NULL || wl->color == RB_BLACK) && (wr == NULL || wr->color == RB_BLACK)) {
				w->color = RB_RED;
				node = p;
			} else {
				if(wl == NULL || wl->color == RB_BLACK) {
					wr->color = RB_BLACK;
					w->color = RB_RED;
					rbtree_left_rotate(root, w);
					w = RB_GET_NODE(p->left);
					wl = RB_GET_NODE(w->left);
				}
				w->color = p->color;
				p->color = RB_BLACK;
				wl->color = RB_BLACK;
				rbtree_right_rotate(root, p);
				break;
			}
		}
	}
	node->color = RB_BLACK;
}

// rbtree api
static void rbtree_node_init(struct rbtree_node *node)
{
	node->color = RB_RED;
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
    struct rbtree_node *r = NULL;
    if(node->left != RB_NULL) {
        node = RB_GET_NODE(node->left);
        while(node->right != RB_NULL)
            node = RB_GET_NODE(node->right);
        r = node;
    } else {
        while(node->p != RB_NULL) {
            struct rbtree_node *p = RB_GET_NODE(node->p);
            if(node->s == p->left)
                node = p;
            else {
                r = p;
                break;
            }
        }
    }
    return r;
}

static struct rbtree_node *rbtree_find(struct rbtree *root, RB_KEY val)
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

static struct rbtree_node *rbtree_lower(struct rbtree *root, RB_KEY val)
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
	assert(RB_GET_NODE(new_node->s) == new_node);
	rbtree_node_init(new_node);
    root->size++;

	if(root->root == RB_NULL) {
		new_node->color = RB_BLACK;
		root->root = new_node->s;
        return;
    }

	RB_POINTER pnode = root->root;
	struct rbtree_node *node = NULL;
	while(pnode != RB_NULL) {
		node = RB_GET_NODE(pnode);
		if(RB_COMPARE(node->v, new_node->v) < 0) {
			pnode = node->right;
		}else{
			pnode = node->left;
		}
	}

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

	struct rbtree_node enode;
	rbtree_node_init(&enode);
	enode.color = RB_BLACK;
	enode.s = RB_NULL;

	// find replace node y
	struct rbtree_node *x, *y;
	RB_POINTER px;
	if(node->left == RB_NULL) {
		px = node->right;
		y = node;
	} else if (node->right == RB_NULL) {
		px = node->left;
		y = node;
	} else {
		y = rbtree_min_node(RB_GET_NODE(node->right));
		px = y->right;
	}
	x = (px != RB_NULL) ? RB_GET_NODE(px) : &enode;

	// delete replace node y
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
	uint8_t color = y->color;
	
	if(y != node) {
		struct rbtree_node *t = NULL;
		// fix if y->p == node, change x->p
		if(y->p == node->s)
			x->p = y->s;
		// replace color
		y->color = node->color;
		// replace parent
		if(node->p == RB_NULL) {
			root->root = y->s;
		} else {
			t = RB_GET_NODE(node->p);
			if(t->left == node->s)
				t->left = y->s;
			else
				t->right = y->s;
		}
		y->p = node->p;
		node->p = RB_NULL;
		// replace left clild
		if(node->left != RB_NULL) {
			t = RB_GET_NODE(node->left);
			t->p = y->s;
		}
		y->left = node->left;
		node->left = RB_NULL;
		// replace right clild
		if(node->right != RB_NULL) {
			t = RB_GET_NODE(node->right);
			t->p = y->s;
		}
		y->right = node->right;
		node->right = RB_NULL;
	}

	if(color == RB_BLACK)
		rbtree_fix_delete(root, x);
}

#endif
