#include <nan_regular.h>
#include <nan_object.h>

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <utility>

namespace nanan {
  
  class nan_regular_except {
  public:
    nan_regular_except(size_t p){ pos = p; };
    virtual ~nan_regular_except(){};
    size_t where() { return pos; }
    
  private:
    size_t pos;
  };
  
  class nan_regular_parse_unknow : public nan_regular_except {
  public:
    nan_regular_parse_unknow() : nan_regular_except(0) {};
    virtual ~nan_regular_parse_unknow(){};
  };
  
  class nan_regular_parse_not : public nan_regular_except {
  public:
    nan_regular_parse_not(size_t p) : nan_regular_except(p) {};
    virtual ~nan_regular_parse_not(){};
  };
  
  class nan_regular_parse_transferred : public nan_regular_except {
  public:
    nan_regular_parse_transferred(size_t p) : nan_regular_except(p) {};
    virtual ~nan_regular_parse_transferred(){};
  };
  
  class nan_regular_parse_set : public nan_regular_except {
  public:
    nan_regular_parse_set(size_t p) : nan_regular_except(p) {};
    virtual ~nan_regular_parse_set(){};
  };
  
  class nan_regular_parse_sub : public nan_regular_except {
  public:
    nan_regular_parse_sub(size_t p) : nan_regular_except(p) {};
    virtual ~nan_regular_parse_sub(){};
  };
  
  class nan_regular_except_eof : public nan_regular_except {
  public:
    nan_regular_except_eof(size_t p) : nan_regular_except(p) {};
    virtual ~nan_regular_except_eof(){};
  };
  
  class nan_regular_range_over : public nan_regular_except {
  public:
    nan_regular_range_over(size_t p) : nan_regular_except(p) {};
    virtual ~nan_regular_range_over(){};
  };
  
  class nan_regular_is_not_print_char : public nan_regular_except {
  public:
    nan_regular_is_not_print_char(size_t p) : nan_regular_except(p) {};
    virtual ~nan_regular_is_not_print_char(){};
  };
  
  class nan_regular_over_max_buffer_len : public nan_regular_except {
  public:
    nan_regular_over_max_buffer_len(size_t p) : nan_regular_except(p) {};
    virtual ~nan_regular_over_max_buffer_len(){};
  };
  
  class nan_regular_not_found_right_close : public nan_regular_except {
  public:
    nan_regular_not_found_right_close(size_t p, int e) : nan_regular_except(p) { end = e; };
    virtual ~nan_regular_not_found_right_close(){};
    
  public:
    int right_close() const { return end; };
    
  private:
    int end;
  };
  
  class nan_regular_nfa2dfa : public nan_regular_except {
  public:
    nan_regular_nfa2dfa() : nan_regular_except(0) {};
    virtual ~nan_regular_nfa2dfa(){};
  };
  
  /********************************************************************************/
  
  static bool s_add_to_unique_vector(std::vector<int> &v, int c) {
    for (auto i : v) {
      if (i == c) return false;
    }
    
    v.push_back(c);
    return true;
  }
  
  static bool s_is_print(char c) { return ((c >= 33) && (c < 126)); }
  
  
  /********************************************************************************/
  
  nan_regular_object::nan_regular_object() : nanan::nan_object() {
    
  }
  
  nan_regular_object::~nan_regular_object() {
    
  }
  
  /********************************************************************************/
  
  nan_regular::nan_regular_edge::nan_regular_edge(int e) {
    if (e == -1) epsilon = true;
    else epsilon = false;
    
    charset.push_back(e);
  }
  
  nan_regular::nan_regular_edge::nan_regular_edge(std::vector<int> &e,
                                                  bool unique) {
    epsilon = false;
    /* unique操作 */
    if (unique == false) {
      charset = e;
    } else {
      bool found = false;
      for (auto i : e) {
        for (auto j : charset) if (i == j) { found = true; break; }
        if (found == false) charset.push_back(i);
        else found = false;
      }
    }
  }
  
