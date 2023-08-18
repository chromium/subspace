template <class Type, auto AutoValue, class TypeWithDefault = int,
          TypeWithDefault ValueOfDependentType = 90210, class... Pack>
  requires(AutoValue != ValueOfDependentType)
struct TemplateStruct {};

template <class T>
struct TemplateMethods {
  TemplateMethods();

  T template_params(T);

  template <class U>
  U local_template_params(T, U);

  template <class U>
  operator U();
};

template <class U>
U template_function(U);
