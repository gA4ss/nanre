#include <nan_regular.h>
#include <nan_object.h>

#include <cstdio>
#include <cstdlib>
#include <cctype>

namespace nanan {
  
  class nan_regular_except {
  public:
    nan_regular_except(size_t p){ pos = p; };
    virtual ~nan_regular_except(){};
    size_t where() { return pos; }
    
  private:
    size_t pos;
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
  
  
  /********************************************************************************/
  
  nan_regular_object::nan_regular_object() : nanan::nan_object() {
    
  }
  
  nan_regular_object::~nan_regular_object() {
    
  }
  
  /********************************************************************************/
  
  nan_regular::nan_regular_delta::nan_regular_delta(int l, int r, int e) : nanan::nan_regular_object() {
    left_state = l;
    right_state = r;
    edge.push_back(e);
  }
  
  nan_regular::nan_regular_delta::nan_regular_delta(int l, int r, std::vector<int> &e, bool unique) : nanan::nan_regular_object() {
    left_state = l;
    right_state = r;
    
    /* unique操作 */
    if (unique == false) {
      edge = e;
    } else {
      bool found = false;
      for (auto i : e) {
        for (auto j : edge) if (i == j) { found = true; break; }
        if (found == false) edge.push_back(i);
        else found = false;
      }
    }
  }
  
  nan_regular::nan_regular_delta::~nan_regular_delta() {
    
  }
  /********************************************************************************/
  
  static bool s_add_to_unique_vector(std::vector<int> &v, int c) {
    for (auto i : v) {
      if (i == c) return false;
    }
    
    v.push_back(c);
    return true;
  }
  
  static bool s_is_print(char c) { return ((c >= 33) && (c < 126)); }
  static const int s_nan_regular_char_epsilon = -1;
  
  nan_regular::nan_regular(size_t mbl) : nanan::nan_regular_object() {
    _max_buffer_len = mbl;
  }
  
  nan_regular::nan_regular(const std::string &re_str, size_t mbl) : nanan::nan_regular_object() {
    _max_buffer_len = mbl;
    load(re_str);
  }
  
  nan_regular::~nan_regular() {
    _delta_set.clear();
    _curr_delta = nullptr;
  }
  
  void nan_regular::load(const std::string &re_str) {
    if (re_str.empty()) _regular_expression.clear();
    else {
      if (re_str.size() > _max_buffer_len) throw nan_regular_over_max_buffer_len(0);
      _regular_expression = re_str;
    }
    
    _state_relation_table.clear();
    _curr_pos = 0;
    _curr_state = 0;
    
    compile_regular_expression();
  }
  
  std::vector<size_t> nan_regular::match(const std::string &str) {
    std::vector<size_t> res;
    
    return res;
  }
  
  int nan_regular::next_state() {
    return _curr_state++;
  }
  
  std::shared_ptr<nan_regular::nan_regular_delta> nan_regular::make_delta(int e) {
    std::shared_ptr<nan_regular::nan_regular_delta> n =
      std::shared_ptr<nan_regular::nan_regular_delta>(new nan_regular::nan_regular_delta(-1, -1, e));
    if (n == nullptr) {
      error(NAN_ERROR_RUNTIME_ALLOC_MEMORY);
    }
    
    return n;
  }
  
  std::shared_ptr<nan_regular::nan_regular_delta> nan_regular::make_delta(std::vector<int> &e) {
    std::shared_ptr<nan_regular::nan_regular_delta> n =
    std::shared_ptr<nan_regular::nan_regular_delta>(new nan_regular::nan_regular_delta(-1, -1, e));
    if (n == nullptr) {
      error(NAN_ERROR_RUNTIME_ALLOC_MEMORY);
    }
    
    return n;
  }
  
