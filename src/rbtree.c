#include "rbtree.h"

#include <stdio.h>
#include <stdlib.h>


void print_rbtree(const rbtree *t, const node_t *node, int depth, char dir) {
  if (node == t->nil) {
    return;
  }

  printf("%*s%c key: %d, color: %c\n", depth * 4, "", dir, node->key, node->color == RBTREE_RED ? 'r' : 'b');

  print_rbtree(t, node->left, depth + 1, '<');
  print_rbtree(t, node->right, depth + 1, '>');
}

rbtree *new_rbtree(void) {
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  node_t *nil = (node_t *)malloc(sizeof(node_t));
  nil->color = RBTREE_BLACK;
  nil->key = 0;
  nil->left = NULL;
  nil->right = NULL;
  nil->parent = NULL;
  p->nil = nil;
  p->root = nil;
  return p;
}

void delete_node_recursion(rbtree *t, node_t *node) {
  if (node == t->nil) {
    return;
  }
  if (node->left != t->nil) {
    delete_node_recursion(t, node->left);
  }
  if (node->right != t->nil) {
    delete_node_recursion(t, node->right);
  }
  free(node);
}

void delete_rbtree(rbtree *t) {
  delete_node_recursion(t, t->root);
  free(t->nil);
  free(t);
}

void left_rotate(rbtree *t, node_t *p) {
  node_t *c = p->right;
  p->right = c->left;
  if (c->left != t->nil) {
    c->left->parent = p;
  }
  c->parent = p->parent;
  if (c->parent == t->nil) {
    t->root = c;
  } else if (c->parent->right == p) {
    c->parent->right = c;
  } else {
    c->parent->left = c;
  }
  p->parent = c;
  c->left = p;
}

void right_rotate(rbtree *t, node_t *p) {
  node_t *c = p->left;
  p->left = c->right;
  if (c->right != t->nil) {
    c->right->parent = p;
  }
  c->parent = p->parent;
  if (c->parent == t->nil) {
    t->root = c;
  } else if (c->parent->left == p) {
    c->parent->left = c;
  } else {
    c->parent->right = c;
  }
  p->parent = c;
  c->right = p;
}

void rbtree_insert_fixup(rbtree *t, node_t *n) {
  while (n->parent->color == RBTREE_RED) {
    if (n->parent == n->parent->parent->left) { // 자식의 부모가 할배의 왼쪽
      if (n->parent->parent->right->color == RBTREE_BLACK) { // 삼촌이 검정
        if (n == n->parent->right) {
          n = n->parent;
          left_rotate(t, n); 
        } 
        n->parent->color = RBTREE_BLACK;
        n->parent->parent->color = RBTREE_RED;
        right_rotate(t, n->parent->parent);
      } else {
        n->parent->parent->color = RBTREE_RED;
        n->parent->color = RBTREE_BLACK;
        n->parent->parent->right->color = RBTREE_BLACK;
        n = n->parent->parent;
      }
    } else {
      if (n->parent->parent->left->color == RBTREE_BLACK) { // 삼촌이 검정
        if (n == n->parent->left) {
          n = n->parent;
          right_rotate(t, n); 
        } 
        n->parent->color = RBTREE_BLACK;
        n->parent->parent->color = RBTREE_RED;
        left_rotate(t, n->parent->parent);
      } else {
        n->parent->parent->color = RBTREE_RED;
        n->parent->color = RBTREE_BLACK;
        n->parent->parent->left->color = RBTREE_BLACK;
        n = n->parent->parent;
      }
    }
  }
  t->root->color = RBTREE_BLACK;
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  node_t *new_node = (node_t *)malloc(sizeof(node_t));
  new_node->color = RBTREE_RED;
  new_node->key = key;
  new_node->left = t->nil;
  new_node->right = t->nil;
  new_node->parent = t->nil;

  if (t->root == t->nil) {
    t->root = new_node; // #2 위반
    t->root->color = RBTREE_BLACK;
    return t->root;
  }

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
  new_node->parent = prev;
  if (key < new_node->parent->key) {
    new_node->parent->left = new_node;
  } else {
    new_node->parent->right = new_node;
  }

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

void rbtree_erase_fixup(rbtree *t, node_t *ex_node) {
  node_t *u;
  while (ex_node != t->root && ex_node->color == RBTREE_BLACK) {
    if (ex_node->parent->left == ex_node) {
      u = ex_node->parent->right;
      if (u->color == RBTREE_RED) {
        u->color = RBTREE_BLACK;
        ex_node->parent->color = RBTREE_RED;
        left_rotate(t, ex_node->parent);
        u = ex_node->parent->right;
      } 
      if (u->left->color == RBTREE_BLACK && u->right->color == RBTREE_BLACK) {
        u->color = RBTREE_RED;
        ex_node = ex_node->parent;
      } else {
        if (u->right->color == RBTREE_BLACK) {
          u->left->color = RBTREE_BLACK;
          u->color = RBTREE_RED;
          right_rotate(t, u);
          u = ex_node->parent->right;
        }
        u->color = ex_node->parent->color;
        u->right->color = RBTREE_BLACK;
        ex_node->parent->color = RBTREE_BLACK;
        left_rotate(t, ex_node->parent);
        ex_node = t->root;
      }
    } else {
      u = ex_node->parent->left;
      if (u->color == RBTREE_RED) {
        u->color = RBTREE_BLACK;
        ex_node->parent->color = RBTREE_RED;
        right_rotate(t, ex_node->parent);
        u = ex_node->parent->left;
      } 
      if (u->right->color == RBTREE_BLACK && u->left->color == RBTREE_BLACK) {
        u->color = RBTREE_RED;
        ex_node = ex_node->parent;
      } else {
        if (u->left->color == RBTREE_BLACK) {
          u->right->color = RBTREE_BLACK;
          u->color = RBTREE_RED;
          left_rotate(t, u);
          u = ex_node->parent->left;
        }
        u->color = ex_node->parent->color;
        u->left->color = RBTREE_BLACK;
        ex_node->parent->color = RBTREE_BLACK;
        right_rotate(t, ex_node->parent);
        ex_node = t->root;
      }
    }
  }
  ex_node->color = RBTREE_BLACK;
}

void transplant(rbtree *t, node_t *a, node_t *b) {
  if (a->parent == t->nil) {
    t->root = b;
  } else if (a->parent->left == a) {
    a->parent->left = b;
  } else {
    a->parent->right = b;
  }
  b->parent = a->parent;
}

int rbtree_erase(rbtree *t, node_t *node) {
  node_t *ex_node;
  int rm_color = node->color; // 삭제되는 노드 자식이 0,1 개 -> 삭제되는 노드 색
  if (node->left == t->nil) {
    ex_node = node->right;
    transplant(t, node, node->right);
  } else if (node->right == t->nil) {
    ex_node = node->left;
    transplant(t, node, node->left);
  } else {
    node_t *sc = rbtree_subtree_min(t, node->right);
    rm_color = sc->color;
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
