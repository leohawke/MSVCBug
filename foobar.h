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
		\brief ���죺ʹ�ú���ָ�롣
		*/
		lconstfn
			GHEvent(FuncType* f = {})
			: BaseType(f)
		{}
		/*!
		\brief ʹ�ú�������
		*/
		template<class _fCallable>
		lconstfn
			GHEvent(_fCallable f, enable_if_t<
				std::is_constructible<BaseType, _fCallable>::value, int> = 0)
			: BaseType(f)
		{}
		/*!
		\brief ʹ����չ��������
		\todo �ƶϱȽ���Ȳ�����
		*/
		template<class _fCallable>
		lconstfn
			GHEvent(_fCallable&& f, enable_if_t<
				!std::is_constructible<BaseType, _fCallable>::value, int> = 0)
			: BaseType(make_expanded<_tRet(_tParams...)>(lforward(f)))
		{}
		/*!
		\brief ���죺ʹ�ö������úͳ�Ա����ָ�롣
		\warning ʹ�ÿճ�Աָ�빹��ĺ��������������δ������Ϊ��
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
	\brief ��ʽ�����Ĵ�������
	*/
	class FormContextHandler
	{
	public:
		ContextHandler Handler;
		/*!
		\brief �������̣���֤����װ�Ĵ������ĵ��÷���ǰ��������
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
			\brief ����һ����ʽ��
			\exception LSLException �쳣������
			\throw LoggedEvent ���棺���Ͳ�ƥ�䣬
			�� Handler �׳��� bad_any_cast ת����
			\throw LoggedEvent ������ Handler �׳��� bad_any_cast ���
			std::exception ת����
			\throw std::invalid_argument ����δͨ����

			���鲻���ڻ��ڼ��ͨ���󣬶Խڵ���� Hanlder �������׳��쳣��
			*/
			ReductionStatus
				operator()(ValueNode&, ValueNode&) const {
				return ReductionStatus::Clean;
			}
	};


	/*!
	\brief �ϸ������Ĵ�������
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
			\brief ��������
			\throw ListReductionFailure �б��������һ�
			\sa ReduceArguments

			��ÿһ��������ֵ��Ȼ�����������Կɵ��õ������ Hanlder �������׳��쳣��
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
	\note ʹ�� ADL ToContextHandler ��
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
