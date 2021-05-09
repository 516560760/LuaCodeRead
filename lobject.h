/*
** $Id: lobject.h,v 2.20.1.2 2008/08/06 13:29:48 roberto Exp $
** Type definitions for Lua objects Lua对象的类型定义
** See Copyright Notice in lua.h 请参阅lua.h中的版权声明
*/


#ifndef lobject_h
#define lobject_h


#include <stdarg.h>


#include "llimits.h"
#include "lua.h"


/* tags for values visible from Lua 通过lua的类型进行二次处理的标签 */

/* lua values 最后一个类型 */
#define LAST_TAG	LUA_TTHREAD

/* lua values 中类型总数 */
#define NUM_TAGS	(LAST_TAG+1)


/*
** Extra tags for non-values 非lua值类型的额外标签
*/
#define LUA_TPROTO	(LAST_TAG+1)
#define LUA_TUPVAL	(LAST_TAG+2)
#define LUA_TDEADKEY	(LAST_TAG+3)


/*
** Union of all collectable objects 所有可收集对象的union
*/
typedef union GCObject GCObject;

/*
** Common Header for all collectable objects (in macro form, to be included in other objects) 所有可收集对象的公共标头（以宏形式，将包含在其他对象中）
*/
#define CommonHeader	GCObject *next; lu_byte tt; lu_byte marked


/*
** Common header in struct form 结构形式的通用标头
*/
typedef struct GCheader {
  CommonHeader;
} GCheader;




/*
** Union of all Lua values 所有Lua值的union
*/
typedef union {
  GCObject *gc;
  void *p; /* lightuserdata数据 userdata 放到gc里 */
  lua_Number n; /* number数据 */
  int b; /* boolean数据 */
} Value;


/*
** Tagged Values 有类型标记的值
*/

#define TValuefields	Value value; int tt

/*
** Tagged Values 包含类型的lua值结构体，类型声明类型为int tt
*/
typedef struct lua_TValue {
  TValuefields;
} TValue;


/* Macros to test type 测试类型的宏(Test tt is xx) */

#define ttisnil(o)	(ttype(o) == LUA_TNIL) /* TValue是否为nil */
#define ttisnumber(o)	(ttype(o) == LUA_TNUMBER) /* TValue是否为number */
#define ttisstring(o)	(ttype(o) == LUA_TSTRING) /* TValue是否为string */
#define ttistable(o)	(ttype(o) == LUA_TTABLE) /* TValue是否为table */
#define ttisfunction(o)	(ttype(o) == LUA_TFUNCTION) /* TValue是否为function */
#define ttisboolean(o)	(ttype(o) == LUA_TBOOLEAN) /* TValue是否为boolean */
#define ttisuserdata(o)	(ttype(o) == LUA_TUSERDATA) /* TValue是否为userdata */
#define ttisthread(o)	(ttype(o) == LUA_TTHREAD) /* TValue是否为thread */
#define ttislightuserdata(o)	(ttype(o) == LUA_TLIGHTUSERDATA) /* TValue是否为lightuserdata */

/* Macros to access values 访问宏 */
#define ttype(o)	((o)->tt) /* 取TValue的类型(tt) */
#define gcvalue(o)	check_exp(iscollectable(o), (o)->value.gc) /* 取TValue的value.gc */
#define pvalue(o)	check_exp(ttislightuserdata(o), (o)->value.p) /* 取TValue的value.p */
#define nvalue(o)	check_exp(ttisnumber(o), (o)->value.n) /* 取TValue的value.n */
#define rawtsvalue(o)	check_exp(ttisstring(o), &(o)->value.gc->ts) /* 取TValue的value.gc->ts的地址 */
#define tsvalue(o)	(&rawtsvalue(o)->tsv) /* 取TValue的value.gc->ts->tsv的地址 */
#define rawuvalue(o)	check_exp(ttisuserdata(o), &(o)->value.gc->u)
#define uvalue(o)	(&rawuvalue(o)->uv)
#define clvalue(o)	check_exp(ttisfunction(o), &(o)->value.gc->cl)
#define hvalue(o)	check_exp(ttistable(o), &(o)->value.gc->h)
#define bvalue(o)	check_exp(ttisboolean(o), (o)->value.b)
#define thvalue(o)	check_exp(ttisthread(o), &(o)->value.gc->th)

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0)) /* 是否为假 */

