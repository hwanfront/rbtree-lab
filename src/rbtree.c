#include "rbtree.h"

#include <stdio.h>
#include <stdlib.h>

typedef node_t *(*get_node)(node_t *);
typedef void (*set_node)(node_t *, node_t *);
typedef void (*rotate)(rbtree *, node_t *);

typedef enum { INSERT_CASE_1, INSERT_CASE_2, INSERT_CASE_3 } insert_fixup_case;
typedef enum { ERASE_CASE_1, ERASE_CASE_2, ERASE_CASE_3, ERASE_CASE_4 } erase_fixup_case;

typedef union {
  node_t *ptr;
  node_t **d_ptr;
} node_ptr;

typedef struct {
  get_node get_op_child;
  rotate rotation;
  rotate op_rotation;
} insert_fixup_ctx;

typedef struct {
  get_node get_child;
  get_node get_op_child;
  rotate rotation;
  rotate op_rotation;
} erase_fixup_ctx;

node_t *get_left(node_t *node) { return node->left; }
node_t *get_right(node_t *node) { return node->right; }
void set_left(node_t *parent, node_t *child) { parent->left = child; }
void set_right(node_t *parent, node_t *child) { parent->right = child; }

void print_rbtree(const rbtree *t, const node_t *node, int depth, char dir) {
  if (node == t->nil) {
    return;
  }

  printf("%*s%c key: %d, color: %c\n", depth * 4, "", dir, node->key, node->color == RBTREE_RED ? 'r' : 'b');

  print_rbtree(t, node->left, depth + 1, '<');
  print_rbtree(t, node->right, depth + 1, '>');
}

rbtree *create_rbtree(node_t *nil) {
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  p->nil = p->root = nil;
  return p;
}

node_t *create_node(rbtree *t, const key_t key) {
  node_t *node = (node_t *)malloc(sizeof(node_t));
  node->color = RBTREE_RED;
  node->key = key;
  node->left = node->right = node->parent = t->nil;
  return node;
}

node_t *create_nil() {
  node_t *nil = (node_t *)malloc(sizeof(node_t));
  nil->color = RBTREE_BLACK;
  nil->key = 0;
  nil->left = nil->right = nil->parent = NULL;
  return nil;
}

rbtree *new_rbtree(void) {
  return create_rbtree(create_nil());
}

void delete_all_nodes_recursion(rbtree *t, node_t *node) {
  if (node == t->nil) {
    return;
  }
  if (node->left != t->nil) {
    delete_all_nodes_recursion(t, node->left);
  }
  if (node->right != t->nil) {
    delete_all_nodes_recursion(t, node->right);
  }
  free(node);
}

void delete_rbtree(rbtree *t) {
  delete_all_nodes_recursion(t, t->root);
  free(t->nil);
  free(t);
}

void _rotate(rbtree *t, node_t *p, get_node get_child, get_node get_op_child, set_node set_child, set_node set_op_child) {
  node_t *c = get_op_child(p);
  set_op_child(p, get_child(c));
  if(get_child(c) != t->nil) {
    get_child(c)->parent = p;
  }
  c->parent = p->parent;
  if (c->parent == t->nil) {
    t->root = c;
  } else if (get_op_child(p->parent) == p) {
    set_op_child(p->parent, c);
  } else {
    set_child(p->parent, c);
  }
  set_child(c, p);
  p->parent = c;
}

void left_rotate(rbtree *t, node_t *p) {
  _rotate(t, p, get_left, get_right, set_left, set_right);
}

void right_rotate(rbtree *t, node_t *p) {
  _rotate(t, p, get_right, get_left, set_right, set_left);
}

void insert_fixup(insert_fixup_case c, rbtree *t, node_t **n, insert_fixup_ctx ctx) {
  switch(c) {
  case INSERT_CASE_3:
    *n = (*n)->parent;
    ctx.rotation(t, *n); 
    break;
  case INSERT_CASE_2:  
    (*n)->parent->color = RBTREE_BLACK;
    (*n)->parent->parent->color = RBTREE_RED;
    ctx.op_rotation(t, (*n)->parent->parent);
    break;
  case INSERT_CASE_1:
    (*n)->parent->parent->color = RBTREE_RED;
    (*n)->parent->color = RBTREE_BLACK;
    ctx.get_op_child((*n)->parent->parent)->color = RBTREE_BLACK;
    *n = (*n)->parent->parent;
    break;
  }
}