  nan_regular::nan_regular_edge::~nan_regular_edge() {
    
  }
  
  bool nan_regular::nan_regular_edge::matched(int c) {
    for (auto i : charset) {
      if (i == c) return true;
    }
    return false;
  }
  
  /********************************************************************************/
  
  nan_regular::nan_regular_state::nan_regular_state(int st) : nanan::nan_regular_object() {
    state = st;
    accept = false;
  }
  
  
  nan_regular::nan_regular_state::~nan_regular_state() {
    edges.clear();
  }
  
  void nan_regular::nan_regular_state::add_edge(std::shared_ptr<nan_regular::nan_regular_state> st,
                                                int e) {
    std::shared_ptr<nan_regular::nan_regular_edge> edge =
    std::shared_ptr<nan_regular::nan_regular_edge>(new nan_regular::nan_regular_edge(e));
    if (edge == nullptr) error(NAN_ERROR_RUNTIME_ALLOC_MEMORY);
    add_edge(st, edge);
  }
  
  void nan_regular::nan_regular_state::add_edge(std::shared_ptr<nan_regular::nan_regular_state> st,
                                                std::vector<int> &e,
                                                bool unique) {
    std::shared_ptr<nan_regular::nan_regular_edge> edge =
    std::shared_ptr<nan_regular::nan_regular_edge>(new nan_regular::nan_regular_edge(e, unique));
    if (edge == nullptr) error(NAN_ERROR_RUNTIME_ALLOC_MEMORY);
    add_edge(st, edge);
  }

  void nan_regular::nan_regular_state::add_edge(std::shared_ptr<nan_regular::nan_regular_state> st,
                                                edge_t edge) {
    if (edges.find(st) == edges.end()) {
      edges[st] = edge;
    } else {
      for (auto c : edge->charset) {
        s_add_to_unique_vector(edges[st]->charset, c);
      }
    }
  }
  
  /********************************************************************************/
  
  nan_regular::nan_regular(size_t mbl) : nanan::nan_regular_object() {
    _max_buffer_len = mbl;
  }
  
  nan_regular::nan_regular(const std::string &re_str, size_t mbl) : nanan::nan_regular_object() {
    _max_buffer_len = mbl;
    load(re_str);
  }
  
  nan_regular::~nan_regular() {
  }
  
  void nan_regular::load(const std::string &re_str) {
    if (re_str.empty()) _regular_expression.clear();
    else {
      if (re_str.size() > _max_buffer_len) throw nan_regular_over_max_buffer_len(0);
      _regular_expression = re_str;
    }
    _curr_pos = 0;
    _curr_state = -1;
    _curr = nullptr;
    _prev = nullptr;
    _nfa = nullptr;
    _dfa = nullptr;
    _charset.clear();
    
    compile_regular_expression();
  }
  
  std::vector<size_t> nan_regular::match(const std::string &str) {
    std::vector<size_t> res;
    
    return res;
  }
  
  void nan_regular::print_states(nan_regular::state_t s) {
    if (s == nullptr) {
      printf("state map is empty.\n");
      return;
    }
    
    for (auto i : s->edges) {
      printf("s%d->s%d", s->state, i.first->state);
      if (i.first->accept) {
        printf("[accept]");
      }
      printf("\n");
      
      if (i.second->epsilon) {
        printf("[epsilon]");
      } else {
        for (auto c : i.second->charset) {
          printf("%c", c);
        }
      }
      printf("\n---------------------------------------------\n");
      if (s->state < i.first->state) {
        print_states(i.first);
      }
    }
  }
  
  void nan_regular::print_nfa() {
    print_states(_nfa);
  }
  
  void nan_regular::print_dfa() {
    print_states(_dfa);
  }
  
  int nan_regular::next_state() {
    return ++_curr_state;
  }
  
  nan_regular::state_t nan_regular::new_state(int st) {
    nan_regular::state_t ptr = std::shared_ptr<nan_regular::nan_regular_state>(new nan_regular::nan_regular_state(st));
    if (ptr == nullptr) error(NAN_ERROR_RUNTIME_ALLOC_MEMORY);
    _state_stack.push_back(ptr);
    return ptr;
  }
  
