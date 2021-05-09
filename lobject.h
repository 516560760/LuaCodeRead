/*
** $Id: lobject.h,v 2.20.1.2 2008/08/06 13:29:48 roberto Exp $
** Type definitions for Lua objects Lua��������Ͷ���
** See Copyright Notice in lua.h �����lua.h�еİ�Ȩ����
*/


#ifndef lobject_h
#define lobject_h


#include <stdarg.h>


#include "llimits.h"
#include "lua.h"


/* tags for values visible from Lua ͨ��lua�����ͽ��ж��δ���ı�ǩ */

/* lua values ���һ������ */
#define LAST_TAG	LUA_TTHREAD

/* lua values ���������� */
#define NUM_TAGS	(LAST_TAG+1)


/*
** Extra tags for non-values ��luaֵ���͵Ķ����ǩ
*/
#define LUA_TPROTO	(LAST_TAG+1)
#define LUA_TUPVAL	(LAST_TAG+2)
#define LUA_TDEADKEY	(LAST_TAG+3)


/*
** Union of all collectable objects ���п��ռ������union
*/
typedef union GCObject GCObject;

/*
** Common Header for all collectable objects (in macro form, to be included in other objects) ���п��ռ�����Ĺ�����ͷ���Ժ���ʽ�������������������У�
*/
#define CommonHeader	GCObject *next; lu_byte tt; lu_byte marked


/*
** Common header in struct form �ṹ��ʽ��ͨ�ñ�ͷ
*/
typedef struct GCheader {
  CommonHeader;
} GCheader;




/*
** Union of all Lua values ����Luaֵ��union
*/
typedef union {
  GCObject *gc;
  void *p; /* lightuserdata���� userdata �ŵ�gc�� */
  lua_Number n; /* number���� */
  int b; /* boolean���� */
} Value;


/*
** Tagged Values �����ͱ�ǵ�ֵ
*/

#define TValuefields	Value value; int tt

/*
** Tagged Values �������͵�luaֵ�ṹ�壬������������Ϊint tt
*/
typedef struct lua_TValue {
  TValuefields;
} TValue;


/* Macros to test type �������͵ĺ�(Test tt is xx) */

#define ttisnil(o)	(ttype(o) == LUA_TNIL) /* TValue�Ƿ�Ϊnil */
#define ttisnumber(o)	(ttype(o) == LUA_TNUMBER) /* TValue�Ƿ�Ϊnumber */
#define ttisstring(o)	(ttype(o) == LUA_TSTRING) /* TValue�Ƿ�Ϊstring */
#define ttistable(o)	(ttype(o) == LUA_TTABLE) /* TValue�Ƿ�Ϊtable */
#define ttisfunction(o)	(ttype(o) == LUA_TFUNCTION) /* TValue�Ƿ�Ϊfunction */
#define ttisboolean(o)	(ttype(o) == LUA_TBOOLEAN) /* TValue�Ƿ�Ϊboolean */
#define ttisuserdata(o)	(ttype(o) == LUA_TUSERDATA) /* TValue�Ƿ�Ϊuserdata */
#define ttisthread(o)	(ttype(o) == LUA_TTHREAD) /* TValue�Ƿ�Ϊthread */
#define ttislightuserdata(o)	(ttype(o) == LUA_TLIGHTUSERDATA) /* TValue�Ƿ�Ϊlightuserdata */

/* Macros to access values ���ʺ� */
#define ttype(o)	((o)->tt) /* ȡTValue������(tt) */
#define gcvalue(o)	check_exp(iscollectable(o), (o)->value.gc) /* ȡTValue��value.gc */
#define pvalue(o)	check_exp(ttislightuserdata(o), (o)->value.p) /* ȡTValue��value.p */
#define nvalue(o)	check_exp(ttisnumber(o), (o)->value.n) /* ȡTValue��value.n */
#define rawtsvalue(o)	check_exp(ttisstring(o), &(o)->value.gc->ts) /* ȡTValue��value.gc->ts�ĵ�ַ */
#define tsvalue(o)	(&rawtsvalue(o)->tsv) /* ȡTValue��value.gc->ts->tsv�ĵ�ַ */
#define rawuvalue(o)	check_exp(ttisuserdata(o), &(o)->value.gc->u)
#define uvalue(o)	(&rawuvalue(o)->uv)
#define clvalue(o)	check_exp(ttisfunction(o), &(o)->value.gc->cl)
#define hvalue(o)	check_exp(ttistable(o), &(o)->value.gc->h)
#define bvalue(o)	check_exp(ttisboolean(o), (o)->value.b)
#define thvalue(o)	check_exp(ttisthread(o), &(o)->value.gc->th)

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0)) /* �Ƿ�Ϊ�� */

/*
** for internal debug only �������ڲ�����
*/
/* �������һ����(�������ڲ�����) */
#define checkconsistency(obj) \
  lua_assert(!iscollectable(obj) || (ttype(obj) == (obj)->value.gc->gch.tt))

/* ����Ƿ񻹻��ţ�Ҳ�ж��������Ƿ�һ��(�������ڲ�����) */
#define checkliveness(g,obj) \
  lua_assert(!iscollectable(obj) || \
  ((ttype(obj) == (obj)->value.gc->gch.tt) && !isdead(g, (obj)->value.gc)))


/* Macros to set values ����ֵ�ĺ� */

/* TValue����Ϊnil */
#define setnilvalue(obj) ((obj)->tt=LUA_TNIL)
/* TValue����number */
#define setnvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.n=(x); i_o->tt=LUA_TNUMBER; }
/* TValue����lightuserdata */
#define setpvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.p=(x); i_o->tt=LUA_TLIGHTUSERDATA; }
/* TValue����boolean */
#define setbvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.b=(x); i_o->tt=LUA_TBOOLEAN; }
/* TValue����string����Ϊ��GCObject��������Ҫ��lua_State */
#define setsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TSTRING; \
    checkliveness(G(L),i_o); }
/* TValue����userdata����Ϊ��GCObject��������Ҫ��lua_State */
#define setuvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TUSERDATA; \
    checkliveness(G(L),i_o); }
/* TValue����thread����Ϊ��GCObject��������Ҫ��lua_State */
#define setthvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTHREAD; \
    checkliveness(G(L),i_o); }
/* TValue����function����Ϊ��GCObject��������Ҫ��lua_State */
#define setclvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TFUNCTION; \
    checkliveness(G(L),i_o); }
/* TValue����table����Ϊ��GCObject��������Ҫ��lua_State */
#define sethvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTABLE; \
    checkliveness(G(L),i_o); }
/* TValue����protp����Ϊ��GCObject��������Ҫ��lua_State */
#define setptvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TPROTO; \
    checkliveness(G(L),i_o); }



/* ��û�����ٵ� TValue2 ��ֵ�� TValue1 */
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


/* �Ƿ��ǻ�������(�жϷ�ʽΪȡTValue���ͣ��Ƿ����LUA_TSTRING) */
#define iscollectable(o)	(ttype(o) >= LUA_TSTRING)



typedef TValue *StkId;  /* index to stack elements */


/*
** String headers for string table �ڴ沼��Ϊsizeof(TString)+(len+1)*sizeof(char)���������ģ�+1����Ϊĩβ��һ��'\0'
*/
typedef union TString {
  L_Umaxalign dummy;  /* ensures maximum alignment for strings */
  struct {
    CommonHeader;
    lu_byte reserved; /* �����ַ��ı�� */
    unsigned int hash; /* �ַ�����ϣֵ���� stringtable �� ɢ��Ͱ����size��������±� �㷨��hash & (size-1) */
    size_t len; /* �ַ������� */
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

