template <class Type, auto AutoValue, class TypeWithDefault = int,
          TypeWithDefault ValueOfDependentType = 90210, class... Pack>
  requires(AutoValue != ValueOfDependentType)
struct TemplateStruct {};
