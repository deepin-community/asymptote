/***** Autogenerated from runhistory.in; changes will be overwritten *****/

#line 1 "runtimebase.in"
/*****
 * runtimebase.in
 * Andy Hammerlindl  2009/07/28
 *
 * Common declarations needed for all code-generating .in files.
 *
 *****/


#line 1 "runhistory.in"
/*****
 * runhistory.in
 *
 * Runtime functions for history operations.
 *
 *****/

#line 1 "runtimebase.in"
#include "stack.h"
#include "types.h"
#include "builtin.h"
#include "entry.h"
#include "errormsg.h"
#include "array.h"
#include "triple.h"
#include "callable.h"
#include "opsymbols.h"

using vm::stack;
using vm::error;
using vm::array;
using vm::read;
using vm::callable;
using types::formal;
using types::function;
using camp::triple;

#define PRIMITIVE(name,Name,asyName) using types::prim##Name;
#include <primitives.h>
#undef PRIMITIVE

typedef double real;

void unused(void *);

namespace run {
array *copyArray(array *a);
array *copyArray2(array *a);
array *copyArray3(array *a);

double *copyTripleArray2Components(array *a, size_t &N,
                                   GCPlacement placement=NoGC);
triple *copyTripleArray2C(array *a, size_t &N,
                          GCPlacement placement=NoGC);
}

function *realRealFunction();

#define CURRENTPEN processData().currentpen

#line 12 "runhistory.in"

#include "array.h"
#include "mathop.h"
#include "builtin.h"

using namespace camp;
using namespace settings;
using namespace vm;
using namespace run;

typedef array stringarray;
using types::stringArray;

namespace camp {
bool allowRender=true;
}

#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
#include <readline/readline.h>
#include <readline/history.h>

struct historyState {
  bool store;
  HISTORY_STATE state;
};

typedef mem::map<CONST string, historyState> historyMap_t;
historyMap_t historyMap;
static HISTORY_STATE history_save;

// Store a deep copy of the current readline history in dest.
void store_history(HISTORY_STATE *dest)
{
  HISTORY_STATE *src=history_get_history_state();
  if(src) {
    *dest=*src;
    for(Int i=0; i < src->length; ++i)
      dest->entries[i]=src->entries[i];
    free(src);
  }
}

stringarray* get_history(Int n)
{
  int N=intcast(n);
  if(N <= 0) N=history_length;
  else N=Min(N,history_length);
  array *a=new array((size_t) N);
  int offset=history_length-N+1;
  for(int i=0; i < N; ++i) {
    HIST_ENTRY *last=history_get(offset+i);
    string s=last ? last->line : "";
    (*a)[i]=s;
  }
  return a;
}

string historyfilename(const string &name)
{
  return historyname+"_"+name;
}

#endif

namespace run {

extern string emptystring;

#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)

#endif

void cleanup()
{
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
  store_history(&history_save);
  int nlines=intcast(getSetting<Int>("historylines"));
  for(historyMap_t::iterator h=historyMap.begin(); h != historyMap.end();
      ++h) {
    history_set_history_state(&h->second.state);
    if(h->second.store) {
      stifle_history(nlines);
      write_history(historyfilename(h->first).c_str());
      unstifle_history();
    }
  }
  history_set_history_state(&history_save);
#endif
#ifdef HAVE_LIBGSL
  trans::GSLrngFree();
#endif
}
}

// Autogenerated routines:



#ifndef NOSYM
#include "runhistory.symbols.h"