  nan_regular::edge_t nan_regular::new_edge(int e) {
    nan_regular::edge_t edge = std::shared_ptr<nan_regular::nan_regular_edge>(new nan_regular::nan_regular_edge(e));
    if (edge == nullptr) error(NAN_ERROR_RUNTIME_ALLOC_MEMORY);
    s_add_to_unique_vector(_charset, e);      /* 添加到字符集合 */
    return edge;
  }
  
  nan_regular::edge_t nan_regular::new_edge(std::vector<int> &e,
                                            bool unique) {
    nan_regular::edge_t edge = std::shared_ptr<nan_regular::nan_regular_edge>(new nan_regular::nan_regular_edge(e, unique));
    if (edge == nullptr) error(NAN_ERROR_RUNTIME_ALLOC_MEMORY);
    for (auto c : e) s_add_to_unique_vector(_charset, c);      /* 添加到字符集合 */
    return edge;
  }
  
  void nan_regular::begin_pos() { _curr_pos = 0; }
  void nan_regular::end_pos() { _curr_pos = _regular_expression.size() - 1; }
  int nan_regular::curr_char() {
    if (_curr_pos >= _regular_expression.size()) {
      throw nan_regular_range_over(_curr_pos);
    }
    
    int ch = 0;
    
    if (_curr_pos) ch = _regular_expression[_curr_pos-1];
    else ch = _regular_expression[0];
    
    return ch;
  }
  
  int nan_regular::next_char() {
    if (_regular_expression.empty()) {
      throw nan_regular_except_eof(_curr_pos);
    }
    int ch = _regular_expression[_curr_pos++];
    
    if (ch == 0) return 0;
    if (s_is_print(ch) == false) throw nan_regular_is_not_print_char(_curr_pos);
    return ch;
  }
  
  int nan_regular::lookforward_char(size_t c) {
    if (_curr_pos + c >= _regular_expression.size()) {
      throw nan_regular_range_over(_curr_pos+c);
    }
    
    int ch = _regular_expression[_curr_pos+c];
    if (ch == 0) return 0;
    if (s_is_print(ch)) throw nan_regular_is_not_print_char(_curr_pos);
    return ch;
  }
  
  void nan_regular::rollback_char(size_t c) {
    if (static_cast<long>(_curr_pos - c) < 0) {
      throw nan_regular_range_over(_curr_pos-c);
    }
    
    _curr_pos -= c;
  }
  
  void nan_regular::compile_regular_expression() {
    
    try {
      
      thompson(0);
      if (_state_stack.empty() == false) throw nan_regular_parse_unknow();
      nfa2dfa(_nfa);
    } catch (nan_regular_parse_unknow e) {              /* 不知名的错误 */
    } catch (nan_regular_parse_not e) {                 /* 分析not操作符出错 */
    } catch (nan_regular_parse_set e) {                 /* 分析集合操作出错 */
    } catch (nan_regular_parse_sub e) {                 /* 分析子集操作出错 */
    } catch (nan_regular_range_over e) {                /* 读取越界 */
    } catch (nan_regular_parse_transferred e) {         /* 错误的转义字符 */
    } catch (nan_regular_except_eof e) {                /* 到达末尾 */
    } catch (nan_regular_is_not_print_char e) {         /* 遇到不可打印字符 */
    } catch (...) { throw; }
  }
  
  /* 匹配除换行符号之外的所有符号 */
  nan_regular::edge_t nan_regular::parse_dot() {
    std::vector<int> chr_res;
    for (int i = 1; i < 10; i++) chr_res.push_back(i);
    for (int i = 11; i < 128; i++) chr_res.push_back(i);
    
    return new_edge(chr_res);
  }
  
