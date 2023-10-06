#pragma once
#include "LuaAux.hpp"
#include "FuncTraits.hpp"
namespace Lua
{
	template<auto fn, typename ...TUpvalues>
	struct CClosure
	{
		using FnType = decltype(fn);
		template<typename ...TArgs>
		using TReturn = typename std::invoke_result<FnType, TUpvalues&..., TArgs...>::type;

		template<typename ...TArgs>
		using ArgsTuple = std::tuple<TArgs...>;
		using Upvalues = std::tuple<TUpvalues...>;


	public:
		template<typename ...TArgs>
		static int Function(lua_State* l)
		{
			using namespace std;

			using Indexes = index_sequence_for<TArgs...>;
			static_assert(is_invocable<FnType, TUpvalues&..., TArgs...>::value, "Given function can't be called with such arguments!");
			using ArgsTupleT = ArgsTuple<TArgs...>;

			Upvalues upvalues;
			GetUpvalues<0, Upvalues, TUpvalues...>(l, upvalues);
			ArgsTupleT args;
			GetArgs<0, ArgsTupleT, TArgs ...>(l, args);
			if constexpr (is_void<TReturn<TArgs...>>::value)
			{
				CClosure::CallHelper(upvalues, args, index_sequence_for<TUpvalues...>{}, Indexes{});
				ReplaceUpvalues(l, upvalues);
				return 0;
			}
			else
			{
				TReturn<TArgs...> result = CClosure::CallHelper(upvalues, args, index_sequence_for<TUpvalues...>{}, Indexes{});
				ReplaceUpvalues(l, upvalues);
				size_t n_results = PushResult(l, result);
				return static_cast<int>(n_results);
			}
		}
	private:
		template <size_t ... UpIs, typename ...TArgs, size_t ... Is>
		inline static TReturn<TArgs...> CallHelper(Upvalues& upvalues, ArgsTuple<TArgs...>& args, std::index_sequence<UpIs...> const, std::index_sequence<Is...> const)
		{
			return fn(std::get<UpIs>(upvalues)..., std::get<Is>(args)...);
		}
	};

	template<auto fn>
	struct CFunction : public CClosure<fn> {
	};


	template<typename FnClass, typename ...TUpvalues>
	struct ClassClosure
	{
		template<typename ...TArgs>
		using TReturn = typename std::invoke_result<FnClass, TUpvalues&..., TArgs...>::type;

		template<typename ...TArgs>
		using ArgsTuple = std::tuple<TArgs...>;
		using Upvalues = std::tuple<TUpvalues...>;


	public:
		template<typename ...TArgs>
		static int Function(lua_State* l)
		{
			using namespace std;

			using Indexes = index_sequence_for<TArgs...>;
			static_assert(std::is_invocable<FnClass, TUpvalues&..., TArgs...>::value, "Given class doesnt provide callable or can't be called with such arguments!");
			using ArgsTupleT = ArgsTuple<TArgs...>;

			Upvalues upvalues;
			GetUpvalues<0, Upvalues, TUpvalues...>(l, upvalues);
			ArgsTupleT args;
			GetArgs<0, ArgsTupleT, TArgs ...>(l, args);
			if constexpr (is_void<TReturn<TArgs...>>::value)
			{
				ClassClosure::CallHelper(l, upvalues, args, index_sequence_for<TUpvalues...>{}, Indexes{});
				ReplaceUpvalues(l, upvalues);
				return 0;
			}
			else
			{
				TReturn<TArgs...> result = ClassClosure::CallHelper(l, upvalues, args, index_sequence_for<TUpvalues...>{}, Indexes{});
				ReplaceUpvalues(l, upvalues);
				size_t n_results = PushResult(l, result);
				return static_cast<int>(n_results);
			}
		}
	private:
		template <size_t ... UpIs, typename ...TArgs, size_t ... Is>
		inline static TReturn<TArgs...> CallHelper(lua_State* l, Upvalues& upvalues, ArgsTuple<TArgs...>& args, std::index_sequence<UpIs...> const, std::index_sequence<Is...> const)
		{

			if constexpr (std::is_constructible<FnClass, lua_State*>::value)
			{
				return FnClass{ l }(std::get<UpIs>(upvalues)..., std::get<Is>(args)...);
			}
			else
			{
				return FnClass{}(std::get<UpIs>(upvalues)..., std::get<Is>(args)...);
			}
		}
	};

	template<typename FnClass>
	struct ClassFunction : public ClassClosure<FnClass> {
	};

	namespace FuncUtility
	{
		template<typename T>
		struct IncrementArgIndex
		{
			static  constexpr size_t value = 1;
		};

		template<typename T>
		struct IncrementArgIndex<Upvalue<T>>
		{
			static  constexpr size_t value = 0;
		};

		template<typename T>
		struct IncrementUpvalueIndex
		{
			static  constexpr size_t value = 0;
		};

		template<typename T>
		struct IncrementUpvalueIndex<Upvalue<T>>
		{
			static constexpr size_t value = 1;
		};

		template<typename T>
		struct IsUpvalueType
		{
			static constexpr bool value = IncrementUpvalueIndex<T>::value == 1;
		};

		template<typename T, typename Optional = void>
		struct ArgExtractor
		{
			template<size_t ArgI, size_t UpvalueI>
			static 	constexpr T Get(lua_State* l)
			{
				return TypeParser<T>::Get(l, ArgI + 1);
			}
		};