#endif
namespace run {
// Return the last n lines of the history named name.
#line 110 "runhistory.in"
// stringarray* history(string name, Int n=1);
void gen_runhistory0(stack *Stack)
{
  Int n=vm::pop<Int>(Stack,1);
  string name=vm::pop<string>(Stack);
#line 111 "runhistory.in"
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
  bool newhistory=historyMap.find(name) == historyMap.end();

  string filename;

  if(newhistory) {
    filename=historyfilename(name);
    std::ifstream exists(filename.c_str());
    if(!exists) {Stack->push<stringarray*>(new array(0)); return;}
  }

  store_history(&history_save);
  HISTORY_STATE& history=historyMap[name].state;
  history_set_history_state(&history);

  if(newhistory)
    read_history(filename.c_str());

  array *a=get_history(n);

  store_history(&history);
  history_set_history_state(&history_save);

  {Stack->push<stringarray*>(a); return;}
#else
  unused(&n);
  {Stack->push<stringarray*>(new array(0)); return;}
#endif
}

// Return the last n lines of the interactive history.
#line 143 "runhistory.in"
// stringarray* history(Int n=0);
void gen_runhistory1(stack *Stack)
{
  Int n=vm::pop<Int>(Stack,0);
#line 144 "runhistory.in"
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
  {Stack->push<stringarray*>(get_history(n)); return;}
#else
  unused(&n);
  {Stack->push<stringarray*>(new array(0)); return;}
#endif
}

// Prompt for a string using prompt, the GNU readline library, and a
// local history named name.
#line 155 "runhistory.in"
// string readline(string prompt=emptystring, string name=emptystring,                bool tabcompletion=false);
void gen_runhistory2(stack *Stack)
{
  bool tabcompletion=vm::pop<bool>(Stack,false);
  string name=vm::pop<string>(Stack,emptystring);
  string prompt=vm::pop<string>(Stack,emptystring);
#line 157 "runhistory.in"
  if(!(isatty(STDIN_FILENO) || getSetting<Int>("inpipe") >= 0))
    {Stack->push<string>(emptystring); return;}
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
  interact::init_readline(tabcompletion);

  store_history(&history_save);
  bool newhistory=historyMap.find(name) == historyMap.end();
  historyState& h=historyMap[name];
  HISTORY_STATE& history=h.state;
  history_set_history_state(&history);

  if(newhistory)
    read_history(historyfilename(name).c_str());

  static char *line=NULL;
  /* Return the memory to the free pool
     if the buffer has already been allocated. */
  if(line) {
    free(line);
    line=NULL;
  }

  /* Get a line from the user. */
  allowRender=false;
  line=readline(prompt.c_str());
  allowRender=true;

  if(!line) cout << endl;

  history_set_history_state(&history_save);

  {Stack->push<string>(line ? string(line) : emptystring); return;}
#else
  cout << prompt;
  string s;
  getline(cin,s);
  unused(&tabcompletion); // Avoid unused variable warning message.
  {Stack->push<string>(s); return;}
#endif
}

// Save a string in a local history named name.
// If store=true, store the local history in the file historyfilename(name).
#line 201 "runhistory.in"
// void saveline(string name, string value, bool store=true);
void gen_runhistory3(stack *Stack)
{
  bool store=vm::pop<bool>(Stack,true);
  string value=vm::pop<string>(Stack);
  string name=vm::pop<string>(Stack);
#line 202 "runhistory.in"
#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBCURSES)
  store_history(&history_save);
  bool newhistory=historyMap.find(name) == historyMap.end();
  historyState& h=historyMap[name];
  h.store=store;
  HISTORY_STATE& history=h.state;
  history_set_history_state(&history);

  if(newhistory)
    read_history(historyfilename(name).c_str());

  if(value != "") {
    add_history(value.c_str());
    if(store) {
      std::ofstream hout(historyfilename(name).c_str(),std::ios::app);
      hout << value << endl;
    }
  }

  store_history(&history);
  history_set_history_state(&history_save);
#else
  unused(&store);
#endif
}

} // namespace run

namespace trans {

void gen_runhistory_venv(venv &ve)
{
#line 109 "runhistory.in"
  addFunc(ve, run::gen_runhistory0, stringArray(), SYM(history), formal(primString(), SYM(name), false, false), formal(primInt(), SYM(n), true, false));
#line 142 "runhistory.in"
  addFunc(ve, run::gen_runhistory1, stringArray(), SYM(history), formal(primInt(), SYM(n), true, false));
#line 153 "runhistory.in"
  addFunc(ve, run::gen_runhistory2, primString(), SYM(readline), formal(primString(), SYM(prompt), true, false), formal(primString(), SYM(name), true, false), formal(primBoolean(), SYM(tabcompletion), true, false));
#line 199 "runhistory.in"
  addFunc(ve, run::gen_runhistory3, primVoid(), SYM(saveline), formal(primString(), SYM(name), false, false), formal(primString(), SYM(value), false, false), formal(primBoolean(), SYM(store), true, false));
}

} // namespace trans