  nan_regular::edge_t nan_regular::parse_set() {
    if (curr_char() != '[') throw nan_regular_parse_set(_curr_pos);
    
    bool not_opt = false;
    std::vector<int> chr_res;
    
    char ch = next_char();
    if (ch == '^') {
      not_opt = true;
      ch = next_char();
    }
    
    while (ch != ']') {
      if ((ch >= '0') && (ch <= '9')) {
        /* 下一个字符是否是 '-' */
        char tmp = next_char();
        if (tmp == '-') {
          /* 范围操作符号 */
          tmp = ch;
          ch = next_char();
          if ((ch > tmp) && (ch <= '9')) {
            for (int i = tmp; i <= '9'; i++) s_add_to_unique_vector(chr_res, i);
            goto _read_char;
          } else throw nan_regular_parse_set(_curr_pos);
        } else rollback_char(1);
      } else if ((ch >= 'a') && (ch <= 'z')) {
        /* 下一个字符是否是 '-' */
        char tmp = next_char();
        if (tmp == '-') {
          /* 范围操作符号 */
          tmp = ch;
          ch = next_char();
          if ((ch > tmp) && (ch <= 'z')) {
            for (int i = tmp; i <= 'z'; i++) s_add_to_unique_vector(chr_res, i);
            goto _read_char;
          } else throw nan_regular_parse_set(_curr_pos);
        } else rollback_char(1);
      } else if ((ch >= 'A') && (ch <= 'Z')) {
        /* 下一个字符是否是 '-' */
        char tmp = next_char();
        if (tmp == '-') {
          /* 范围操作符号 */
          tmp = ch;
          ch = next_char();
          if ((ch > tmp) && (ch <= 'Z')) {
            for (int i = tmp; i <= 'Z'; i++) s_add_to_unique_vector(chr_res, i);
            goto _read_char;
          } else throw nan_regular_parse_set(_curr_pos);
        } else rollback_char(1);
      }
      
      if (ch == '\\') {
        /* 处理一些集合吧，目前没有任何实现 */
      }
      s_add_to_unique_vector(chr_res, ch);
      
    _read_char:
      ch = next_char();
      if (ch == 0) throw nan_regular_not_found_right_close(_curr_pos, ']');
    }
    
    /* 如果存在否操作符号，则取反 */
    if (not_opt) {
      std::vector<int> tmp;
      bool b = false;
      for (int i = 33; i <= 126; i++) {
        for (auto j : chr_res) { if (j == i) { b = true; break; } }
        if (!b) s_add_to_unique_vector(tmp, i);
        else b = false;
      }
      chr_res = tmp;
    }
    
    return new_edge(chr_res);
  }
  
  nan_regular::edge_t nan_regular::parse_transferred() {
    if (curr_char() != '\\') throw nan_regular_parse_transferred(_curr_pos);
    
    int ch = next_char();
    
    if ((ch == '(') ||
        (ch == '[') ||
        (ch == '.') ||
        (ch == '|') ||
        (ch == '+') ||
        (ch == '*') ||
        (ch == '\\')) {
      return new_edge(ch);
    } else {
      throw nan_regular_parse_transferred(_curr_pos);
    }
    return nullptr;
  }
  
  void nan_regular::link_state(nan_regular::edge_t edge) {
    state_t st = new_state(next_state());
    _curr->add_edge(st, -1);
    _curr = st;
    st = new_state(next_state());
    _curr->add_edge(st, edge);
    _prev = _curr;
    _curr = st;
  }
  
  void nan_regular::or_link_state(nan_regular::state_t top,
                                  nan_regular::edge_t edge) {
    state_t st = new_state(next_state());
    top->add_edge(st, -1);
    _curr = st;
    
    st = new_state(next_state());
    _curr->add_edge(st, edge);
    _prev = _curr;
    _curr = st;
  }
  
  void nan_regular::asterisk_link_state() {
    _curr->add_edge(_prev, -1);
    state_t st = new_state(next_state());
    _prev->add_edge(st, -1);
    
    state_t one = new_state(next_state());
    st->add_edge(one, -1);
    _curr->add_edge(one, -1);
    _curr = one;
  }
  
