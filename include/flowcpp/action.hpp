#pragma once

#include <memory>
#include <string>
#include <type_traits>

#include "any.hpp"

namespace flow {

template <class Payload = flow::any, class Type = flow::any, class Meta = flow::any>
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

  type_t type() const { return _p->type(); }

  payload_t payload() const { return _p->payload(); }

  bool error() const { return _p->error(); }

  meta_t meta() const { return _p->meta(); }

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

using action = basic_action<>;

}  // namespace flow
