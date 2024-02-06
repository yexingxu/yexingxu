

#pragma once

#include "shm/algorithm.hpp"

namespace shm {
namespace details {

template <typename, typename>
// AXIVION Next Construct AutosarC++19_03-A12.0.1 : Not required since a
// default'ed destructor does not define a destructor, hence the copy/move
// operations are not deleted. The only adaptation is that the dtor is protected
// to prohibit the user deleting the child type by explicitly calling the
// destructor of the base type. Additionally, this is a marker struct that adds
// only the described property to the new type. Adding copy/move operations
// would contradict the purpose.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,
// hicpp-special-member-functions)
struct CopyAssignable {
 protected:
  ~CopyAssignable() = default;
};

template <typename, typename>
// AXIVION Next Construct AutosarC++19_03-A12.0.1 : Not required since a
// default'ed destructor does not define a destructor, hence the copy/move
// operations are not deleted. The only adaptation is that the dtor is protected
// to prohibit the user deleting the child type by explicitly calling the
// destructor of the base type. Additionally, this is a marker struct that adds
// only the described property to the new type. Adding copy/move operations
// would contradict the purpose.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,
// hicpp-special-member-functions)
struct MoveAssignable {
 protected:
  ~MoveAssignable() = default;
};

template <typename, typename>
// AXIVION Next Construct AutosarC++19_03-A12.0.1 : Not required since a
// default'ed destructor does not define a destructor, hence the copy/move
// operations are not deleted. The only adaptation is that the dtor is protected
// to prohibit the user deleting the child type by explicitly calling the
// destructor of the base type. Additionally, this is a marker struct that adds
// only the described property to the new type. Adding copy/move operations
// would contradict the purpose.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,
// hicpp-special-member-functions)
struct AssignByValueCopy {
 protected:
  ~AssignByValueCopy() = default;
};

template <typename, typename>
// AXIVION Next Construct AutosarC++19_03-A12.0.1 : Not required since a
// default'ed destructor does not define a destructor, hence the copy/move
// operations are not deleted. The only adaptation is that the dtor is protected
// to prohibit the user deleting the child type by explicitly calling the
// destructor of the base type. Additionally, this is a marker struct that adds
// only the described property to the new type. Adding copy/move operations
// would contradict the purpose.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,
// hicpp-special-member-functions)
struct AssignByValueMove {
 protected:
  ~AssignByValueMove() = default;
};

template <typename, typename>
// AXIVION Next Construct AutosarC++19_03-A12.0.1 : Not required since a
// default'ed destructor does not define a destructor, hence the copy/move
// operations are not deleted. The only adaptation is that the dtor is protected
// to prohibit the user deleting the child type by explicitly calling the
// destructor of the base type. Additionally, this is a marker struct that adds
// only the described property to the new type. Adding copy/move operations
// would contradict the purpose.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,
// hicpp-special-member-functions)
struct ProtectedConstructByValueCopy {
 protected:
  ~ProtectedConstructByValueCopy() = default;
};

template <typename, typename>
// AXIVION Next Construct AutosarC++19_03-A12.0.1 : Not required since a
// default'ed destructor does not define a destructor, hence the copy/move
// operations are not deleted. The only adaptation is that the dtor is protected
// to prohibit the user deleting the child type by explicitly calling the
// destructor of the base type. Additionally, this is a marker struct that adds
// only the described property to the new type. Adding copy/move operations
// would contradict the purpose.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,
// hicpp-special-member-functions)
struct CopyConstructable {
 protected:
  ~CopyConstructable() = default;
};

template <typename, typename>
// AXIVION Next Construct AutosarC++19_03-A12.0.1 : Not required since a
// default'ed destructor does not define a destructor, hence the copy/move
// operations are not deleted. The only adaptation is that the dtor is protected
// to prohibit the user deleting the child type by explicitly calling the
// destructor of the base type. Additionally, this is a marker struct that adds
// only the described property to the new type. Adding copy/move operations
// would contradict the purpose.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,
// hicpp-special-member-functions)
struct MoveConstructable {
 protected:
  ~MoveConstructable() = default;
};

template <typename, typename>
// AXIVION Next Construct AutosarC++19_03-A12.0.1 : Not required since a
// default'ed destructor does not define a destructor, hence the copy/move
// operations are not deleted. The only adaptation is that the dtor is protected
// to prohibit the user deleting the child type by explicitly calling the
// destructor of the base type. Additionally, this is a marker struct that adds
// only the described property to the new type. Adding copy/move operations
// would contradict the purpose.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,
// hicpp-special-member-functions)
struct ConstructByValueCopy {
 protected:
  ~ConstructByValueCopy() = default;
};

template <typename, typename>
// AXIVION Next Construct AutosarC++19_03-A12.0.1 : Not required since a
// default'ed destructor does not define a destructor, hence the copy/move
// operations are not deleted. The only adaptation is that the dtor is protected
// to prohibit the user deleting the child type by explicitly calling the
// destructor of the base type. Additionally, this is a marker struct that adds
// only the described property to the new type. Adding copy/move operations
// would contradict the purpose.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,
// hicpp-special-member-functions)
struct DefaultConstructable {
 protected:
  ~DefaultConstructable() = default;
};

template <typename, typename>
// AXIVION Next Construct AutosarC++19_03-A12.0.1 : Not required since a
// default'ed destructor does not define a destructor, hence the copy/move
// operations are not deleted. The only adaptation is that the dtor is protected
// to prohibit the user deleting the child type by explicitly calling the
// destructor of the base type. Additionally, this is a marker struct that adds
// only the described property to the new type. Adding copy/move operations
// would contradict the purpose.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,
// hicpp-special-member-functions)
struct Convertable {
 protected:
  ~Convertable() = default;
};

struct ProtectedConstructor_t {};

static constexpr ProtectedConstructor_t ProtectedConstructor{
    ProtectedConstructor_t()};

template <typename T>
inline typename T::value_type newTypeAccessor(const T& b) noexcept {
  return b.m_value;
}

template <typename T>
inline typename T::value_type& newTypeRefAccessor(T& b) noexcept {
  return b.m_value;
}

template <typename, typename T>
// AXIVION Next Construct AutosarC++19_03-A12.0.1 : Not required since a
// default'ed destructor does not define a destructor, hence the copy/move
// operations are not deleted. The only adaptation is that the dtor is protected
// to prohibit the user deleting the child type by explicitly calling the
// destructor of the base type. Additionally, this is a marker struct that adds
// only the described property to the new type. Adding copy/move operations
// would contradict the purpose.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,
// hicpp-special-member-functions)
struct Sortable {
  friend bool operator<=(const T& lhs, const T& rhs) noexcept {
    return newTypeAccessor(lhs) <= newTypeAccessor(rhs);
  }

