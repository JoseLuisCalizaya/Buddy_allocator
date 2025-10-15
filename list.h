#pragma once
#ifndef LIST_H
#define LIST_H

struct Block;

class ListNode {
public:
  ListNode *prev = this;
  ListNode *next = this;
  ListNode();
  void push(ListNode *);
  ListNode *pop();
  void remove();
  Block *transmute();
};

#endif // LIST_H
