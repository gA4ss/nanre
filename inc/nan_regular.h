#ifndef nan_regular_h
#define nan_regular_h

#include <nan_object.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>

namespace nanan {
  
  class nan_regular_object : public nanan::nan_object {
  public:
    nan_regular_object();
    virtual ~nan_regular_object();
  };
  
  class nan_regular : public nan_regular_object {
  public:
    class nan_regular_delta : public nan_regular_object {
    public:
      nan_regular_delta(int l, int r, int e);
      nan_regular_delta(int l, int r, std::vector<int> &e, bool unique=false);
      virtual ~nan_regular_delta();
      
    public:
      int left_state;
      int right_state;
      std::vector<int> edge;
    };
    
  public:
    nan_regular();
    nan_regular(const std::string &re_str);
    virtual ~nan_regular();
    
  public:
    virtual void load(const std::string &re_str);
    virtual std::vector<size_t> match(const std::string &str);
    virtual void print_deltas();
    
    
  protected:
    virtual std::shared_ptr<nan_regular::nan_regular_delta> make_delta(int e);
    virtual std::shared_ptr<nan_regular::nan_regular_delta> make_delta(std::vector<int> &e);
    virtual void parse();
    virtual std::vector<std::shared_ptr<nan_regular::nan_regular_delta> > make_deltas();
    virtual std::shared_ptr<nan_regular::nan_regular_delta> parse_dot();
    virtual std::shared_ptr<nan_regular::nan_regular_delta> parse_set();
    virtual std::shared_ptr<nan_regular::nan_regular_delta> parse_transferred();
    
  protected:
    virtual void begin_pos();
    virtual void end_pos();
    virtual int curr_char();
    virtual int next_char();
    virtual int lookforward_char(size_t c);
    virtual void rollback_char(size_t c);
    
  protected:
    std::vector<std::vector<int> > _state_relation_table;       /*!< 生成的状态关系树会转换成此关系矩阵 */
    std::vector<std::shared_ptr<nan_regular::nan_regular_delta> > _delta_set;
    std::string _regular_expression;
    size_t _curr_pos;
    int _curr_state;
  };
}

#endif /* nan_regular_h */
