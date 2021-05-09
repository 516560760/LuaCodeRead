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


/* ���ù�ϣͰ��С */
void luaS_resize (lua_State *L, int newsize) {
  GCObject **newhash;
  stringtable *tb;
  int i;
  if (G(L)->gcstate == GCSsweepstring)
    return;  /* cannot resize during GC traverse ����GC�׶β����� */
  newhash = luaM_newvector(L, newsize, GCObject *); /* ���� GCObject * ���� */
  tb = &G(L)->strt;
  for (i=0; i<newsize; i++) newhash[i] = NULL;
  /* rehash */
  for (i=0; i<tb->size; i++) { /* ������ϣͰ���飬��֮ǰ���е�tsͨ����size���¼��������±�ŵ��µĹ�ϣͰ�У���Ϊ����������ʽ���ڣ������ڴ�仯ֻ�й�ϣͰ���� */
    GCObject *p = tb->hash[i];
    while (p) {  /* for each node in the list ���������� */
      GCObject *next = p->gch.next;  /* save next */
      unsigned int h = gco2ts(p)->hash;
      int h1 = lmod(h, newsize);  /* new position */
      lua_assert(cast_int(h%newsize) == lmod(h, newsize));
      p->gch.next = newhash[h1];  /* chain it */
      newhash[h1] = p;
      p = next;
    }
  }
  luaM_freearray(L, tb->hash, tb->size, TString *); /* �ͷ��Ϲ�ϣͰ���� */
  tb->size = newsize;
  tb->hash = newhash;
}

/* �����µ��ַ�����ӵ��ַ������� */
static TString *newlstr (lua_State *L, const char *str, size_t l,
                                       unsigned int h) {
  TString *ts;
  stringtable *tb;
  if (l+1 > (MAX_SIZET - sizeof(TString))/sizeof(char))
    luaM_toobig(L);
  ts = cast(TString *, luaM_malloc(L, (l+1)*sizeof(char)+sizeof(TString)));
  ts->tsv.len = l; /* �ַ������ȣ�����'\0' */
  ts->tsv.hash = h; /* ������ȷ��Ҫ�����µ��ַ������Ѿ������ظ��ַ�����飬���Թ�ϣֵ�Ǵ������� */
  ts->tsv.marked = luaC_white(G(L)); /* GC��ǰ�ɫ */
  ts->tsv.tt = LUA_TSTRING; /* ���ͱ�� */
  ts->tsv.reserved = 0; /* �Ǳ����ַ��������µ��ַ����Ѿ����˼�飬�϶����Ǳ����ַ��� */
  memcpy(ts+1, str, l*sizeof(char)); /* �ַ����ڴ濽�� */
  ((char *)(ts+1))[l] = '\0';  /* ending 0 */
  tb = &G(L)->strt;
  h = lmod(h, tb->size); /* ͨ���ַ�����ϣͰ����size-1����&���㣬��ȡ�����±� */
  ts->tsv.next = tb->hash[h];  /* chain new entry �������ַ������뵽����ͷ�� */
  tb->hash[h] = obj2gco(ts);
  tb->nuse++; /* �ַ������� */
  if (tb->nuse > cast(lu_int32, tb->size) && tb->size <= MAX_INT/2) /* ������ϣͰ��С */
    luaS_resize(L, tb->size*2);  /* too crowded */
  return ts;
}

/* ����һ���µ��ַ��� */
TString *luaS_newlstr (lua_State *L, const char *str, size_t l) {
  GCObject *o;
  unsigned int h = cast(unsigned int, l);  /* seed ȡ�ַ���������Ϊ��ʼ��ϣseed */
  size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars ��ֹ�ַ���̫����+1�Ǳ�֤���ټ���һ�ι�ϣ */
  size_t l1;
  for (l1=l; l1>=step; l1-=step)  /* compute hash �����ϣ */
    h = h ^ ((h<<5)+(h>>2)+cast(unsigned char, str[l1-1]));
  for (o = G(L)->strt.hash[lmod(h, G(L)->strt.size)];
       o != NULL;
       o = o->gch.next) { /* ������������Ƿ������ͬ�ַ����������򷵻� */
    TString *ts = rawgco2ts(o);
    if (ts->tsv.len == l && (memcmp(str, getstr(ts), l) == 0)) {
      /* string may be dead */
      if (isdead(G(L), o)) changewhite(o); /* ������ַ����Ѿ����ˣ����ǻ�û�л��գ���ǰ�ɫ */
      return ts;
    }
  }
  return newlstr(L, str, l, h);  /* not found û�ҵ��������µ� */
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

