// #include <concepts>
// #include <utility>
// struct as
// {
//     template <class T> operator T()
//     {
//         std::unreachable();
//     };
//     template <class T> struct t
//     {
//         T &&x;
//         template <class U>
//         [[nodiscard]] constexpr decltype(static_cast<U>((T &&)x)) operator->*(U (as::*)()) noexcept(
//             noexcept(static_cast<U>((T &&)x)))
//         {
//             return static_cast<U>((T &&)x);
//         }
//     };
// };
// constexpr auto operator->*(auto &&x, std::same_as<as> auto) noexcept
// {
//     return as::t((decltype(x))x);
// }
// #define as ->*::as()->*&::as::operator

// static_assert(3.5 as int == 3);
// int main()
// {
// }