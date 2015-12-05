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
  
  nan_regular::nan_regular() : nanan::nan_regular_object() {
    
  }
  
  nan_regular::nan_regular(const std::string &re_str) : nanan::nan_regular_object() {
    load(re_str);
  }
  
  nan_regular::~nan_regular() {
    _delta_set.clear();
  }
  
  void nan_regular::load(const std::string &re_str) {
    if (re_str.empty()) _regular_expression.clear();
    else _regular_expression = re_str;
    
    _state_relation_table.clear();
    _curr_pos = 0;
    _curr_state = 0;
    
    parse();
  }
  
  std::vector<size_t> nan_regular::match(const std::string &str) {
    std::vector<size_t> res;
    
    return res;
  }
  
  std::shared_ptr<nan_regular::nan_regular_delta> nan_regular::make_delta(int e) {
    std::shared_ptr<nan_regular::nan_regular_delta> n =
      std::shared_ptr<nan_regular::nan_regular_delta>(new nan_regular::nan_regular_delta(_curr_state, _curr_state+1, e));
    if (n == nullptr) {
      error(NAN_ERROR_RUNTIME_ALLOC_MEMORY);
    }
    
    return n;
  }
  
  std::shared_ptr<nan_regular::nan_regular_delta> nan_regular::make_delta(std::vector<int> &e) {
    std::shared_ptr<nan_regular::nan_regular_delta> n =
    std::shared_ptr<nan_regular::nan_regular_delta>(new nan_regular::nan_regular_delta(_curr_state, _curr_state+1, e));
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
    
    return _regular_expression[_curr_pos];
  }
  
  int nan_regular::next_char() {
    if (_regular_expression.empty()) {
      throw nan_regular_except_eof(_curr_pos);
    }
    return _regular_expression[_curr_pos++];
  }
  
  int nan_regular::lookforward_char(size_t c) {
    if (_curr_pos + c >= _regular_expression.size()) {
      throw nan_regular_range_over(_curr_pos+c);
    }
    
    return _regular_expression[_curr_pos+c];
  }
  
  void nan_regular::rollback_char(size_t c) {
    if (static_cast<long>(_curr_pos - c) < 0) {
      throw nan_regular_range_over(_curr_pos-c);
    }
    
    _curr_pos -= c;
  }
  
  static bool s_is_print(char c) { return ((c >= 33) && (c < 126)); }
  static bool s_is_eof(char c) { return c == '\0'; }
  static const int s_nan_regular_char_epsilon = -1;
  
  void nan_regular::parse() {
    
    try {
      
      _delta_set = make_deltas();
      
      
    } catch (nan_regular_parse_not e) {                 /* 分析not操作符出错 */
    } catch (nan_regular_parse_set e) {                 /* 分析集合操作出错 */
    } catch (nan_regular_parse_sub e) {                 /* 分析子集操作出错 */
    } catch (nan_regular_range_over e) {                /* 读取越界 */
    } catch (nan_regular_parse_transferred e) {         /* 错误的转义字符 */
    } catch (nan_regular_except_eof e) {                /* 到达末尾 */
      return;
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
      if (s_is_print(ch)) {
        if ((ch >= '0') && (ch <= '9')) {
          /* 下一个字符是否是 '-' */
          char tmp = next_char();
          if (tmp == '-') {
            /* 范围操作符号 */
            tmp = ch;
            next_char();
            if ((ch > tmp) && (ch <= '9')) for (int i = tmp; i <= '9'; i++) s_add_to_unique_vector(chr_res, i);
            else throw nan_regular_parse_set(_curr_pos);
          } else rollback_char(1);
        } else if ((ch >= 'a') && (ch <= 'z')) {
          /* 下一个字符是否是 '-' */
          char tmp = next_char();
          if (tmp == '-') {
            /* 范围操作符号 */
            tmp = ch;
            next_char();
            if ((ch > tmp) && (ch <= 'z')) for (int i = tmp; i <= 'z'; i++) s_add_to_unique_vector(chr_res, i);
            else throw nan_regular_parse_set(_curr_pos);
          } else rollback_char(1);
        } else if ((ch >= 'A') && (ch <= 'Z')) {
          /* 下一个字符是否是 '-' */
          char tmp = next_char();
          if (tmp == '-') {
            /* 范围操作符号 */
            tmp = ch;
            next_char();
            if ((ch > tmp) && (ch <= 'Z')) for (int i = tmp; i <= 'Z'; i++) s_add_to_unique_vector(chr_res, i);
            else throw nan_regular_parse_set(_curr_pos);
          } else rollback_char(1);
        } else {
          
          if (ch == '\\') {
            /* 处理一些集合吧，目前没有任何实现 */
          }
          
          s_add_to_unique_vector(chr_res, ch);
        }
      } else {
        throw nan_regular_is_not_print_char(_curr_pos);
      }
      ch = next_char();
    }
    
    if (not_opt) {
      std::vector<int> tmp;
      for (int i = 33; i <= 126; i++) {
        for (auto j : chr_res) if (i != j) s_add_to_unique_vector(tmp, i);
      }
      chr_res = tmp;
    }
    
    return make_delta(chr_res);
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
  
  std::vector<std::shared_ptr<nan_regular::nan_regular_delta> > nan_regular::make_deltas() {
    std::vector<std::shared_ptr<nan_regular::nan_regular_delta> > res;
    while (true) {
      int ch = next_char();
      if (ch == 0) break;
      if (s_is_print(ch) == false) throw nan_regular_is_not_print_char(_curr_pos);
      
      if (ch == '\\') res.push_back(parse_transferred());
      else if (ch == '[') res.push_back(parse_set());
      else if (ch == '.') res.push_back(parse_dot());
      else if ((ch == '(') || (ch == '|') || (ch == '+') || (ch == '*')) ch = next_char();
      else res.push_back(make_delta(ch));
    }
    return res;
  }
  
  void nan_regular::print_deltas() {
    printf("total of delta = %lu\n", _delta_set.size());
    for (auto i : _delta_set) {
      printf("characters: \"");
      for (auto c : i->edge)
        printf("%c", c);
      printf("\"\n");
      printf("state%d -> state%d\n", i->left_state, i->right_state);
    }
  }
}