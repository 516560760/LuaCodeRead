/*
** $Id: lstring.c,v 2.8.1.1 2007/12/27 13:02:25 roberto Exp $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/


#include <string.h>

#define lstring_c
#define LUA_CORE

#include "lua.h"

#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"


/* 重置哈希桶大小 */
void luaS_resize (lua_State *L, int newsize) {
  GCObject **newhash;
  stringtable *tb;
  int i;
  if (G(L)->gcstate == GCSsweepstring)
    return;  /* cannot resize during GC traverse 处于GC阶段不进行 */
  newhash = luaM_newvector(L, newsize, GCObject *); /* 创建 GCObject * 数组 */
  tb = &G(L)->strt;
  for (i=0; i<newsize; i++) newhash[i] = NULL;
  /* rehash */
  for (i=0; i<tb->size; i++) { /* 遍历哈希桶数组，把之前所有的ts通过新size重新计算数组下标放到新的哈希桶中，因为是以链表形式存在，所以内存变化只有哈希桶数组 */
    GCObject *p = tb->hash[i];
    while (p) {  /* for each node in the list 遍历此链表 */
      GCObject *next = p->gch.next;  /* save next */
      unsigned int h = gco2ts(p)->hash;
      int h1 = lmod(h, newsize);  /* new position */
      lua_assert(cast_int(h%newsize) == lmod(h, newsize));
      p->gch.next = newhash[h1];  /* chain it */
      newhash[h1] = p;
      p = next;
    }
  }
  luaM_freearray(L, tb->hash, tb->size, TString *); /* 释放老哈希桶数组 */
  tb->size = newsize;
  tb->hash = newhash;
}

/* 创建新的字符串添加到字符串表中 */
static TString *newlstr (lua_State *L, const char *str, size_t l,
                                       unsigned int h) {
  TString *ts;
  stringtable *tb;
  if (l+1 > (MAX_SIZET - sizeof(TString))/sizeof(char))
    luaM_toobig(L);
  ts = cast(TString *, luaM_malloc(L, (l+1)*sizeof(char)+sizeof(TString)));
  ts->tsv.len = l; /* 字符串长度，不算'\0' */
  ts->tsv.hash = h; /* 这里是确定要生成新的字符串，已经过了重复字符串检查，所以哈希值是传过来的 */
  ts->tsv.marked = luaC_white(G(L)); /* GC标记白色 */
  ts->tsv.tt = LUA_TSTRING; /* 类型标记 */
  ts->tsv.reserved = 0; /* 非保留字符，创建新的字符串已经过了检查，肯定不是保留字符串 */
  memcpy(ts+1, str, l*sizeof(char)); /* 字符串内存拷贝 */
  ((char *)(ts+1))[l] = '\0';  /* ending 0 */
  tb = &G(L)->strt;
  h = lmod(h, tb->size); /* 通过字符串哈希桶数组size-1进行&运算，获取数组下标 */
  ts->tsv.next = tb->hash[h];  /* chain new entry 把新增字符串插入到链的头部 */
  tb->hash[h] = obj2gco(ts);
  tb->nuse++; /* 字符串个数 */
  if (tb->nuse > cast(lu_int32, tb->size) && tb->size <= MAX_INT/2) /* 调整哈希桶大小 */
    luaS_resize(L, tb->size*2);  /* too crowded */
  return ts;
}

/* 遇到一个新的字符串 */
TString *luaS_newlstr (lua_State *L, const char *str, size_t l) {
  GCObject *o;
  unsigned int h = cast(unsigned int, l);  /* seed 取字符串长度作为初始哈希seed */
  size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars 防止字符串太长，+1是保证最少计算一次哈希 */
  size_t l1;
  for (l1=l; l1>=step; l1-=step)  /* compute hash 计算哈希 */
    h = h ^ ((h<<5)+(h>>2)+cast(unsigned char, str[l1-1]));
  for (o = G(L)->strt.hash[lmod(h, G(L)->strt.size)];
       o != NULL;
       o = o->gch.next) { /* 遍历链表查找是否存在相同字符串，存在则返回 */
    TString *ts = rawgco2ts(o);
    if (ts->tsv.len == l && (memcmp(str, getstr(ts), l) == 0)) {
      /* string may be dead */
      if (isdead(G(L), o)) changewhite(o); /* 如果此字符串已经死了，但是还没有回收，标记白色 */
      return ts;
    }
  }
  return newlstr(L, str, l, h);  /* not found 没找到创建个新的 */
}


Udata *luaS_newudata (lua_State *L, size_t s, Table *e) {
  Udata *u;
  if (s > MAX_SIZET - sizeof(Udata))
    luaM_toobig(L);
  u = cast(Udata *, luaM_malloc(L, s + sizeof(Udata)));
  u->uv.marked = luaC_white(G(L));  /* is not finalized */
  u->uv.tt = LUA_TUSERDATA;
  u->uv.len = s;
  u->uv.metatable = NULL;
  u->uv.env = e;
  /* chain it on udata list (after main thread) */
  u->uv.next = G(L)->mainthread->next;
  G(L)->mainthread->next = obj2gco(u);
  return u;
}

