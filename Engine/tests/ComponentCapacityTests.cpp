#include <catch2/catch_test_macros.hpp>
#include <ECS/Component.h>
#include <stdexcept>
#include <utility>


template<std::size_t Index> struct CapacityType {};
template<std::size_t... Index>
void RegisterCapacityTypes(std::index_sequence<Index...>)
{
    (static_cast<void>(Component<CapacityType<Index>>::GetID()), ...);
}
TEST_CASE("Component capacity fails before allocating an invalid ID")
{
    RegisterCapacityTypes(std::make_index_sequence<MAX_COMPONENTS>{});
    CHECK(Component<CapacityType<MAX_COMPONENTS - 1>>::GetID() ==
        static_cast<int>(MAX_COMPONENTS - 1));
    CHECK_THROWS_AS(Component<CapacityType<MAX_COMPONENTS>>::GetID(),
        std::runtime_error);
}