/*
** for internal debug only 仅用于内部调试
*/
/* 检查类型一致性(仅用于内部调试) */
#define checkconsistency(obj) \
  lua_assert(!iscollectable(obj) || (ttype(obj) == (obj)->value.gc->gch.tt))

/* 检查是否还活着，也判断了类型是否一致(仅用于内部调试) */
#define checkliveness(g,obj) \
  lua_assert(!iscollectable(obj) || \
  ((ttype(obj) == (obj)->value.gc->gch.tt) && !isdead(g, (obj)->value.gc)))


/* Macros to set values 设置值的宏 */

/* TValue设置为nil */
#define setnilvalue(obj) ((obj)->tt=LUA_TNIL)
/* TValue设置number */
#define setnvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.n=(x); i_o->tt=LUA_TNUMBER; }
/* TValue设置lightuserdata */
#define setpvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.p=(x); i_o->tt=LUA_TLIGHTUSERDATA; }
/* TValue设置boolean */
#define setbvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.b=(x); i_o->tt=LUA_TBOOLEAN; }
/* TValue设置string，因为是GCObject，所以需要传lua_State */
#define setsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TSTRING; \
    checkliveness(G(L),i_o); }
/* TValue设置userdata，因为是GCObject，所以需要传lua_State */
#define setuvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TUSERDATA; \
    checkliveness(G(L),i_o); }
/* TValue设置thread，因为是GCObject，所以需要传lua_State */
#define setthvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTHREAD; \
    checkliveness(G(L),i_o); }
/* TValue设置function，因为是GCObject，所以需要传lua_State */
#define setclvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TFUNCTION; \
    checkliveness(G(L),i_o); }
/* TValue设置table，因为是GCObject，所以需要传lua_State */
#define sethvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTABLE; \
    checkliveness(G(L),i_o); }
/* TValue设置protp，因为是GCObject，所以需要传lua_State */
#define setptvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TPROTO; \
    checkliveness(G(L),i_o); }



/* 把没有销毁的 TValue2 赋值给 TValue1 */
#define setobj(L,obj1,obj2) \
  { const TValue *o2=(obj2); TValue *o1=(obj1); \
    o1->value = o2->value; o1->tt=o2->tt; \
    checkliveness(G(L),o1); }


/*
** different types of sets, according to destination
*/

/* from stack to (same) stack */
#define setobjs2s	setobj
/* to stack (not from same stack) */
#define setobj2s	setobj
#define setsvalue2s	setsvalue
#define sethvalue2s	sethvalue
#define setptvalue2s	setptvalue
/* from table to same table */
#define setobjt2t	setobj
/* to table */
#define setobj2t	setobj
/* to new object */
#define setobj2n	setobj
#define setsvalue2n	setsvalue

#define setttype(obj, tt) (ttype(obj) = (tt))


/* 是否是回收类型(判断方式为取TValue类型，是否大于LUA_TSTRING) */
#define iscollectable(o)	(ttype(o) >= LUA_TSTRING)



typedef TValue *StkId;  /* index to stack elements */


/*
** String headers for string table 内存布局为sizeof(TString)+(len+1)*sizeof(char)，是连续的，+1是因为末尾有一个'\0'
*/
typedef union TString {
  L_Umaxalign dummy;  /* ensures maximum alignment for strings */
  struct {
    CommonHeader;
    lu_byte reserved; /* 保留字符的标记 */
    unsigned int hash; /* 字符串哈希值，在 stringtable 中 散列桶数组size算出数组下标 算法：hash & (size-1) */
    size_t len; /* 字符串长度 */
  } tsv;
} TString;


#define getstr(ts)	cast(const char *, (ts) + 1)
#define svalue(o)       getstr(rawtsvalue(o))



