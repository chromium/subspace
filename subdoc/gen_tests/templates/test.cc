template <class Type, auto AutoValue, class TypeWithDefault = int,
          TypeWithDefault ValueOfDependentType = 90210, class... Pack>
  requires(AutoValue != ValueOfDependentType)
struct TemplateStruct {};

template <class T, class... U>
concept Concept = true;

template <class T>
struct S {};

template <class T>
struct TemplateMethods {
  TemplateMethods();

  T template_params(T t)
    requires(Concept<T>);

  template <Concept U>
  U local_template_params(T t, U u);

  Concept<S> auto concept_return();

  void concept_param(Concept auto var);

  template <class A, class B>
    requires(Concept<A, B>)
  void requires_func();

  template <class U>
  operator U();

  template <class U>
  static U member;

  S<S<int>> template_field;
};

// Returns template parameter.
template <class U>
U template_function(U);

/// Returns template instantiation.
S<int> return_template();

/// Has two overloads.
template <class A, class B>
void requires_overload(A, B) 
  requires(Concept<A, B>);
template <class A, class B>
void requires_overload(A, B) 
  requires(!Concept<A, B>);