  friend bool operator<(const T& lhs, const T& rhs) noexcept {
    return newTypeAccessor(lhs) < newTypeAccessor(rhs);
  }

  friend bool operator>(const T& lhs, const T& rhs) noexcept {
    return newTypeAccessor(lhs) > newTypeAccessor(rhs);
  }

  friend bool operator>=(const T& lhs, const T& rhs) noexcept {
    return newTypeAccessor(lhs) >= newTypeAccessor(rhs);
  }

 protected:
  ~Sortable() = default;
};

template <typename, typename T>
// AXIVION Next Construct AutosarC++19_03-A12.0.1 : Not required since a
// default'ed destructor does not define a destructor, hence the copy/move
// operations are not deleted. The only adaptation is that the dtor is protected
// to prohibit the user deleting the child type by explicitly calling the
// destructor of the base type. Additionally, this is a marker struct that adds
// only the described property to the new type. Adding copy/move operations
// would contradict the purpose.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions,
// hicpp-special-member-functions)
struct Comparable {
  friend bool operator==(const T& lhs, const T& rhs) noexcept {
    return newTypeAccessor(lhs) == newTypeAccessor(rhs);
  }

  friend bool operator!=(const T& lhs, const T& rhs) noexcept {
    return !(lhs == rhs);
  }

 protected:
  ~Comparable() = default;
};

template <typename Derived, typename T,
          template <typename, typename> class... Policies>
class NewType : public Policies<Derived, NewType<Derived, T, Policies...>>... {
 protected:
  /// 'ProtectedConstructor_t' is a compile time variable to select the correct
  /// constructors
  /// @NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter)
  NewType(ProtectedConstructor_t, const T& rhs) noexcept;

  /// @brief copy constructor
  NewType(const NewType& rhs) noexcept;

  /// @brief move constructor
  NewType(NewType&& rhs) noexcept;

  /// @brief copy assignment
  NewType& operator=(const NewType& rhs) noexcept;

  /// @brief move assignment
  NewType& operator=(NewType&& rhs) noexcept;

  /// @note Since 'using Foo = NewType<int>' and 'using Bar = NewType<int>'
  /// result in 'Foo' and 'Bar' being the same type, this enforces the creation
  /// of the new type by inheritance
  ~NewType() = default;

 public:
  /// @brief the type of *this
  using ThisType = NewType<Derived, T, Policies...>;
  /// @brief the type of the underlying value
  using value_type = T;

  /// @brief default constructor
  NewType() noexcept;

  /// @brief construct with value copy
  explicit NewType(const T& rhs) noexcept;

  /// @brief copy by value assignment
  NewType& operator=(const T& rhs) noexcept;