  nan_regular::edge_t nan_regular::make_edge(int ch) {
    edge_t edge = nullptr;
    
    if (ch == '\\') edge = parse_transferred();
    else if (ch == '[') edge = parse_set();
    else if (ch == '.') edge = parse_dot();
    else edge = new_edge(ch);
    
    return edge;
  }
  
  std::vector<nan_regular::state_t> nan_regular::find_accept_state(nan_regular::state_t top) {
    std::vector<nan_regular::state_t> res, sub;
    int count = 0;
    
    for (auto i : top->edges) {
      /* 保证不进入回环的路线 */
      if (i.first->state > top->state) {
        sub = find_accept_state(i.first);
        for (auto j : sub) res.push_back(j);
        count++;
      }
    }
    
    if (count == 0) {
      res.push_back(top);
      return res;
    }
    
    return res;
  }
  
  size_t nan_regular::state_sign(const nan_regular::state_t &state) {
    return std::hash<nan_regular::state_t>()(state);
  }
  
  size_t nan_regular::states_sign(std::vector<nan_regular::state_t> states) {
    if (states.empty()) {
      return 0;
    }
    
    std::sort(states.begin(), states.end(), [](nan_regular::state_t n, nan_regular::state_t m){
      return n < m;
    });
    
    std::ostringstream oss;
    for (auto i : states) {
      size_t hash = std::hash<int>()(i->state);
      oss << hash;
      //std::cout << oss.str() << std::endl;
    }
    
    return std::hash<std::string>()(oss.str());
  }
  
  bool nan_regular::state_set_is_equal(const std::vector<nan_regular::state_t> &v1,
                                       const std::vector<nan_regular::state_t> &v2) {
    if (v1.size() != v2.size()) {
      return false;
    }
    
    return (states_sign(v1) == states_sign(v2));
  }
  
  bool nan_regular::state_is_in_set(const nan_regular::state_t &state,
                                    const std::vector<nan_regular::state_t> &set) {
    if (set.empty()) {
      return false;
    }
    
    for (auto s : set) {
      if (s->state == state->state) {
        return true;
      }
    }
    
    return false;
  }
  
  nan_regular::state_t nan_regular::thompson(int end) {
    state_t top = nullptr, call = nullptr;
    edge_t edge = nullptr;
    
    top = new_state(next_state());
    if (_nfa == nullptr) {
      _nfa = _curr = top;
    } else {
      _curr = top;
    }
    
    bool is_or = false;
    int ch = 0, prev_ch = 0;
    while (true) {
      prev_ch = ch;
      ch = next_char();
      if ((ch == 0) && (0 != end)) throw nan_regular_not_found_right_close(_curr_pos, end);
      if (ch == end) break;
      
      if (ch == '(') { call = _curr, _prev = thompson(')'); }
      else if (ch == '|') { is_or = true; continue; }
      else if (ch == '+') { _curr->add_edge(_prev, -1); continue; }
      else if (ch == '*') { asterisk_link_state(); continue; }
      else edge = make_edge(ch);
      
      if (edge) {
        if (is_or) or_link_state(top, edge);
        else link_state(edge);
      } else {
        if (is_or) top->add_edge(_prev, -1);
        else call->add_edge(_prev, -1);
      }
      
      is_or = false;
      edge = nullptr;
    }
    
    /* 建立完成状态并且做聚合 */
    std::vector<nan_regular::state_t> accept = find_accept_state(top);
    if (accept.empty() == false) {
      state_t tail = new_state(next_state());
      for (auto a : accept) {
        a->add_edge(tail, -1);
      }
      _curr = tail;
    }
    
    size_t index = _state_stack.size();
    while (top != _state_stack[--index]) _state_stack.pop_back();
    _state_stack.pop_back();        /* 弹出栈顶 */
    
    if (end == 0) {
      _curr->accept = true;
    }
    
    return top;
  }
  
