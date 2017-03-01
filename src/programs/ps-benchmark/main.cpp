#include <iostream>
#include <ctime>       
#include <vector>
#include <random>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <pokerstove/penum/ShowdownEnumerator.h>
#include <pokerstove/peval/Card.h>
#include <pokerstove/peval/CardSet.h>
#include <pokerstove/peval/CardSetGenerators.h>
#include <pokerstove/peval/PokerHandEvaluator.h>
    
#include <chrono>
#include <utility>

using namespace pokerstove;

template<int N, int K, typename Rng>
inline uint64_t floydSample(Rng &&g) {
    std::uniform_int_distribution<> dist;
    using Bounds = std::uniform_int_distribution<>::param_type;
    uint64_t rv = 0;
    for(auto v = N - K; v < N; ++v) {
        auto draw = dist(g, Bounds(0, v));
        auto bit = uint64_t(1) << draw;
        if(bit & rv) {
            draw = v;
            bit = uint64_t(1) << draw;
        }
        rv |= bit;
    }
    return rv;
}

int total = 0;
uint64_t (*sampleGen)(std::mt19937 &generator) =
    [](std::mt19937 &generator) { return floydSample<52, 7>(generator); };
int (*pokerstove_evaluate)(uint64_t) = [](uint64_t cards) { return 0; }; //CardSet(cards).evaluateHigh().code(); };

void experiment(unsigned count, std::mt19937 &generator) {
    while(count--) {
        auto cards =// ep::floydSample<ep::NNumbers*ep::NSuits, 7>(generator);
            sampleGen(generator);
        total += pokerstove_evaluate(cards);
    }
};

template<typename Callable, typename... Args>
long benchmark(Callable &&call, Args &&... arguments) {
    auto now = std::chrono::high_resolution_clock::now();
    call(std::forward<Args>(arguments)...);
    auto end = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - now);
    return diff.count();
}

// int winner(const CardSet& community, const CardSet& p1, const CardSet& p2)
// {
// 	// PokerHandEvaluation first_eval = _peval->evaluate(p1, community);
// 	// PokerHandEvaluation second_eval = _peval->evaluate(p2, community);
// 	// return first_eval.high().code() - second_eval.high().code();
// 	return (community | p1).evaluateHigh().code() - (community | p2).evaluateHigh().code();
// }

int main(int argc, char** argv)
{
    auto count = 1 << 24;

    auto eval = [](uint64_t cards) { return CardSet(cards).evaluateHigh().code(); };

    std::random_device rd;
    std::mt19937 generator(rd());

    auto empty = benchmark(experiment, count, generator);

    pokerstove_evaluate = eval;

    auto nonempty = benchmark(experiment, count, generator);

    std::cout << empty << " " << nonempty << " " << std::endl;
    std::cout << "Micros per eval: " << 1.0*(nonempty - empty) / count << std::endl;
    std::cout << "Million evals per sec: " << 1.0*count / (nonempty - empty) << std::endl;
    std::cout << total << std::endl;
    return 0;
}