typedef union Udata {
  L_Umaxalign dummy;  /* ensures maximum alignment for `local' udata */
  struct {
    CommonHeader;
    struct Table *metatable;
    struct Table *env;
    size_t len;
  } uv;
} Udata;




/*
** Function Prototypes
*/
typedef struct Proto {
  CommonHeader;
  TValue *k;  /* constants used by the function */
  Instruction *code;
  struct Proto **p;  /* functions defined inside the function */
  int *lineinfo;  /* map from opcodes to source lines */
  struct LocVar *locvars;  /* information about local variables */
  TString **upvalues;  /* upvalue names */
  TString  *source;
  int sizeupvalues;
  int sizek;  /* size of `k' */
  int sizecode;
  int sizelineinfo;
  int sizep;  /* size of `p' */
  int sizelocvars;
  int linedefined;
  int lastlinedefined;
  GCObject *gclist;
  lu_byte nups;  /* number of upvalues */
  lu_byte numparams;
  lu_byte is_vararg;
  lu_byte maxstacksize;
} Proto;


/* masks for new-style vararg */
#define VARARG_HASARG		1
#define VARARG_ISVARARG		2
#define VARARG_NEEDSARG		4


typedef struct LocVar {
  TString *varname;
  int startpc;  /* first point where variable is active */
  int endpc;    /* first point where variable is dead */
} LocVar;



/*
** Upvalues
*/

typedef struct UpVal {
  CommonHeader;
  TValue *v;  /* points to stack or to its own value */
  union {
    TValue value;  /* the value (when closed) */
    struct {  /* double linked list (when open) */
      struct UpVal *prev;
      struct UpVal *next;
    } l;
  } u;
} UpVal;


/*
** Closures
*/

#define ClosureHeader \
	CommonHeader; lu_byte isC; lu_byte nupvalues; GCObject *gclist; \
	struct Table *env

typedef struct CClosure {
  ClosureHeader;
  lua_CFunction f;
  TValue upvalue[1];
} CClosure;


typedef struct LClosure {
  ClosureHeader;
  struct Proto *p;
  UpVal *upvals[1];
} LClosure;


typedef union Closure {
  CClosure c;
  LClosure l;
} Closure;


#define iscfunction(o)	(ttype(o) == LUA_TFUNCTION && clvalue(o)->c.isC)
#define isLfunction(o)	(ttype(o) == LUA_TFUNCTION && !clvalue(o)->c.isC)


/*
** Tables
*/

typedef union TKey {
  struct {
    TValuefields;
    struct Node *next;  /* for chaining */
  } nk;
  TValue tvk;
} TKey;


typedef struct Node {
  TValue i_val;
  TKey i_key;
} Node;


typedef struct Table {
  CommonHeader;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */ 
  lu_byte lsizenode;  /* log2 of size of `node' array */
  struct Table *metatable;
  TValue *array;  /* array part */
  Node *node;
  Node *lastfree;  /* any free position is before this position */
  GCObject *gclist;
  int sizearray;  /* size of `array' array */
} Table;



/*
** `module' operation for hashing (size is always a power of 2)
*/
#define lmod(s,size) \
	(check_exp((size&(size-1))==0, (cast(int, (s) & ((size)-1)))))


#define twoto(x)	(1<<(x))
#define sizenode(t)	(twoto((t)->lsizenode))


#define luaO_nilobject		(&luaO_nilobject_)

LUAI_DATA const TValue luaO_nilobject_;

#define ceillog2(x)	(luaO_log2((x)-1) + 1)

LUAI_FUNC int luaO_log2 (unsigned int x);
LUAI_FUNC int luaO_int2fb (unsigned int x);
LUAI_FUNC int luaO_fb2int (int x);
LUAI_FUNC int luaO_rawequalObj (const TValue *t1, const TValue *t2);
LUAI_FUNC int luaO_str2d (const char *s, lua_Number *result);
LUAI_FUNC const char *luaO_pushvfstring (lua_State *L, const char *fmt,
                                                       va_list argp);
LUAI_FUNC const char *luaO_pushfstring (lua_State *L, const char *fmt, ...);
LUAI_FUNC void luaO_chunkid (char *out, const char *source, size_t len);


#endif