  std::vector<nan_regular::state_t>& nan_regular::e_closure(nan_regular::state_t state, bool clr) {
    if (clr == true) {
      _e_closure_pass.clear();
    }
    
    _e_closure_pass.push_back(state);
    
    for (auto e : state->edges) {
      if (e.second->epsilon) {
        /* 如果当前检测状态不在已经遍历的表中则进入 */
        if (state_is_in_set(e.first, _e_closure_pass) == false) {
          e_closure(e.first, false);
        }
      }
    }
  
    return _e_closure_pass;
  }
  
  std::vector<nan_regular::state_t>& nan_regular::e_closure(const std::vector<nan_regular::state_t> &states, bool clr) {
    if (clr == true) {
      _e_closure_pass.clear();
    }
    
    for (auto s : states) {
      if (state_is_in_set(s, _e_closure_pass) == false) _e_closure_pass.push_back(s);
      for (auto e : s->edges) {
        if (e.second->epsilon) {
          /* 如果当前检测状态不在已经遍历的表中则进入 */
          if (state_is_in_set(e.first, _e_closure_pass) == false) {
            e_closure(e.first, false);
          }
        }
      }
    }
    
    return _e_closure_pass;
  }
  
  std::vector<nan_regular::state_t> nan_regular::delta(std::vector<nan_regular::state_t> states, int c) {
    std::vector<nan_regular::state_t> res;
    
    if (states.empty()) {
      return res;
    }
    
    for (auto i : states) {
      for (auto j : i->edges) {
        if (j.second->matched(c)) res.push_back(j.first);
      }
    }
    
    return res;
  }
  
  static bool s_states_has_accept(const std::vector<nan_regular::state_t> &states) {
    for (auto s : states) if (s->accept) return true;
    return false;
  }
  
  /* 子集构造算法 */
  void nan_regular::nfa2dfa(nan_regular::state_t s0) {
    std::vector<std::vector<nan_regular::state_t> > Q;
    std::vector<std::vector<nan_regular::state_t> > wl;
    std::vector<nan_regular::state_t> q0 = e_closure(s0);       /* 剔除开头的所有e移动 */
    Q.push_back(q0);
    wl.push_back(q0);
    
    int d_s = 0;
    std::vector<std::pair<size_t, nan_regular::state_t> > d_set;
    nan_regular::state_t d = nullptr;
    
    while (wl.empty() == false) {
      std::vector<nan_regular::state_t> q = wl.back();
      size_t start_sign = states_sign(q);
      
      /* 在d集合中寻找 */
      d = nullptr;
      for (auto x : d_set) if (x.first == start_sign) { d = x.second; break;}
      if (d == nullptr) {
        d = new_state(d_s++);
        if (s_states_has_accept(q)) d->accept = true;
        if (_dfa == nullptr) _dfa = d;
        d_set.push_back(std::make_pair(start_sign, d));
      }
      wl.pop_back();
      
      /* 测试字符集 */
      for (auto c : _charset) {
        /* q -(c)-> ways_sign */
        std::vector<nan_regular::state_t> branchs = delta(q, c); if (branchs.empty()) continue;
        std::vector<nan_regular::state_t> ways = e_closure(branchs); if (ways.empty()) continue;
        size_t end_sign = states_sign(ways);
        
        /* 在d集合中寻找 */
        nan_regular::state_t d_next = nullptr;
        for (auto x : d_set) if (x.first == end_sign) d_next = x.second;
        if (d_next == nullptr) {
          d_next = new_state(d_s++);
          if (s_states_has_accept(ways)) d_next->accept = true;
          d_set.push_back(std::make_pair(end_sign, d_next));
        }
        d->add_edge(d_next, c);
        
        /* 测试当前的状态集合是否在Q中 */
        bool next_loop = false;
        for (auto i : Q) if (states_sign(i) == end_sign) { next_loop = true; break; }
        if (next_loop) continue;
        
        /* 加入到Q与工作列表中 */
        Q.push_back(ways);
        wl.push_back(ways);
      }
    }/* end while */
  }
}