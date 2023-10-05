#pragma once

#include <type_traits>
#include <iostream>
#include <vector>

//template<bool condition, typename A, typename B>
//struct Ternary;
//
//template<typename A, typename B>
//struct Ternary<true, A, B>
//{
//	using type = A;
//};
//
//template<typename A, typename B>
//struct Ternary<false, A, B>
//{
//	using type = B;
//};

struct SuperBase {
	using type = void;
};

template<typename T>
struct Base :SuperBase {
	using type = T;
};

template<typename T>
struct Derived :Base<T> {};

template<typename T>
struct NotDerived {
	using type = T;
};

//template<typename T>
//struct UnWrap;
//
//template<template <typename> class  C, typename T>
//struct UnWrap<C<T>>;
//
//template<template <typename>typename  C, typename T>
//struct UnWrap<C<T>>
//{
//	using type = typename Ternary<std::is_base_of<Base<T>, C<T>>::value, T, C<T>>::type;
//};

template<typename T, typename DerivedFromSuper = void>
struct UnWrap
{
	using type = T;
};

template<typename T>
struct UnWrap<T, std::enable_if_t<std::is_base_of_v<SuperBase, T>>>
{
	using type = typename T::type;
};
template<typename T>
struct  Default : Base<T>
{
	static  T value;
};
template<typename T>
T Default<T>::value{};

struct MyDerived : Base<float>
{
	static constexpr float value = 1.0f;
};

int main()
{
	using namespace std;

	cout << typeid(UnWrap<Derived<int>>::type).name() << endl;
	cout << typeid(UnWrap<int>::type).name() << endl;
	cout << typeid(UnWrap<NotDerived<int>>::type).name() << endl;
	cout << typeid(UnWrap<Derived<vector<int>>>::type).name() << endl;
	cout << typeid(UnWrap<vector<int>>::type).name() << endl;
	cout << typeid(UnWrap<MyDerived>::type).name() << endl;
	cout << typeid(UnWrap<Default<vector<float>>>::type).name() << endl;
}