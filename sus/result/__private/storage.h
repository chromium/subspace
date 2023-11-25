// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

#include <concepts>
#include <cstdint>

#include "sus/cmp/ord.h"
#include "sus/macros/no_unique_address.h"
#include "sus/mem/copy.h"
#include "sus/mem/move.h"
#include "sus/mem/take.h"

namespace sus::result::__private {

enum WithT { WITH_T };
enum WithE { WITH_E };

template <class T, bool no_unique_address>
struct MaybeNoUniqueAddress {
  template <class... U>
  constexpr MaybeNoUniqueAddress(WithT, U&&... v) noexcept
      : v(WITH_T, ::sus::forward<U>(v)...) {}
  template <class... U>
  constexpr MaybeNoUniqueAddress(WithE, U&&... v) noexcept
      : v(WITH_E, ::sus::forward<U>(v)...) {}

  T v;
};
template <class T>
struct MaybeNoUniqueAddress<T, true> {
  template <class... U>
  constexpr MaybeNoUniqueAddress(WithT, U&&... v) noexcept
      : v(WITH_T, ::sus::forward<U>(v)...) {}
  template <class... U>
  constexpr MaybeNoUniqueAddress(WithE, U&&... v) noexcept
      : v(WITH_E, ::sus::forward<U>(v)...) {}

  [[sus_no_unique_address]] T v;
};

template <class E>
struct StorageVoid {
  enum State { Ok, Err, Moved };

  constexpr StorageVoid(WithT) noexcept : inner_(WITH_T) {}
  constexpr StorageVoid(WithE, const E& e) noexcept
    requires(std::is_copy_constructible_v<E>)
      : inner_(WITH_E, e) {}
  constexpr StorageVoid(WithE, E&& e) noexcept
    requires(std::is_move_constructible_v<E>)
      : inner_(WITH_E, ::sus::move(e)) {}

  ~StorageVoid() noexcept = default;

  StorageVoid(const StorageVoid&) noexcept = default;
  StorageVoid& operator=(const StorageVoid&) noexcept = default;
  StorageVoid(StorageVoid&&) noexcept = default;
  StorageVoid& operator=(StorageVoid&&) noexcept = default;

  constexpr bool is_moved() const noexcept { return inner_.v.state == Moved; }
  constexpr bool is_ok() const noexcept { return inner_.v.state == Ok; }
  constexpr bool is_err() const noexcept { return inner_.v.state == Err; }

  template <class As>
  constexpr void get_ok() const noexcept {}
  constexpr const E& get_err() const noexcept {
    return inner_.v.u.err;  //
  }

  template <class As>
  constexpr void get_ok_mut() & noexcept {}
  constexpr E& get_err_mut() & noexcept {
    return inner_.v.u.err;  //
  }

  template <class As>
  constexpr void take_ok() & noexcept {
    inner_.v.state = Moved;  //
  }
  constexpr E take_err() & noexcept
    requires(::sus::mem::Move<E>)
  {
    inner_.v.state = Moved;
    return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                         inner_.v.u.err);
  }

  constexpr void drop_ok() & noexcept {
    inner_.v.state = Moved;  //
  }
  constexpr void drop_err() & noexcept {
    inner_.v.state = Moved;
    std::destroy_at(&inner_.v.u.err);
  }

 private:
  struct Inner {
    constexpr explicit Inner(WithT) : u(WITH_T), state(Ok) {}
    template <class U>
    constexpr Inner(WithE, U&& err)
        : u(WITH_E, ::sus::forward<U>(err)), state(Err) {}

    ~Inner() noexcept
      requires(std::is_trivially_destructible_v<E>)
    = default;
    constexpr ~Inner() noexcept
      requires(!std::is_trivially_destructible_v<E>)
    {
      switch (state) {
        case Ok: break;
        case Err: std::destroy_at(&u.err); break;
        case Moved: break;
      }
    }

    Inner(const Inner&)
      requires(std::is_trivially_copy_constructible_v<E>)
    = default;
    constexpr Inner(const Inner& o)
      requires(!std::is_trivially_copy_constructible_v<E> &&
               std::is_copy_constructible_v<E>)
    {
      switch (o.state) {
        case Ok: break;
        case Err: std::construct_at(&u.err, o.u.err); break;
        case Moved: panic_with_message("Result used after move");
      }
      // After construct_at since it may write into the field if it's in tail
      // padding.
      state = o.state;
    }

