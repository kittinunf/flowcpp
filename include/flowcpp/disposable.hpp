#pragma once

namespace flow {

template <class Disposed, class Disposable>
class basic_disposable {
 public:
  using disposed_t = Disposed;
  using disposable_t = Disposable;

  template <class T>
  basic_disposable(const T &t) : _p(new concrete<T>(t)) {}

  basic_disposable(basic_disposable &&) = default;

  basic_disposable(const basic_disposable &disposable)
      : _p(disposable._p->copy()) {}

  basic_disposable &operator=(basic_disposable disposable) {
    _p = move(disposable._p);
    return *this;
  }

  basic_disposable &operator=(basic_disposable &&) = default;

  template <class D1, class D2>
  friend D1 disposed(const basic_disposable<D1, D2> &disposable);

  template <class D1, class D2>
  friend D2 disposable(const basic_disposable<D1, D2> &disposable);

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

// disposable-based concept
template <class D1, class D2>
D1 disposed(const basic_disposable<D1, D2> &disposable) {
  return disposable._p->disposed();
}

template <class D1, class D2>
D2 disposable(const basic_disposable<D1, D2> &disposable) {
  return disposable._p->disposable();
}

}  // namespace flow