void left_insert_fixup(insert_fixup_case c, rbtree *t, node_t **n) {
  insert_fixup_ctx ctx = { get_right, left_rotate, right_rotate };
  insert_fixup(c, t, n, ctx);
}

void right_insert_fixup(insert_fixup_case c, rbtree *t, node_t **n) {
  insert_fixup_ctx ctx = { get_left, right_rotate, left_rotate };
  insert_fixup(c, t, n, ctx);
}

void rbtree_insert_fixup(rbtree *t, node_t *n) {
  while (n->parent->color == RBTREE_RED) {
    if (n->parent == n->parent->parent->left) { // 자식의 부모가 할배의 왼쪽
      if (n->parent->parent->right->color == RBTREE_BLACK) { // 삼촌이 검정
        if (n == n->parent->right) {
          left_insert_fixup(INSERT_CASE_3, t, &n);
        } 
        left_insert_fixup(INSERT_CASE_2, t, &n);
      } else {
        left_insert_fixup(INSERT_CASE_1, t, &n);
      }
    } else {
      if (n->parent->parent->left->color == RBTREE_BLACK) { // 삼촌이 검정
        if (n == n->parent->left) {
          right_insert_fixup(INSERT_CASE_3, t, &n);
        }
        right_insert_fixup(INSERT_CASE_2, t, &n);
      } else {
        right_insert_fixup(INSERT_CASE_1, t, &n);
      }
    }
  }
  t->root->color = RBTREE_BLACK;
}

node_t *rbtree_find_insert_parent_node(rbtree *t, const key_t key) {
  node_t *cur = t->root;
  node_t *prev = t->nil;

  while (cur != t->nil) {
    prev = cur;
    if (key < cur->key) {
      cur = cur->left;
    } else {
      cur = cur->right;
    }
  }
  return prev;
}

void rbtree_insert_node(rbtree *t, const key_t key, node_t *node, node_t *parent) {
  node->parent = parent;
  if (key < node->parent->key) {
    node->parent->left = node;
  } else {
    node->parent->right = node;
  }
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  node_t *new_node = create_node(t, key);

  if (t->root == t->nil) {
    t->root = new_node; // #2 위반
    t->root->color = RBTREE_BLACK;
    return t->root;
  }

  rbtree_insert_node(t, key, new_node, rbtree_find_insert_parent_node(t, key));
  rbtree_insert_fixup(t, new_node);
  return t->root;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  node_t *cur = t->root;
  while (cur != t->nil) {
    if (cur->key == key) {
      return cur;
    }
    if (key < cur->key) {
      cur = cur->left;
    } else {
      cur = cur->right;
    }
  }
  return NULL;
}

node_t *rbtree_subtree_min(const rbtree *t, node_t *subtree) {
  node_t *node = subtree;
  if (node == t->nil) {
    return t->nil;
  }
  while (node->left != t->nil) {
    node = node->left;
  }
  return node;
}

node_t *rbtree_min(const rbtree *t) {
  return rbtree_subtree_min(t, t->root);
}

node_t *rbtree_subtree_max(const rbtree *t, node_t *subtree) {
  node_t *node = subtree;
  if (node == t->nil) {
    return t->nil;
  }
  while (node->right != t->nil) {
    node = node->right;
  }
  return node;
}

node_t *rbtree_max(const rbtree *t) {
  return rbtree_subtree_max(t, t->root);
}

void erase_fixup(erase_fixup_case c, rbtree *t, node_t **ex_node, node_t **u, erase_fixup_ctx ctx) {
  switch(c) {
  case ERASE_CASE_1:
    (*u)->color = RBTREE_BLACK;
    (*ex_node)->parent->color = RBTREE_RED;
    ctx.rotation(t, (*ex_node)->parent);
    *u = ctx.get_op_child((*ex_node)->parent);
    break;
  case ERASE_CASE_2:
    (*u)->color = RBTREE_RED;
    *ex_node = (*ex_node)->parent;
    break;
  case ERASE_CASE_3:
    ctx.get_child(*u)->color = RBTREE_BLACK;
    (*u)->color = RBTREE_RED;
    ctx.op_rotation(t, *u);
    *u = ctx.get_op_child((*ex_node)->parent);
    break;
  case ERASE_CASE_4:
    (*u)->color = (*ex_node)->parent->color;
    ctx.get_op_child(*u)->color = RBTREE_BLACK;
    (*ex_node)->parent->color = RBTREE_BLACK;
    ctx.rotation(t, (*ex_node)->parent);
    *ex_node = t->root;
    break;
  }
}

