#pragma once

#include <functional>
#include <memory>

namespace flow {

template <class Disposed = std::function<bool()>, class Disposable = std::function<void()>>
class basic_disposable {
 public:
  using disposed_t = Disposed;
  using disposable_t = Disposable;

  template <class T>
  basic_disposable(const T &t) : _p(new concrete<T>(t)) {}

  basic_disposable(basic_disposable &&) = default;

  basic_disposable(const basic_disposable &disposable) : _p(disposable._p->copy()) {}

  basic_disposable &operator=(basic_disposable disposable) {
    _p = move(disposable._p);
    return *this;
  }

  basic_disposable &operator=(basic_disposable &&) = default;

  disposed_t disposed() const { return _p->disposed(); }

  disposable_t disposable() const { return _p->disposable(); }

  void dispose() const { _p->disposable()(); }

 private:
  struct concept {
    virtual ~concept() = default;

    virtual concept *copy() const = 0;

    virtual disposed_t disposed() const = 0;

    virtual disposable_t disposable() const = 0;
  };

  template <class T>
  struct concrete : public concept {
    explicit concrete(T t) : _t(std::move(t)) {}

    concept *copy() const override { return new concrete(*this); }

    disposed_t disposed() const override { return _t.disposed(); }

    disposable_t disposable() const override { return _t.disposable(); }

    T _t;
  };

  std::unique_ptr<concept> _p;
};

using disposable = basic_disposable<>;

}  // namespace flow