		template<typename T>
		struct ArgExtractor<Default<T>>
		{
			template<size_t ArgI, size_t UpvalueI>
			static 	constexpr T Get(lua_State* l)
			{
				if (TypeParser<T>::Check(l, ArgI + 1))
					return TypeParser<T>::Get(l, ArgI + 1);
				return Default<T>::value;
			}
		};

		template<typename T>
		struct ArgExtractor<Upvalue<T>>
		{
			template<size_t ArgI, size_t UpvalueI>
			static constexpr T Get(lua_State* l)
			{
				return TypeParser<T>::Get(l, lua_upvalueindex((int)UpvalueI + 1));
			}
		};


		template<typename T>
		struct ArgExtractor<T, std::enable_if_t<std::is_base_of_v<OptionalArg, T>>>
		{
			using ReturnT = typename T::type;
			template<size_t ArgI, size_t UpvalueI>
			static constexpr ReturnT Get(lua_State* l)
			{
				if (TypeParser<ReturnT>::Check(l, ArgI + 1))
					return TypeParser<ReturnT>::Get(l, ArgI + 1);
				return T::value;
			}
		};

		template<typename T>
		struct UpvalueReplacer
		{
			template<size_t UpvalueI>
			static  constexpr void Replace(lua_State* l, const T& value)
			{
				if constexpr (!std::is_pointer<T>::value)
				{
					TypeParser<T>::Push(l, value);
					lua_replace(l, lua_upvalueindex((int)UpvalueI + 1));
				}
			}
		};
		template<size_t ArgIndex, size_t UpvalueIndex, typename TArgsTuple>
		constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
		{
			return ArgIndex + UpvalueIndex;
		}

		template<size_t ArgIndex, size_t UpvalueIndex, typename TArgsTuple, typename TArg, typename ...TArgs>
		constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
		{
			std::get<ArgIndex + UpvalueIndex>(args) = ArgExtractor<TArg>::Get<ArgIndex, UpvalueIndex>(l);
			return GetArgs<
				ArgIndex + IncrementArgIndex<TArg>::value,
				UpvalueIndex + IncrementUpvalueIndex<TArg>::value,
				TArgsTuple, TArgs...>(l, args);
		}

		template<typename TArgsTuple, typename ...TArgs>
		constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
		{
			return GetArgs<0, 0, TArgsTuple, TArgs...>(l, args);
		}

		template<size_t ArgIndex, size_t UpvalueIndex, typename TArgsTuple>
		constexpr size_t ReplaceUpvalues(lua_State* l, TArgsTuple& args)
		{
			return ArgIndex + UpvalueIndex;
		}

		template<size_t ArgIndex, size_t UpvalueIndex, typename TArgsTuple, typename T, typename ...Ts>
		constexpr size_t ReplaceUpvalues(lua_State* l, TArgsTuple& args)
		{
			if constexpr (IsUpvalueType<T>::value)
				UpvalueReplacer<typename T::type>::Replace<UpvalueIndex>(l, std::get<ArgIndex + UpvalueIndex>(args));
			return ReplaceUpvalues<
				ArgIndex + IncrementArgIndex<T>::value,
				UpvalueIndex + IncrementUpvalueIndex<T>::value,
				TArgsTuple, Ts...>(l, args);
		}

		template<typename TArgsTuple, typename ...Ts>
		constexpr size_t ReplaceUpvalues(lua_State* l, TArgsTuple& args)
		{
			return ReplaceUpvalues<0, 0, TArgsTuple, Ts...>(l, args);
		}

	}

	template<auto fn, typename ...TArgs>
	struct Closure
	{
		using FnType = decltype(fn);
		static_assert(std::is_invocable<FnType, UnWrap_t< TArgs>&...>::value, "Given function can't be called with such arguments!");
		using TReturn = std::invoke_result_t<FnType, UnWrap_t<TArgs>&...>;

		using ArgsTuple = std::tuple<UnWrap_t<TArgs>...>;
		using Indexes = std::index_sequence_for<TArgs...>;

	public:
		static int Function(lua_State* l)
		{
			ArgsTuple args;
			Closure::GetArgs(l, args);
			if constexpr (std::is_void<TReturn>::value)
			{
				Closure::CallHelper(args, Indexes{});
				Closure::ReplaceUpvalues(l, args);
				return 0;
			}
			else
			{
				TReturn result = Closure::CallHelper(args, Indexes{});
				Closure::ReplaceUpvalues(l, args);
				size_t n_results = PushResult(l, result);
				return static_cast<int>(n_results);
			}
		}
	private:
		static constexpr size_t GetArgs(lua_State* l, ArgsTuple& args)
		{
			return FuncUtility::GetArgs<0, 0, ArgsTuple, TArgs...>(l, args);
		}

		static  constexpr size_t ReplaceUpvalues(lua_State* l, ArgsTuple& args)
		{
			return FuncUtility::ReplaceUpvalues<0, 0, ArgsTuple, TArgs...>(l, args);
		}

		template <size_t ... Is>
		inline static TReturn CallHelper(ArgsTuple& args, std::index_sequence<Is...> const)
		{
			return fn(std::get<Is>(args)...);
		}
	};
}