void left_erase_fixup(erase_fixup_case c, rbtree *t, node_t **ex_node, node_t **u) {
  erase_fixup_ctx ctx = { get_left, get_right, left_rotate, right_rotate };
  erase_fixup(c, t, ex_node, u, ctx);
}

void right_erase_fixup(erase_fixup_case c, rbtree *t, node_t **ex_node, node_t **u) {
  erase_fixup_ctx ctx = { get_right, get_left, right_rotate, left_rotate };
  erase_fixup(c, t, ex_node, u, ctx);
}

void rbtree_erase_fixup(rbtree *t, node_t *ex_node) {
  node_t *u;
  while (ex_node != t->root && ex_node->color == RBTREE_BLACK) {
    if (ex_node->parent->left == ex_node) {
      u = ex_node->parent->right;
      if (u->color == RBTREE_RED) {
        left_erase_fixup(ERASE_CASE_1, t, &ex_node, &u);
      } 
      if (u->left->color == RBTREE_BLACK && u->right->color == RBTREE_BLACK) {
        left_erase_fixup(ERASE_CASE_2, t, &ex_node, &u);
      } else {
        if (u->right->color == RBTREE_BLACK) {
          left_erase_fixup(ERASE_CASE_3, t, &ex_node, &u);
        }
        left_erase_fixup(ERASE_CASE_4, t, &ex_node, &u);
      }
    } else {
      u = ex_node->parent->left;
      if (u->color == RBTREE_RED) {
        right_erase_fixup(ERASE_CASE_1, t, &ex_node, &u);
      } 
      if (u->right->color == RBTREE_BLACK && u->left->color == RBTREE_BLACK) {
        right_erase_fixup(ERASE_CASE_2, t, &ex_node, &u);
      } else {
        if (u->left->color == RBTREE_BLACK) {
          right_erase_fixup(ERASE_CASE_3, t, &ex_node, &u);
        }
        right_erase_fixup(ERASE_CASE_4, t, &ex_node, &u);
      }
    }
  }
  ex_node->color = RBTREE_BLACK;
}

void transplant(rbtree *t, node_t *a, node_t *b) {
  b->parent = a->parent;
  if (a->parent == t->nil) {
    t->root = b;
    return;
  } 
  if (a->parent->left == a) {
    a->parent->left = b;
    return;
  } 
  a->parent->right = b;
}

int rbtree_erase(rbtree *t, node_t *node) {
  node_t *ex_node;
  int rm_color = node->color; // 삭제되는 노드 자식이 0,1 개 -> 삭제되는 노드 색깔
  if (node->left == t->nil) {
    ex_node = node->right;
    transplant(t, node, node->right);
  } else if (node->right == t->nil) {
    ex_node = node->left;
    transplant(t, node, node->left);
  } else {
    node_t *sc = rbtree_subtree_min(t, node->right);
    rm_color = sc->color; // 삭제되는 노드 자식이 2 개 -> successor 노드 색깔
    ex_node = sc->right;
    if (sc == node->right) { // successor 가 삭제 노드 바로 오른쪽
      ex_node->parent = sc;
    } else {
      transplant(t, sc, sc->right);
      sc->right = node->right;
      sc->right->parent = sc;
    }
    transplant(t, node, sc);
    sc->left = node->left;
    sc->left->parent = sc;
    sc->color = node->color; 
  }

  free(node);

  if (rm_color == RBTREE_BLACK) {
    rbtree_erase_fixup(t, ex_node);
  }

  return 0;
}

size_t set_rbtree_node_to_array(const rbtree *t, node_t *node, key_t *arr, size_t idx, const size_t n) {
  if (idx >= n) {
    return idx;
  }
  if (node == t->nil) {
    return idx;
  }
  if (node->left != t->nil) {
    idx = set_rbtree_node_to_array(t, node->left, arr, idx, n);
    if (idx >= n) {
      return idx;
    }
  }
  arr[idx++] = node->key;
  if (node->right != t->nil) {
    idx = set_rbtree_node_to_array(t, node->right, arr, idx, n);
    if (idx >= n) {
      return idx;
    }
  }
  return idx;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  set_rbtree_node_to_array(t, t->root, arr, 0, n);
  return 0;
}
