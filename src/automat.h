
#pragma once

#include <utility>

namespace atmt {

template<typename ElmT, typename InElmT, typename OutElmT>
class Automat {
 protected:
	ElmT elm_;
	InElmT last_in_;
	OutElmT last_out_;

 public:
	using In = InElmT;
	using Elm = ElmT;
	using Out = OutElmT;
	using RefElm = std::reference_wrapper<ElmT>;
	using RefOut = std::reference_wrapper<OutElmT>;

	void Next(const In& in) {}

	void SetElm(const Elm& elm) {
		elm_ = elm;
	}

	const RefElm GetElm() {
		return elm_;
	}
};

} // namespace atmt
