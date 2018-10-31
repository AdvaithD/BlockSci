//
//  proxy.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_hpp
#define proxy_hpp

#include "generic_sequence.hpp"

#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/transform.hpp>

#include <any>
#include <functional>

template<typename T>
struct Proxy;

template<typename T>
Proxy<T> makeProxy() {
	return std::function<T(std::any &)>{[](std::any &t) -> T {
		return std::any_cast<T>(t);
	}};
}

template<typename T>
struct GenericProxy {
	std::function<T(std::any &)> func;
	
	GenericProxy(const std::function<T(std::any &)> &func_) : func(func_) {}

	GenericProxy(const GenericProxy<T> & proxy) : func(proxy.func) {}

	GenericProxy(GenericProxy<T> && proxy) : func(std::move(proxy.func)) {}

	GenericProxy(const T &val) : GenericProxy(std::function<T(std::any &)>{[=](std::any &) -> T {
		return val;
	}}) {}

	T operator()(std::any &t) const {
		return func(t);
	}

	T operator()(std::any && t) const {
		return func(t);
	}
};

struct ProxyIterator {
	virtual std::function<any_view<std::any, ranges::category::input>(std::any &)> getGenericIterator() const = 0;
	virtual ~ProxyIterator() = default;

	std::function<any_view<std::any, ranges::category::input>(std::any &)> getGeneric() const {
		return getGenericIterator();
	}
};

struct ProxyRange : public ProxyIterator {
	virtual std::function<any_view<std::any, random_access_sized>(std::any &)> getGenericRange() const = 0;
	virtual ~ProxyRange() = default;

	std::function<any_view<std::any, ranges::category::input>(std::any &)> getGenericIterator() const override {
		return getGenericRange();
	}

	std::function<any_view<std::any, random_access_sized>(std::any &)> getGeneric() const {
		return getGenericRange();
	}
};

template <ranges::category range_cat>
struct ProxySequenceType;

template <>
struct ProxySequenceType<ranges::category::input> {
	using type = ProxyIterator;
};

template <>
struct ProxySequenceType<random_access_sized> {
	using type = ProxyRange;
};

template <ranges::category range_cat>
using proxy_sequence = typename ProxySequenceType<range_cat>::type;


struct ProxyAddress {
	virtual std::function<blocksci::AnyScript(std::any &)> getGenericScript() const = 0;
	virtual ~ProxyAddress() = default;

	std::function<blocksci::AnyScript(std::any &)> getGeneric() const {
		return getGenericScript();
	}
};

template<typename T>
struct Proxy : public GenericProxy<T> {
	using output_t = T;
	using GenericProxy<T>::GenericProxy;

	std::function<T(std::any &)> getGeneric() const {
		return this->func;
	}
};

template<typename T>
struct Proxy<ranges::optional<T>> : public GenericProxy<ranges::optional<T>> {
	using output_t = ranges::optional<T>;
	using GenericProxy<ranges::optional<T>>::GenericProxy;

	Proxy(const Proxy<T> &p) : Proxy(std::function<ranges::optional<T>(std::any &)>{[=](std::any &v) -> ranges::optional<T> {
		return p(v);
	}}) {}

	Proxy(const T &val) : Proxy(std::function<ranges::optional<T>(std::any &)>{[=](std::any &) -> ranges::optional<T> {
		return val;
	}}) {}

	std::function<ranges::optional<T>(std::any &)> getGeneric() const {
		return this->func;
	}
};


template<typename T>
struct Proxy<Iterator<T>> :  public GenericProxy<Iterator<T>>, public ProxyIterator {
	using output_t = Iterator<T>;
	using GenericProxy<Iterator<T>>::GenericProxy;

	std::function<Iterator<std::any>(std::any &)> getGenericIterator() const override {
		return [f = this->func](std::any &val) -> Iterator<std::any> {
			return ranges::view::transform(f(val), [](T && t) -> std::any {
				return std::move(t);
			});
		};
	}
};

template<typename T>
struct Proxy<Range<T>> : public GenericProxy<Range<T>>, public ProxyRange {
	using output_t = Range<T>;
	using GenericProxy<Range<T>>::GenericProxy;

	std::function<Range<std::any>(std::any &)> getGenericRange() const override {
		return [f = this->func](std::any &val) -> Range<std::any> {
			return ranges::view::transform(f(val), [](T && t) -> std::any {
				return std::move(t);
			});
		};
	}
};

template<blocksci::AddressType::Enum type>
struct Proxy<blocksci::ScriptAddress<type>> : public GenericProxy<blocksci::ScriptAddress<type>>, public ProxyAddress {
	using output_t = blocksci::ScriptAddress<type>;
	using GenericProxy<blocksci::ScriptAddress<type>>::GenericProxy;

	std::function<blocksci::AnyScript(std::any &)> getGenericScript() const override {
		return [f = this->func](std::any &val) -> blocksci::AnyScript {
			return blocksci::ScriptVariant{f(val)};
		};
	}
};

template<>
struct Proxy<blocksci::AnyScript> : public GenericProxy<blocksci::AnyScript>, public ProxyAddress {
	using output_t = blocksci::AnyScript;
	using GenericProxy<blocksci::AnyScript>::GenericProxy;

	std::function<blocksci::AnyScript(std::any &)> getGenericScript() const override {
		return func;
	}
};

#endif /* proxy_hpp */