  std::shared_ptr<nan_regular::nan_regular_delta> nan_regular::make_epsilon_delta(int l, int r) {
    std::shared_ptr<nan_regular::nan_regular_delta> n =
    std::shared_ptr<nan_regular::nan_regular_delta>(new nan_regular::nan_regular_delta(l,
                                                                                       r,
                                                                                       s_nan_regular_char_epsilon));
    if (n == nullptr) {
      error(NAN_ERROR_RUNTIME_ALLOC_MEMORY);
    }
    
    return n;
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
      
      _delta_set = parse();
      
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
  std::shared_ptr<nan_regular::nan_regular_delta> nan_regular::parse_dot() {
    std::vector<int> chr_res;
    for (int i = 1; i < 10; i++) {
      chr_res.push_back(i);
    }
    
    for (int i = 11; i < 128; i++) {
      chr_res.push_back(i);
    }
    
    return make_delta(chr_res);
  }
  
  std::shared_ptr<nan_regular::nan_regular_delta> nan_regular::parse_set() {
    if (curr_char() != '[') {
      throw nan_regular_parse_set(_curr_pos);
    }
    
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
    
    return make_delta(chr_res);
  }
  
  std::vector<std::shared_ptr<nan_regular::nan_regular_delta> > nan_regular::parse_sub() {
    if (curr_char() != '(') {
      throw nan_regular_parse_sub(_curr_pos);
    }
    
    return parse(')');
  }
  
  std::shared_ptr<nan_regular::nan_regular_delta> nan_regular::parse_transferred() {
    if (curr_char() != '\\') {
      throw nan_regular_parse_transferred(_curr_pos);
    }
    
    int ch = next_char();
    
    if ((ch == '(') ||
        (ch == '[') ||
        (ch == '.') ||
        (ch == '|') ||
        (ch == '+') ||
        (ch == '*') ||
        (ch == '\\')) {
      return make_delta(ch);
    } else {
      throw nan_regular_parse_transferred(_curr_pos);
    }
    return nullptr;
  }
  
  std::vector<std::shared_ptr<nan_regular::nan_regular_delta> > nan_regular::parse(int end) {
    std::vector<std::shared_ptr<nan_regular::nan_regular_delta> > res;
    std::shared_ptr<nan_regular::nan_regular_delta> d = nullptr;
    std::shared_ptr<nan_regular::nan_regular_delta> e = nullptr;
    
    /* 初始化第一个delta */
    if (_curr_delta == nullptr) {
      _curr_delta = std::shared_ptr<nan_regular::nan_regular_delta>(new nan_regular::nan_regular_delta(-1,
                                                                                                       0,
                                                                                                       s_nan_regular_char_epsilon));
      if (_curr_delta == nullptr) error(NAN_ERROR_RUNTIME_ALLOC_MEMORY);
      res.push_back(_curr_delta);
    }
    
    while (true) {
      int ch = next_char();
      if ((ch == 0) && (0 != end)) {
        throw nan_regular_not_found_right_close(_curr_pos, end);
      }
      if (ch == end) break;
      
      if (ch == '\\') {
        d = parse_transferred();
        _curr_delta->right_state = d->left_state;
        e = make_epsilon_delta(d->right_state, -1);
        _curr_delta = e;
        
        res.push_back(d);
        res.push_back(e);
      } else if (ch == '[') {
        d = parse_set();
        _curr_delta->right_state = d->left_state;
        e = make_epsilon_delta(d->right_state, -1);
        _curr_delta = e;
        
        res.push_back(d);
        res.push_back(e);
      } else if (ch == '.') {
        d = parse_dot();
        _curr_delta->right_state = d->left_state;
        e = make_epsilon_delta(d->right_state, -1);
        _curr_delta = e;
        
        res.push_back(d);
        res.push_back(e);
      } else if (ch == '(') {
        std::vector<std::shared_ptr<nan_regular::nan_regular_delta> > tmp;
        tmp = parse_sub();
        for (auto i : tmp) {
          _curr_delta->right_state = i->left_state;
          e = make_epsilon_delta(d->right_state, -1);
          _curr_delta = e;
          
          res.push_back(d);
          res.push_back(e);
        }
      } else if (ch == '|') {
      } else if (ch == '+') {
      } else if (ch == '*') {
      } else res.push_back(make_delta(ch));
    }
    return res;
  }
  
#if (NDEBUG==0)
  static void s_print_ansi(int c) {
    if (s_is_print(c)) printf("%c", c);
    else printf("%02x", c);
  }
  void nan_regular::print_deltas() {
    printf("total of delta = %lu\n", _delta_set.size());
    for (auto i : _delta_set) {
      printf("characters: \"");
      for (auto c : i->edge)
        s_print_ansi(c);
      printf("\"\n");
      printf("state%d -> state%d\n", i->left_state, i->right_state);
      printf("----------\n");
    }
  }
#endif
}