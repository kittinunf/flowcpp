#pragma once

#include <memory>

namespace flow {

class any {
 public:
  any() = default;

  any(const any &other) {
    if (other._p) _p = std::unique_ptr<concept>(other._p->copy());
  }

  template <class T>
  any(const T &t) : _p(new concrete<T>(t)) {}

  template <class T>
  any &operator=(T &&t) {
    _p.reset(new concrete<T>(std::forward<T>(t)));
    return *this;
  }

  any &operator=(any other) {
    _p = std::move(other._p);
    return *this;
  }

  any &operator=(any &&other) {
    _p = std::move(other._p);
    return *this;
  }

  template <class T>
  T &as() {
    return static_cast<concrete<T> *>(_p.get())->_t;
  }

  operator bool() const { return (_p)? true : false; }

 private:
  struct concept {
    virtual ~concept() = default;
    virtual concept *copy() const = 0;
  };

  template <class T>
  struct concrete : concept {
    concrete(T const &t) : _t(t) {}

    concept *copy() const override { return new concrete<T>(*this); }

    T _t;
  };

  std::unique_ptr<concept> _p;
};
}
