#include "type_traits.hpp"


#define limpl(...) __VA_ARGS__

#	define lconstfn constexpr
#	define LB_PURE

#	define lnoexcept noexcept
#define lnoexcept_spec(...) lnoexcept(noexcept(__VA_ARGS__))

#	define lnothrow lnoexcept


#	define LB_STATELESS

#	define lthrow throw

#	define lnothrowv

namespace sad {
	using std::enable_if_t;

	template<typename _type, typename _type2>
	using equality_operator_t
		= decltype(std::declval<_type>() == std::declval<_type2>());

	template<typename _type, typename _type2 = _type>
	struct has_equality_operator : is_detected_convertible<bool,
		equality_operator_t, _type, _type2>
	{};

	namespace examiners
	{

		/*!
		\brief 基本等于操作检测。
		*/
		struct equal
		{
			template<typename _type1, typename _type2>
			static lconstfn LB_PURE auto
				are_equal(_type1&& x, _type2&& y)
				lnoexcept_spec(bool(x == y)) -> decltype(bool(x == y))
			{
				return bool(x == y);
			}
		};


		/*!
		\brief 基本等于操作检测：总是相等。
		*/
		struct always_equal
		{
			template<typename _type, typename _type2, limpl(typename
				= enable_if_t<!has_equality_operator<_type&&, _type2&&>::value>)>
				static lconstfn LB_STATELESS bool
				are_equal(_type&&, _type2&&) lnothrow
			{
				return true;
			}
		};


		/*!
		\brief 等于操作检测。
		*/
		struct equal_examiner : public equal, public always_equal
		{
			using equal::are_equal;
			using always_equal::are_equal;
		};

	} // namespace examiners;

	template<typename _type1, typename _type2>
	struct HeldEqual : private examiners::equal_examiner
	{
		using examiners::equal_examiner::are_equal;
	};

	template<typename _type1, typename _type2>
	lconstfn bool
		AreEqualHeld(const _type1& x, const _type2& y)
	{
		return HeldEqual<_type1, _type2>::are_equal(x, y);
	}

}

#include <iostream>
#include <fstream>

int main() {
	//error C2398->c3770->c2672->c2783
	std::cout<<sad::AreEqualHeld(std::ifstream(), std::ifstream())<<std::endl;
	std::cout << sad::AreEqualHeld(1, 2) << std::endl;
}