    Inner& operator=(const Inner&)
      requires(std::is_trivially_copy_assignable_v<E>)
    = default;
    constexpr Inner& operator=(const Inner& o)
      requires(!std::is_trivially_copy_assignable_v<E> &&
               std::is_copy_assignable_v<E>)
    {
      check_with_message(o.state != Moved, "Result used after move");
      if (state == o.state) {
        switch (state) {
          case Ok: break;
          case Err: u.err = o.u.err; break;
          case Moved: break;
        }
      } else {
        switch (state) {
          case Ok: break;
          case Err: std::destroy_at(&u.err); break;
          case Moved: break;
        }
        // If this trips, it means the destructor in this Result moved out
        // of the Result that is being assigned from.
        check_with_message(o.state != Moved, "Result used after move");
        switch (o.state) {
          case Ok: break;
          case Err: std::construct_at(&u.err, o.u.err); break;
          case Moved: sus::unreachable_unchecked(::sus::marker::unsafe_fn);
        }
      }
      // After construct_at since it may write into the field if it's in tail
      // padding.
      state = o.state;
      return *this;
    }

    Inner(Inner&&)
      requires(std::is_trivially_move_constructible_v<E>)
    = default;
    constexpr Inner(Inner&& o)
      requires(!std::is_trivially_move_constructible_v<E>)
    {
      switch (o.state) {
        case Ok: break;
        case Err: std::construct_at(&u.err, ::sus::move(o.u.err)); break;
        case Moved: panic_with_message("Result used after move");
      }
      // After construct_at since it may write into the field if it's in tail
      // padding.
      state = ::sus::mem::replace(o.state, Moved);
    }

    Inner& operator=(Inner&&)
      requires(std::is_trivially_move_assignable_v<E>)
    = default;
    constexpr Inner& operator=(Inner&& o)
      requires(!std::is_trivially_move_assignable_v<E>)
    {
      check_with_message(o.state != Moved, "Result used after move");
      if (state == o.state) {
        switch (state) {
          case Ok: break;
          case Err: u.err = ::sus::move(o.u.err); break;
          case Moved: break;
        }
      } else {
        switch (state) {
          case Ok: break;
          case Err: std::destroy_at(&u.err); break;
          case Moved: break;
        }
        // If this trips, it means the destructor in this Result moved out
        // of the Result that is being assigned from.
        check_with_message(o.state != Moved, "Result used after move");
        switch (o.state) {
          case Ok: break;
          case Err:
            std::construct_at(&u.err, ::sus::move(o.u.err));
            std::destroy_at(&o.u.err);
            break;
          case Moved: sus::unreachable_unchecked(::sus::marker::unsafe_fn);
        }
      }
      // After construct_at since it may write into the field if it's in tail
      // padding.
      state = ::sus::mem::replace(o.state, Moved);
      return *this;
    }

    union Union {
      constexpr Union() noexcept {}

      constexpr Union(WithT) noexcept {}
      template <class U>
      constexpr Union(WithE, U&& err) noexcept : err(::sus::forward<U>(err)) {}

      ~Union() noexcept
        requires(std::is_trivially_destructible_v<E>)
      = default;
      constexpr ~Union() noexcept
        requires(!std::is_trivially_destructible_v<E>)
      {}

      Union(const Union&)
        requires(std::is_trivially_copy_constructible_v<E>)
      = default;
      Union& operator=(const Union&)
        requires(std::is_trivially_copy_assignable_v<E>)
      = default;
      Union(Union&&)
        requires(std::is_trivially_move_constructible_v<E>)
      = default;
      Union& operator=(Union&&)
        requires(std::is_trivially_move_assignable_v<E>)
      = default;

      [[no_unique_address]] E err;
    };
    [[no_unique_address]] Union u;
    [[no_unique_address]] State state;
  };
  static constexpr bool union_cant_clobber_outside_inner =
      ::sus::mem::data_size_of<E>() == ::sus::mem::size_of<E>();

  [[no_unique_address]] MaybeNoUniqueAddress<Inner,
                                             union_cant_clobber_outside_inner>
      inner_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(inner_.v.u.err),
                                           decltype(inner_.v.state));
};

template <class T, class E>
struct StorageNonVoid {
  enum State { Ok, Err, Moved };

  constexpr StorageNonVoid(WithT, const T& t) noexcept
    requires(std::is_copy_constructible_v<T>)
      : inner_(WITH_T, t) {}
  constexpr StorageNonVoid(WithT, T&& t) noexcept
    requires(std::is_move_constructible_v<T>)
      : inner_(WITH_T, ::sus::move(t)) {}
  constexpr StorageNonVoid(WithE, const E& e) noexcept
    requires(std::is_copy_constructible_v<E>)
      : inner_(WITH_E, e) {}
  constexpr StorageNonVoid(WithE, E&& e) noexcept
    requires(std::is_move_constructible_v<E>)
      : inner_(WITH_E, ::sus::move(e)) {}

