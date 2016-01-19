#pragma once

namespace flow {

template <class Payload = std::string, class Type = int, class Meta = void *>
class basic_action {
 public:
  using payload_t = Payload;
  using type_t = Type;
  using meta_t = Meta;

  template <class T>
  basic_action(const T &t) : _p(new concrete<T>(t)) {}

  basic_action(basic_action &&) = default;

  basic_action(const basic_action &action) : _p(action._p->copy()) {}

  basic_action &operator=(basic_action action) {
    _p = std::move(action._p);
    return *this;
  }

  basic_action &operator=(basic_action &&) = default;

  template <class P, class T, class M>
  friend T type(const basic_action<P, T, M> &action);

  template <class P, class T, class M>
  friend P payload(const basic_action<P, T, M> &action);

  template <class P, class T, class M>
  friend bool error(const basic_action<P, T, M> &action);

  template <class P, class T, class M>
  friend M meta(const basic_action<P, T, M> &action);

 private:
  struct concept {
    virtual ~concept() = default;

    virtual concept *copy() const = 0;

    virtual payload_t payload() const = 0;

    virtual type_t type() const = 0;

    virtual meta_t meta() const = 0;

    virtual bool error() const = 0;
  };

  template <class T>
  struct concrete : public concept {
    explicit concrete(T t) : _t(std::move(t)) {}

    concept *copy() const override { return new concrete(*this); }

    payload_t payload() const override { return _t.payload(); }

    type_t type() const override { return _t.type(); }

    meta_t meta() const override { return _t.meta(); }

    bool error() const override { return _t.error(); }

    T _t;
  };

  std::unique_ptr<concept> _p;
};

// action-based concept
template <class P, class T, class M>
T type(const basic_action<P, T, M> &action) {
  return action._p->type();
}

template <class P, class T, class M>
P payload(const basic_action<P, T, M> &action) {
  return action._p->payload();
}

template <class P, class T, class M>
bool error(const basic_action<P, T, M> &action) {
  return action._p->error();
}

template <class P, class T, class M>
M meta(const basic_action<P, T, M> &action) {
  return action._p->meta();
}

}  // namespace flow
