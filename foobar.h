#ifndef FOOBAR_H
#define FOOBAR_H 1

#include "functional.hpp"
#include <string>
namespace sad {
	using string = std::string;

	template<class... _Types>
	using tuple = std::tuple<_Types...>;

	template<typename>
	class GHEvent;

	template<typename _tRet, typename... _tParams>
	class GHEvent<_tRet(_tParams...)>
		: protected std::function<_tRet(_tParams...)>
	{
	public:
		using TupleType = tuple<_tParams...>;
		using FuncType = _tRet(_tParams...);
		using BaseType = std::function<FuncType>;

	public:
		/*!
		\brief 构造：使用函数指针。
		*/
		lconstfn
			GHEvent(FuncType* f = {})
			: BaseType(f)
		{}
		/*!
		\brief 使用函数对象。
		*/
		template<class _fCallable>
		lconstfn
			GHEvent(_fCallable f, enable_if_t<
				std::is_constructible<BaseType, _fCallable>::value, int> = 0)
			: BaseType(f)
		{}
		/*!
		\brief 使用扩展函数对象。
		\todo 推断比较相等操作。
		*/
		template<class _fCallable>
		lconstfn
			GHEvent(_fCallable&& f, enable_if_t<
				!std::is_constructible<BaseType, _fCallable>::value, int> = 0)
			: BaseType(make_expanded<_tRet(_tParams...)>(lforward(f)))
		{}
		/*!
		\brief 构造：使用对象引用和成员函数指针。
		\warning 使用空成员指针构造的函数对象调用引起未定义行为。
		*/
		template<class _type>
		lconstfn
			GHEvent(_type& obj, _tRet(_type::*pm)(_tParams...))
			: GHEvent([&, pm](_tParams... args) lnoexcept(
				noexcept((obj.*pm)(lforward(args)...))
				&& std::is_nothrow_copy_constructible<_tRet>::value) {
			return (obj.*pm)(lforward(args)...);
		})
		{}

		using BaseType::operator();

		using BaseType::operator bool;

		using BaseType::target;

		using BaseType::target_type;
	};

	enum class ReductionStatus : limpl(size_t)
	{
		Clean = 0,
			Retained,
			Retrying
	};

	class ValueNode
	{};

	using ContextHandler = GHEvent<ReductionStatus(ValueNode&, ValueNode&)>;


	template<typename _func>
	struct WrappedContextHandler
	{
		_func Handler;

		//@{
		template<typename _tParam, limpl(typename
			= exclude_self_t<WrappedContextHandler, _tParam>)>
			WrappedContextHandler(_tParam&& arg)
			: Handler(lforward(arg))
		{}
		template<typename _tParam1, typename _tParam2, typename... _tParams>
		WrappedContextHandler(_tParam1&& arg1, _tParam2&& arg2, _tParams&&... args)
			: Handler(lforward(arg1), lforward(arg2), lforward(args)...)
		{}


			template<typename... _tParams>
		ReductionStatus
			operator()(_tParams&&... args) const
		{
			Handler(lforward(args)...);
			return ReductionStatus::Clean;
		}
	};

	template<class _tDst, typename _func>
	inline _tDst
		WrapContextHandler(_func&& h, std::false_type)
	{
		return WrappedContextHandler<GHEvent<make_function_type_t<
			void, make_parameter_tuple_t<typename _tDst::BaseType>>>>(
				lforward(h));
	}
	template<class, typename _func>
	inline _func
		WrapContextHandler(_func&& h, std::true_type)
	{
		return lforward(h);
	}
	template<class _tDst, typename _func>
	inline _tDst
		WrapContextHandler(_func&& h)
	{
		using BaseType = typename _tDst::BaseType;

		return WrapContextHandler<_tDst>(lforward(h), or_<
			std::is_constructible<BaseType, _func&&>,
			std::is_constructible<BaseType, expanded_caller<
			typename _tDst::FuncType, decay_t<_func>>>>());
	}
	//@}

	/*!
	\brief 形式上下文处理器。
	*/
	class FormContextHandler
	{
	public:
		ContextHandler Handler;
		/*!
		\brief 项检查例程：验证被包装的处理器的调用符合前置条件。
		*/
		std::function<bool(const ValueNode&)> Check{ };

		template<typename _func,
			limpl(typename = exclude_self_t<FormContextHandler, _func>)>
			FormContextHandler(_func&& f)
			: Handler(WrapContextHandler<ContextHandler>(lforward(f)))
		{}
		template<typename _func, typename _fCheck>
		FormContextHandler(_func&& f, _fCheck c)
			: Handler(WrapContextHandler<ContextHandler>(lforward(f))), Check(c)
		{}

			/*!
			\brief 处理一般形式。
			\exception LSLException 异常中立。
			\throw LoggedEvent 警告：类型不匹配，
			由 Handler 抛出的 bad_any_cast 转换。
			\throw LoggedEvent 错误：由 Handler 抛出的 bad_any_cast 外的
			std::exception 转换。
			\throw std::invalid_argument 项检查未通过。

			项检查不存在或在检查通过后，对节点调用 Hanlder ，否则抛出异常。
			*/
			ReductionStatus
				operator()(ValueNode&, ValueNode&) const {
				return ReductionStatus::Clean;
			}
	};


	/*!
	\brief 严格上下文处理器。
	*/
	class StrictContextHandler
	{
	public:
		FormContextHandler Handler;

		template<typename _func,
			limpl(typename = exclude_self_t<StrictContextHandler, _func>)>
			StrictContextHandler(_func&& f)
			: Handler(lforward(f))
		{}
		template<typename _func, typename _fCheck>
		StrictContextHandler(_func&& f, _fCheck c)
			: Handler(lforward(f), c)
		{}


			/*!
			\brief 处理函数。
			\throw ListReductionFailure 列表子项不大于一项。
			\sa ReduceArguments

			对每一个子项求值；然后检查项数，对可调用的项调用 Hanlder ，否则抛出异常。
			*/
			ReductionStatus
				operator()(ValueNode&, ValueNode&) const {
				return ReductionStatus::Clean;
			}
	};


	//@{
	template<typename... _tParams>
	inline void
		RegisterForm(ValueNode& node, const string& name,
			_tParams&&... args)
	{
		limpl(
			FormContextHandler(lforward(args)...));
	}

	template<typename... _tParams>
	inline ContextHandler
		ToContextHandler(_tParams&&... args)
	{
		return StrictContextHandler(lforward(args)...);
	}

	/*!
	\note 使用 ADL ToContextHandler 。
	*/
	template<typename... _tParams>
	inline void
		RegisterStrict(ValueNode& node, const string& name, _tParams&&... args)
	{
		limpl(
			ToContextHandler(lforward(args)...));
	}

	template<typename _type, typename _func, typename... _tParams>
	void
		CallBinaryFold(_func f, _type val, ValueNode& term, _tParams&&... args)
	{
	}

}

#endif