  ~StorageNonVoid() noexcept = default;

  StorageNonVoid(const StorageNonVoid& o) noexcept = default;
  StorageNonVoid& operator=(const StorageNonVoid&) noexcept = default;
  StorageNonVoid(StorageNonVoid&&) noexcept = default;
  StorageNonVoid& operator=(StorageNonVoid&&) noexcept = default;

  constexpr bool is_moved() const noexcept { return inner_.v.state == Moved; }
  constexpr bool is_ok() const noexcept { return inner_.v.state == Ok; }
  constexpr bool is_err() const noexcept { return inner_.v.state == Err; }

  template <class As>
  constexpr const std::remove_reference_t<As>& get_ok() const noexcept {
    return inner_.v.u.ok;  //
  }
  constexpr const E& get_err() const noexcept {
    return inner_.v.u.err;  //
  }

  template <class As>
  constexpr As& get_ok_mut() & noexcept {
    return inner_.v.u.ok;  //
  }
  constexpr E& get_err_mut() & noexcept {
    return inner_.v.u.err;  //
  }

  template <class As>
  constexpr As take_ok() & noexcept
    requires(::sus::mem::Move<T>)
  {
    inner_.v.state = Moved;
    return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                         inner_.v.u.ok);
  }
  constexpr E take_err() & noexcept
    requires(::sus::mem::Move<E>)
  {
    inner_.v.state = Moved;
    return ::sus::mem::take_and_destruct(::sus::marker::unsafe_fn,
                                         inner_.v.u.err);
  }

  constexpr void drop_ok() & noexcept {
    inner_.v.state = Moved;  //
    std::destroy_at(&inner_.v.u.ok);
  }
  constexpr void drop_err() & noexcept {
    inner_.v.state = Moved;
    std::destroy_at(&inner_.v.u.err);
  }

 private:
 public:
  struct Inner {
    template <class U>
    constexpr Inner(WithT, U&& ok)
        : u(WITH_T, ::sus::forward<U>(ok)), state(Ok) {}
    template <class U>
    constexpr Inner(WithE, U&& err)
        : u(WITH_E, ::sus::forward<U>(err)), state(Err) {}

    ~Inner() noexcept
      requires(std::is_trivially_destructible_v<T> &&
               std::is_trivially_destructible_v<E>)
    = default;
    constexpr ~Inner() noexcept
      requires(!std::is_trivially_destructible_v<T> ||
               !std::is_trivially_destructible_v<E>)
    {
      switch (state) {
        case Ok: std::destroy_at(&u.ok); break;
        case Err: std::destroy_at(&u.err); break;
        case Moved: break;
      }
    }

    Inner(const Inner&)
      requires(std::is_trivially_copy_constructible_v<T> &&
               std::is_trivially_copy_constructible_v<E>)
    = default;
    constexpr Inner(const Inner& o)
      requires((!std::is_trivially_copy_constructible_v<T> ||
                !std::is_trivially_copy_constructible_v<E>) &&
               std::is_copy_constructible_v<T> &&
               std::is_copy_constructible_v<E>)
    {
      switch (o.state) {
        case Ok: std::construct_at(&u.ok, o.u.ok); break;
        case Err: std::construct_at(&u.err, o.u.err); break;
        case Moved: panic_with_message("Result used after move");
      }
      // After construct_at since it may write into the field if it's in tail
      // padding.
      state = o.state;
    }

    Inner& operator=(const Inner&)
      requires(std::is_trivially_copy_assignable_v<T> &&
               std::is_trivially_copy_assignable_v<E>)
    = default;
    constexpr Inner& operator=(const Inner& o)
      requires((!std::is_trivially_copy_assignable_v<T> ||
                !std::is_trivially_copy_assignable_v<E>) &&
               std::is_copy_assignable_v<T> && std::is_copy_assignable_v<E>)
    {
      check_with_message(o.state != Moved, "Result used after move");
      if (state == o.state) {
        switch (state) {
          case Ok: u.ok = o.u.ok; break;
          case Err: u.err = o.u.err; break;
          case Moved: break;
        }
      } else {
        switch (state) {
          case Ok: std::destroy_at(&u.ok); break;
          case Err: std::destroy_at(&u.err); break;
          case Moved: break;
        }
        // If this trips, it means the destructor in this Result moved out
        // of the Result that is being assigned from.
        check_with_message(o.state != Moved, "Result used after move");
        switch (o.state) {
          case Ok: std::construct_at(&u.ok, o.u.ok); break;
          case Err: std::construct_at(&u.err, o.u.err); break;
          case Moved: sus::unreachable_unchecked(::sus::marker::unsafe_fn);
        }
      }
      // After construct_at since it may write into the field if it's in tail
      // padding.
      state = o.state;
      return *this;
    }

    Inner(Inner&&)
      requires(std::is_trivially_move_constructible_v<T> &&
               std::is_trivially_move_constructible_v<E>)
    = default;
    constexpr Inner(Inner&& o)
      requires((!std::is_trivially_move_constructible_v<T> ||
                !std::is_trivially_move_constructible_v<E>) &&
               std::is_move_constructible_v<T> &&
               std::is_move_constructible_v<E>)
    {
      switch (o.state) {
        case Ok: std::construct_at(&u.ok, ::sus::move(o.u.ok)); break;
        case Err: std::construct_at(&u.err, ::sus::move(o.u.err)); break;
        case Moved: panic_with_message("Result used after move");
      }
      // After construct_at since it may write into the field if it's in tail
      // padding.
      state = ::sus::mem::replace(o.state, Moved);
    }

    Inner& operator=(Inner&&)
      requires(std::is_trivially_move_assignable_v<T> &&
               std::is_trivially_move_assignable_v<E>)
    = default;
    constexpr Inner& operator=(Inner&& o)
      requires((!std::is_trivially_move_assignable_v<T> ||
                !std::is_trivially_move_assignable_v<E>) &&
               std::is_move_assignable_v<T> && std::is_move_assignable_v<E>)
    {
      check_with_message(o.state != Moved, "Result used after move");
      if (state == o.state) {
        switch (state) {
          case Ok: u.ok = ::sus::move(o.u.ok); break;
          case Err: u.err = ::sus::move(o.u.err); break;
          case Moved: break;
        }
      } else {
        switch (state) {
          case Ok: std::destroy_at(&u.ok); break;
          case Err: std::destroy_at(&u.err); break;
          case Moved: break;
        }
        // If this trips, it means the destructor in this Result moved out
        // of the Result that is being assigned from.
        check_with_message(o.state != Moved, "Result used after move");
        switch (o.state) {
          case Ok:
            std::construct_at(&u.ok, ::sus::move(o.u.ok));
            std::destroy_at(&o.u.ok);
            break;
          case Err:
            std::construct_at(&u.err, ::sus::move(o.u.err));
            std::destroy_at(&o.u.err);
            break;
          case Moved: sus::unreachable_unchecked(::sus::marker::unsafe_fn);
        }
      }
      // After construct_at since it may write into the field if it's in tail
      // padding.
      state = ::sus::mem::replace(o.state, Moved);
      return *this;
    }

    union Union {
      constexpr Union() noexcept {}

      template <class U>
      constexpr Union(WithT, U&& ok) : ok(::sus::forward<U>(ok)) {}
      template <class U>
      constexpr Union(WithE, U&& err) : err(::sus::forward<U>(err)) {}

      ~Union() noexcept
        requires(std::is_trivially_destructible_v<T> &&
                 std::is_trivially_destructible_v<E>)
      = default;
      constexpr ~Union() noexcept
        requires(!std::is_trivially_destructible_v<T> ||
                 !std::is_trivially_destructible_v<E>)
      {}

      Union(const Union&)
        requires(std::is_trivially_copy_constructible_v<T> &&
                 std::is_trivially_copy_constructible_v<E>)
      = default;
      Union& operator=(const Union&)
        requires(std::is_trivially_copy_assignable_v<T> &&
                 std::is_trivially_copy_assignable_v<E>)
      = default;
      Union(Union&&)
        requires(std::is_trivially_move_constructible_v<T> &&
                 std::is_trivially_move_constructible_v<E>)
      = default;
      Union& operator=(Union&&)
        requires(std::is_trivially_move_assignable_v<T> &&
                 std::is_trivially_move_assignable_v<E>)
      = default;

      [[no_unique_address]] T ok;
      [[no_unique_address]] E err;
    };
    [[no_unique_address]] Union u;
    [[no_unique_address]] State state;
  };
  static constexpr bool union_cant_clobber_outside_inner =
      (::sus::mem::data_size_of<T>() == ::sus::mem::size_of<T>() &&
       ::sus::mem::data_size_of<E>() == ::sus::mem::size_of<E>());

  [[no_unique_address]] MaybeNoUniqueAddress<Inner,
                                             union_cant_clobber_outside_inner>
      inner_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(inner_.v.u.ok),
                                           decltype(inner_.v.u.err),
                                           decltype(inner_.v.state));
};

}  // namespace sus::result::__private