  /// @brief copy by value assignment
  NewType& operator=(T&& rhs) noexcept;

  /// @brief conversion operator
  // AXIVION Next Construct AutosarC++19_03-A13.5.3 : needed to provide
  // convertable policy so that the derived type can be convertable
  explicit operator T() const noexcept;

  template <typename Type>
  friend typename Type::value_type newTypeAccessor(const Type&) noexcept;

  template <typename Type>
  friend typename Type::value_type& newTypeRefAccessor(Type&) noexcept;

 private:
  T m_value;
};

template <typename Derived, typename T,
          template <typename, typename> class... Policies>
// AXIVION Next Construct AutosarC++19_03-A12.6.1 : m_value is initialized by
// the default constructor of T; the code will not compile, if the default
// constructor of T does not exist or the DefaultConstructable policy is not
// added
inline NewType<Derived, T, Policies...>::NewType() noexcept {
  static_assert(doesContainType<DefaultConstructable<Derived, T>,
                                Policies<Derived, T>...>(),
                "This type is not default constructable, please add the "
                "DefaultConstructable policy.");
}

template <typename Derived, typename T,
          template <typename, typename> class... Policies>
// AXIVION Next Construct AutosarC++19_03-A12.1.5 : delegating wanted only to
// "ProtectedConstructByValueCopy" constructor of T
inline NewType<Derived, T, Policies...>::NewType(ProtectedConstructor_t,
                                                 const T& rhs) noexcept
    : m_value(rhs) {
  static_assert(doesContainType<ProtectedConstructByValueCopy<Derived, T>,
                                Policies<Derived, T>...>(),
                "This type is not protected constructable with value copy, "
                "please add the "
                "ProtectedConstructByValueCopy policy.");
}

template <typename Derived, typename T,
          template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>::NewType(const T& rhs) noexcept
    : m_value(rhs) {
  static_assert(
      doesContainType<ConstructByValueCopy<Derived, T>,
                      Policies<Derived, T>...>(),
      "This type is not constructable with value copy, please add the "
      "ConstructByValueCopy policy.");
}

template <typename Derived, typename T,
          template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>::NewType(const NewType& rhs) noexcept
    : m_value(rhs.m_value) {
  static_assert(
      doesContainType<CopyConstructable<Derived, T>, Policies<Derived, T>...>(),
      "This type is not copy constructable, please add the "
      "CopyConstructable policy.");
}

template <typename Derived, typename T,
          template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>::NewType(NewType&& rhs) noexcept
    : m_value(std::move(rhs.m_value)) {
  static_assert(
      doesContainType<MoveConstructable<Derived, T>, Policies<Derived, T>...>(),
      "This type is not move constructable, please add the "
      "MoveConstructable policy.");
}

template <typename Derived, typename T,
          template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>&
NewType<Derived, T, Policies...>::operator=(const NewType& rhs) noexcept {
  if (this != &rhs) {
    m_value = rhs.m_value;
    static_assert(
        doesContainType<CopyAssignable<Derived, T>, Policies<Derived, T>...>(),
        "This type is not copy assignable, please add the "
        "CopyAssignable policy.");
  }
  return *this;
}

template <typename Derived, typename T,
          template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>&
NewType<Derived, T, Policies...>::operator=(NewType&& rhs) noexcept {
  if (this != &rhs) {
    m_value = std::move(rhs.m_value);
  }
  static_assert(
      doesContainType<MoveAssignable<Derived, T>, Policies<Derived, T>...>(),
      "This type is not move assignable, please add the "
      "MoveAssignable policy.");
  return *this;
}

template <typename Derived, typename T,
          template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>&
NewType<Derived, T, Policies...>::operator=(const T& rhs) noexcept {
  m_value = rhs;
  static_assert(
      doesContainType<AssignByValueCopy<Derived, T>, Policies<Derived, T>...>(),
      "This type is not assignable by value copy, please add the "
      "AssignByValueCopy policy.");
  return *this;
}

template <typename Derived, typename T,
          template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>&
NewType<Derived, T, Policies...>::operator=(T&& rhs) noexcept {
  m_value = std::move(rhs);
  static_assert(
      doesContainType<AssignByValueMove<Derived, T>, Policies<Derived, T>...>(),
      "This type is not assignable by value move, please add the "
      "AssignByValueMove policy.");
  return *this;
}

template <typename Derived, typename T,
          template <typename, typename> class... Policies>
inline NewType<Derived, T, Policies...>::operator T() const noexcept {
  static_assert(
      doesContainType<Convertable<Derived, T>, Policies<Derived, T>...>(),
      "This type is not convertable, please add the "
      "Convertable policy.");
  return m_value;
}

}  // namespace details
}  // namespace shm