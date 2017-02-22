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

using namespace pokerstove;

int winner(const CardSet& community, const CardSet& p1, const CardSet& p2)
{
	// PokerHandEvaluation first_eval = _peval->evaluate(p1, community);
	// PokerHandEvaluation second_eval = _peval->evaluate(p2, community);
	// return first_eval.high().code() - second_eval.high().code();
	return (community | p1).evaluateHigh().code() - (community | p2).evaluateHigh().code();
}

int main()
{
	//boost::shared_ptr<PokerHandEvaluator> _peval(PokerHandEvaluator::alloc ("h"));

    std::vector<int> v(52);
    std::iota(v.begin(), v.end(), 0);
 
    std::random_device rd;
    std::mt19937 g(rd());
 
    constexpr int num_iter = 1000000;

    int value = 0;

    clock_t begin = clock();

    for(auto i=0; i<num_iter; ++i)
    {
	    std::shuffle(v.begin(), v.end(), g);
	
    	CardSet p1 = CardSet(Card(v[0])) | CardSet(Card(v[1]));
		CardSet p2 = CardSet(Card(v[2])) | CardSet(Card(v[3]));
		CardSet community = CardSet(Card(v[4])) | CardSet(Card(v[5])) | CardSet(Card(v[6])) | CardSet(Card(v[7])) | CardSet(Card(v[8]));
		    
		//std::cout << p1.str() << " " << p2.str() << " " << community.str() << std::endl;
		//std::cout << winner(community, p1, p2) << std::endl;
	    value = winner(community, p1, p2);
    }

    clock_t end = clock();
    std::cout << "Number of iterations: " << num_iter << std::endl;
    std::cout << "Total time: " << double(end - begin) / CLOCKS_PER_SEC << "s" << std::endl;


}

