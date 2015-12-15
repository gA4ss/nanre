#ifndef nan_regular_h
#define nan_regular_h

#include <nan_object.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <functional>

namespace nanan {
  
  class nan_regular_object : public nanan::nan_object {
  public:
    nan_regular_object();
    virtual ~nan_regular_object();
  };
  
  class nan_regular : public nan_regular_object {
  public:
    
    class nan_regular_edge : public nan_regular_object {
    public:
      nan_regular_edge(int e);
      nan_regular_edge(const std::vector<int> &e,
                       bool unique=false);
      virtual ~nan_regular_edge();
      
    public:
      bool matched(int c);
      
    public:
      bool epsilon;
      std::vector<int> charset;
    };
    typedef std::shared_ptr<nan_regular::nan_regular_edge> edge_t;
    
    class nan_regular_state : public nan_regular_object {
    public:
      nan_regular_state(int st);
      virtual ~nan_regular_state();
      
    public:
      void add_edge(const std::shared_ptr<nan_regular::nan_regular_state> &st,
                    int e);
      void add_edge(const std::shared_ptr<nan_regular::nan_regular_state> &st,
                    const std::vector<int> &e,
                    bool unique=false);
      void add_edge(const std::shared_ptr<nan_regular::nan_regular_state> &st,
                    const edge_t &edge);
      bool matched(int c);
      bool matched(const std::shared_ptr<nan_regular::nan_regular_state> &st);
      
    public:
      bool accept;
      int state;
      std::map<std::shared_ptr<nan_regular::nan_regular_state>, edge_t > edges;
    };
    typedef std::shared_ptr<nan_regular::nan_regular_state> state_t;
    
  public:
    nan_regular(size_t mbl=1024);
    nan_regular(const std::string &re_str, size_t mbl=1024);
    virtual ~nan_regular();
    
  public:
    virtual void compile(const std::string &re_str);
    virtual bool match_strict(const std::string &str);
    virtual std::vector<std::pair<size_t, size_t> > match_short(const std::string &str);
    virtual std::vector<std::pair<size_t, size_t> > match_long(const std::string &str);
    
#if NDEBUG==0
    virtual void print_states(nan_regular::state_t s);
    virtual void print_nfa();
    virtual void print_dfa();
#endif
    virtual void clear();
    
  protected:
    virtual int next_state();
    virtual nan_regular::state_t new_state(int st);
    virtual nan_regular::edge_t new_edge(int e);
    virtual nan_regular::edge_t new_edge(std::vector<int> &e,
                                         bool unique=false);
    
    virtual void compile_regular_expression();
    virtual nan_regular::state_t thompson(int end=0);
    virtual nan_regular::edge_t parse_dot();
    virtual nan_regular::edge_t parse_set();
    virtual nan_regular::edge_t parse_transferred();
    virtual void nfa2dfa(nan_regular::state_t s0);
    virtual std::vector<nan_regular::state_t>& e_closure(nan_regular::state_t state, bool clr=true);
    virtual std::vector<nan_regular::state_t>& e_closure(const std::vector<nan_regular::state_t> &states, bool clr=true);
    virtual std::vector<nan_regular::state_t> delta(std::vector<nan_regular::state_t> states, int c);
    virtual std::pair<std::vector<nan_regular::state_t>, std::vector<nan_regular::state_t> >
    split(const std::vector<nan_regular::state_t> &S);
    virtual void hopcroft();
    
  protected:
    virtual void link_state(nan_regular::edge_t edge);
    virtual void or_link_state(nan_regular::state_t top, nan_regular::edge_t edge);
    virtual void asterisk_link_state();
    virtual nan_regular::edge_t make_edge(int ch);

  public:
    static std::vector<nan_regular::state_t> find_accept_state(nan_regular::state_t top);
    static size_t state_sign(const nan_regular::state_t &state);
    static size_t states_sign(std::vector<nan_regular::state_t> states);
    static bool state_set_is_equal(const std::vector<nan_regular::state_t> &v1,
                                   const std::vector<nan_regular::state_t> &v2);
    static bool state_is_in_set(const nan_regular::state_t &state,
                                const std::vector<nan_regular::state_t> &set);
    static bool state_set_divide_is_equal(std::vector<std::vector<nan_regular::state_t> > v1,
                                          std::vector<std::vector<nan_regular::state_t> > v2);
    
  protected:
    virtual void begin_pos();
    virtual void end_pos();
    virtual int curr_char();
    virtual int next_char();
    virtual int lookforward_char(size_t c);
    virtual void rollback_char(size_t c);
    
  protected:
    std::string _regular_expression;                                    /*!< 正则表达式 */
    size_t _curr_pos;                                                   /*!< 当前的位置 */
    int _curr_state;                                                    /*!< 当前的状态 */
    state_t _nfa;                                                       /*!< 不确定性有限状态机 */
    state_t _dfa;                                                       /*!< 确定性有限状态机 */
    state_t _curr;                                                      /*!< 当前的状态 */
    state_t _prev;                                                      /*!< 上一个状态 */
    std::vector<state_t> _state_stack;                                  /*!< 状态栈 */
    std::vector<int> _charset;                                          /*!< 字符集 */
    std::vector<std::pair<size_t, nan_regular::state_t> > _dfa_set;     /*!< dfa的状态集合 */
    std::vector<std::vector<bool> > _dfa_map;                           /*!< 状态链接图 */
    
  private:
    size_t _max_buffer_len;
    std::vector<nan_regular::state_t> _e_closure_pass;                  /*!< 支持e_closure的临时变量 */
  };
}

#endif /* nan_regular_h */
