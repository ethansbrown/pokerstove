#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <cstdlib>
#include <ctime>        
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <pokerstove/peval/PokerHandEvaluator.h>
#include <pokerstove/peval/Card.h>
#include <pokerstove/peval/CardSet.h>
#include <pokerstove/peval/CardSetGenerators.h>

using namespace std;
namespace po = boost::program_options;
using namespace pokerstove;

class SimDriver
{
public:
    SimDriver(const string& game, 
               const vector<string>& hands, 
               const string& board
			   )
        : _peval(PokerHandEvaluator::alloc (game))
        , _hands(hands)
        , _board(board)
		, _board_cs(board)
    {
        srand(time(NULL));
        predefined_ranges = create_predefined_ranges();

        initialize_ranges();

        for (auto it0=_ranges[0].begin(); it0!=_ranges[0].end(); it0++)
        {
            for (auto it1=_ranges[1].begin(); it1!=_ranges[1].end(); it1++)
            {
                // << it0->str() << "," << it1->str() << endl;
                simulate(*it0, *it1, _board_cs);
            }
        }
    }   

    template <typename T> int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }

    void initialize_ranges()
    {

        for (auto it=_hands.begin(); it != _hands.end(); ++it)
        {
        	//cout << *it << endl;
            *it = expand_range(*it);
        }

        for (auto i=0; i<_hands.size(); i++)
        {
            vector<string> vector_of_strings;
            set<CardSet> range;
            boost::split(vector_of_strings, _hands[i], boost::is_any_of(","));
            for (auto it=vector_of_strings.begin(); it !=vector_of_strings.end(); ++it)
            {
                CardSet cs(*it);
                if (!cs.intersects(_board_cs))
                {
                    range.insert(cs);
                }
            }
            _ranges.push_back(range);
        }
    }

    string expand_range(const string& hands)
    {

        auto map_it = SimDriver::predefined_ranges.find(hands);
        if (map_it != SimDriver::predefined_ranges.end())
        {

            return expand_range(map_it->second);
        }

    	const string ranks = "23456789TJQKA";
    	const string suits = "shdc";
    	if (hands.find(",") == string::npos) // if ',' not in hands:
    	{
    		if (hands.find("-") != string::npos)
    		{
    			// TODO: Could be XY-XZ where Y>Z

    			// assume of form XX-YY where X>Y
    			string result = hands.substr(3, 2);
    			for (auto i = ranks.find(hands[3]) + 1; i < ranks.find(hands[0]) + 1; i++)
    			{
    				result += ",";
    				result += ranks[i];
    				result += ranks[i];
    			}
    			return expand_range(result);
    		}

    		if (hands.find("+") != string::npos)
    		{
    			if (hands[0] == hands[1])
    			{
	    			string result = hands.substr(0, 2);
	    			for (auto i = ranks.find(hands[1]) + 1; i < ranks.length(); i++)
	    			{
	    				result += ",";
	    				result += ranks[i];
	    				result += ranks[i];
	    			}
	    			return expand_range(result);
		    	}
		    	else // assumes hands[0] > hands[1]
		    	{
		    		string result = hands.substr(0, 2);
		    		if (hands[2] == 's' || hands[2] == 'o')
		    		{
		    			result += hands[2];
		    		}
	    			for (auto i = ranks.find(hands[1]) + 1; i < ranks.find(hands[0]); i++)
	    			{
	    				result += ",";
	    				result += hands[0];
	    				result += ranks[i];
						if (hands[2] == 's' || hands[2] == 'o')
			    		{
			    			result += hands[2];
			    		}
	    			}
	    			return expand_range(result);
		    	}
    		}
    		switch(hands.length())
    		{
    			case 4:
    				return hands;
    			case 3:
    				if (hands[2] == 'o')
    				{
						return  hands.substr(0, 1) + "s" + hands.substr(1, 1) + "h," +
								hands.substr(0, 1) + "s" + hands.substr(1, 1) + "d," +
								hands.substr(0, 1) + "s" + hands.substr(1, 1) + "c," +
								hands.substr(0, 1) + "h" + hands.substr(1, 1) + "s," +
								hands.substr(0, 1) + "h" + hands.substr(1, 1) + "d," +
								hands.substr(0, 1) + "h" + hands.substr(1, 1) + "c," +
								hands.substr(0, 1) + "d" + hands.substr(1, 1) + "s," +
								hands.substr(0, 1) + "d" + hands.substr(1, 1) + "h," +
								hands.substr(0, 1) + "d" + hands.substr(1, 1) + "c," +
								hands.substr(0, 1) + "c" + hands.substr(1, 1) + "s," +
								hands.substr(0, 1) + "c" + hands.substr(1, 1) + "h," +
								hands.substr(0, 1) + "c" + hands.substr(1, 1) + "d";
    				}
    				else
    				{
						return  hands.substr(0, 1) + "s" + hands.substr(1, 1) + "s," +
								hands.substr(0, 1) + "h" + hands.substr(1, 1) + "h," +
								hands.substr(0, 1) + "d" + hands.substr(1, 1) + "d," +
								hands.substr(0, 1) + "c" + hands.substr(1, 1) + "c";
    				}
    			case 2:
    				if (hands[0] == hands[1]) // pocket pair
    				{
    					return  hands.substr(0, 1) + "s" + hands.substr(1, 1) + "h," +
    							hands.substr(0, 1) + "s" + hands.substr(1, 1) + "d," +
    							hands.substr(0, 1) + "s" + hands.substr(1, 1) + "c," +
    							hands.substr(0, 1) + "h" + hands.substr(1, 1) + "d," +
    							hands.substr(0, 1) + "h" + hands.substr(1, 1) + "c," +
    							hands.substr(0, 1) + "d" + hands.substr(1, 1) + "c";
    				}
    				else
    				{
    					return expand_range(hands + "o," + hands + "s");			
    				}
    			default:
    				return "";
    		}
    	}
    	else
    	{
	       	vector<string> range;
	    	boost::split(range, hands, boost::is_any_of(","));
//	    	for (auto x=range.begin(); x!=range.end(); ++x)
//	    	{
//	    		cout << *x << ",";
//	    	}
//	    	cout << endl;
	    	return expand_range(range);
	    }
    }

    // recursive method to return comma separated string of individual hands from range
    // e.g., expand_range("AQs+,JJ") = 
    // "AsQs,AhQh,AdQd,AcQc,AsKs,AhKh,AdKd,AcKc,JsJh,JsJd,JsJc,JhJd,JhJc,JdJc"
    string expand_range(const vector<string>& hands)
    {
    	if (hands.size() > 1)
    	{
    		return expand_range(hands[0]) + "," + expand_range(vector<string>(hands.begin() + 1, hands.end()));
    	}
    	else
    	{
    		return expand_range(hands[0]);
    	}
    }

    // removes strings from 'hands' that intersect with 'board'
    void remove_duplicates(vector<string>& hands, const string& board)
    {
    	auto it=hands.begin();
    	while ( it != hands.end())
    	{
    		if ( board.find(it->substr(0, 2)) != string::npos || 
    				 board.find(it->substr(2, 2)) != string::npos )
    		{
    			it = hands.erase(it);
    		}
    		else
    		{
    			++it;
    		}
    	}
    }

    map<string, double> simulate(set<CardSet> range1, set<CardSet> range2)
    {
    	map<string, double> result;

    	if (_hands.size() != 2)
    	{
    		return result;
    	}

    	for (auto it=range1.begin(); it != range1.end(); ++it)
    	{
    		double value_sum = 0;
    		double value_count = 0;
    		for (auto it2=range2.begin(); it2 != range2.end(); ++it2)
    		{
    			double sim = simulate(*it, *it2, _board);

    			if (sim >= 0)
				{
					value_sum += sim;
					value_count++;
				}
    		}
    		if (value_count > 0)
    		{
    			result.insert(pair<string, double>(it->str(), value_sum / value_count));
    		}
    		else
    		{
    			result.insert(pair<string, double>(it->str(), 0.5)); // default result in case range2 empty
    		}
    	}

    	return result;
    }

    void print()
    {
        cout << this->str();
    }

    // hand vs hand binary, returns sampled result of first_hand in {1, 0}
    double simulate(const CardSet& first_hand, const CardSet& second_hand, const CardSet& board)
    {
    	if (first_hand.intersects(second_hand))
    		return -1;

    	int remaining_cards = 5 - board.size();
    	CardSet excludedCards(board);
    	excludedCards |= CardSet(first_hand);
    	excludedCards |= CardSet(second_hand);

    	auto rest_of_boards = createCardSet(remaining_cards, excludedCards); // assumes flop count = 3, total = 5, remaining = 2
        auto remaining_deck = createCardSet(1, excludedCards);

        for (auto tit=remaining_deck.begin(); tit!=remaining_deck.end(); ++tit)
        {
        	double wins = 0;
			double ties = 0;
			double total = 0;

        	for (auto rit=remaining_deck.begin(); rit!=remaining_deck.end(); ++rit)
        	{


        		if (*rit != *tit) // river != turn
        		{
        			auto entire_board = _board_cs | *tit | *rit;

        			PokerHandEvaluation first_eval = _peval->evaluate(CardSet(first_hand), entire_board);
					PokerHandEvaluation second_eval = _peval->evaluate(CardSet(second_hand), entire_board);

					total++;
					if (first_eval.high().code() > second_eval.high().code())
					{
						wins++;
					}
					else
					{
						if (first_eval.high().code() == second_eval.high().code())
						{
							ties++;
						}
					}

        		}
        	}

        	cout << first_hand.str() << "," << second_hand.str() << "," << _board << "," << tit->str() << "," << (wins+0.5*ties) / total << endl;

        }

		// loop through all possible "rest-of-boards"
//		for (auto rit=rest_of_boards.begin(); rit!=rest_of_boards.end(); ++rit)
//		{
//			auto entire_board = CardSet(board) | *rit;
//
//			PokerHandEvaluation first_eval = _peval->evaluate(CardSet(first_hand), entire_board);
//			PokerHandEvaluation second_eval = _peval->evaluate(CardSet(second_hand), entire_board);
//
//            cout << first_hand.str() << "," << second_hand.str() << "," << _board << "," << rit->str();
//            cout << "," << first_eval.high().str() << "," << second_eval.high().str() << first_eval.high().code();
//            cout << "," << sgn<double>(first_eval.high().code() - second_eval.high().code()) << endl;
//
//
//		}

		return 0;//(first_win_count + tie_count) / (first_win_count + second_win_count + tie_count);

    }

    string str() 
    {
        std::stringstream ret;
        ret << _board << endl << endl;
        // for (int i=0; i<_ranges.size(); i++)
        //     {
        //     //ret << "Total combos: " << _equities[i].size() << endl;
        //     for (auto it=_ranges[i].begin(); it!=_ranges[i].end(); ++it)
        //     {
        //         ret << it->str() << endl;
        //     }

        //     ret << endl;
        // }

        return ret.str();
    }

    vector<string> getHands()
    {
    	return _hands;
    }

    string getBoard()
    {
    	return _board;
    }

    map<string, string> create_predefined_ranges() // Janda
    {
        map<string, string> m;
        m["UTG"] = "33+,AJo+,KQo,ATs+,KTs+,QTs+,J9s+,T9s,98s,87s,76s,65s";
        m["MP"] = "22+,ATo+,KQo,A7s+,A5s,KTs+,QTs+,J9s+,T8s+,97s+,86s+,75s+,65s,54s";
        m["CO"] = "22+,ATo+,KJo+,QJo,A2s+,K6s+,Q7s+,J8s+,T8s+,97s+,86s+,75s+,64s+,54s";
        m["BTN"] = "22+,A2+,K7o+,Q9o+,J9o+,T8o+,98o,87o,K2s+,Q2s+,J5s+,T6s+,96s+,85s+,74s+,64s+,53s+,43s";
        m["SB"] = "22+,A7o+,K9o+,Q9o+,J9o+,T9o,98o,A2s+,K2s+,Q4s+,J7s+,T7s+,97s+,86s+,75s+,64s+,54s";
        m["MP_vs_UTG"] = "QQ-55,AQo+,AQs,AJs,ATs,KJs+,QJs,JTs,T9s,98s,87s";
        m["CO_vs_UTG"] = "QQ-44,AQo+,AQs,AJs,ATs,KJs+,QJs,JTs,T9s,98s,87s,76s,65s";
        m["BTN_vs_UTG"] = "QQ-33,AQo+,AQs,AJs,ATs,KTs+,QTs+,J9s+,T9s,98s,87s,76s,65s,54s";
        m["SB_vs_UTG"] = "QQ-88,AKo,AQs,KQs";
        m["BB_vs_UTG"] = "QQ-44,AQo+,AQs,AJs,ATs,KJs+,QJs,JTs";
        m["CO_vs_MP"] = "JJ-44,AQo+,AQs,AJs,ATs,KTs+,QTs+,JTs,T9s,98s,87s,76s";
        m["BTN_vs_MP"] = "JJ-33,AQo+,AQs,AJs,ATs,KTs+,QTs+,J9s+,T9s,98s,87s,76s,65s,54s";
        m["SB_vs_MP"] = "JJ-77,AQo+,AQs,KQs";
        m["BB_vs_MP"] = "JJ-22,AQo,AQs,AJs,ATs,KJs+,QJs,JTs,T9s,98s,87s";
        m["BTN_vs_CO"] = "AA,TT-22,AJo+,KQo,AQs,AJs,ATs,A9s,A8s,KTs+,QTs+,J9s+,T8s+,97s+,86s+,75s+,65s,54s";
        m["SB_vs_CO"] = "TT-88,AQo,AJo,KQo,AJs,ATs,KJs+,QJs";
        m["BB_vs_CO"] = "TT-22,AQo,AJo,KQo,AJs,ATs,KJs,KTs,QTs+,J9s+,T9s,98s";
        m["SB_vs_BTN"] = "99-66,KTo,QJo,QTo,A9s,A8s,KTs,K9s,QTs+,JTs,T9s";
        m["BB_vs_BTN"] = "99-33,A9o,A8,A7,A6,A5,A4,A3,A2,KT,K9,K8,K7,K6s,K5s,Q7s+,QJo,QTo,Q9o,JT,J9,J8s,T9,T8s";
        m["BB_vs_SB"] = "TT-22,AT,A9,A8,A7,A6,A5,A4,A3,A2,KJ,KT,K9,K8,K7,Q8+,J8+,T8o+,97o+,87o,76o,K6s,K5s,K4s,K3s,K2s,Q7s,Q6s,Q5s,Q4s,Q3s,Q2s,J7s,J6s,J5s,J4s,T5s+,95s+,85s+,74s+,64s+,53s+,43s";
        m["IP_3b_vs_UTG"] = "KK+,AJo,KQo,AKs,A5s,A4s";
        m["IP_3b_vs_MP"] = "QQ+,AJo,KQo,AKs,A5s,A4s,T8s,97s";
        m["IP_3b_vs_CO"] = "AsAh,AsAd,AcAh,AcAd,KK-JJ,AK,ATo,KJo,QJo,A7s,A6s,A5s,A4s,A3s,A2s";
        m["OOP_3b_vs_UTG"] = "KK+,AK,44-33,T9s,98s,87s,76s,65s";
        m["SB_3b_vs_MP"] = "QQ+,66-44,AKo,AQs+,JTs,T9s,98s,87s,76s";
        m["BB_3b_vs_MP"] = "QQ+,AKo,AQs+,QTs,J9s,T8s,97s+,87s,76s,65s,54s";
        m["SB_3b_vs_CO"] = "JJ+,55-44,AKo,AQs+,KTs,QTs,J9s+,T8s+,97s+,87s,76s,65s,54s";
        m["BB_3b_vs_CO"] = "JJ+,44-22,AKo,AQs+,A5s,A4s,K9s,Q9s,T8s,97s,86s+,75s+,64s+,54s";
        m["SB_3b_vs_BTN"] = "TT+,55-33,AT+,KJ+,A7s,A6s,A5s,A4s,A3s,A2s,K8s,K7s,K6s,K5s,K4s,Q8s+,J8s+,T8s,97s+,86s+,75s+,64s+,54s";
        m["BB_3b_vs_BTN"] = "TT+,22,AT+,KJ+,A9s,K4s,K3s,K2s,Q6s,Q5s,Q4s,Q3s,Q2s,J7s,J6s,T7s,96s+,85s+,75s+,64s+,53s+,43s";
        m["UTG_flat_3b"] = "KK,QQ,JJ,TT,AQ+,KQs";
        m["UTG_4b_OOP"] = "AA,98s,87s,76s";
        m["UTG_4b_IP"] = "AA,AJs,ATs,76s";
        m["MP_flat_3b_OOP"] = "QQ,JJ,TT,AQ+,KQs";
        m["MP_flat_3b_IP"] = "AA,QQ,JJ,TT,AQo+,AJs+,KQs,QJs";
        m["MP_4b_OOP"] = "";
        m["MP_4b_IP"] = "";
        m["BTN_flat_3b"] = "AA,TT-77,AQo,AJo,ATo,KT+,K9s,QJo,Q9s+,J9s+,AQs,AJs,ATs,A9s,A8s,A7s,A5s,A4s,A3s,A2s,T8s+,97s+,87s,76s,65s";
        return m;
    };

private:
    boost::shared_ptr<PokerHandEvaluator> _peval;
    vector<string> _hands;
    vector<set<CardSet> > _ranges;
    string _board;
    CardSet _board_cs;  
    map<string, string> predefined_ranges; 
};

int main (int argc, char ** argv)
{
	clock_t begin = clock();

    string extendedHelp = "\n"
        "   examples:\n"
		"       ./bin/ps-sim AK UTG --board Ad7s5d\n"
        "       ./bin/ps-sim BTN_vs_UTG UTG --board Ad7s5d\n"
        "\n"
        ;

    try 
    {
        // set up the program options, handle the help case, and extract the values
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,?",    "produce help message")
            ("game,g",    po::value<string>()->default_value("h"), "game to use for evaluation")
            ("board,b",   po::value<string>()->default_value(""),  "community cards for he/o/o8")
            ("hand,h",    po::value< vector<string> >(),           "a hand for evaluation")
            //("quiet,q",   "produce no output")
            ;
      
        po::positional_options_description p;
        p.add("hand", -1);
        po::variables_map vm;
        po::store (po::command_line_parser(argc, argv)
                   .style(po::command_line_style::unix_style)
                   .options(desc)
                   .positional(p)
                   .run(), vm);
        po::notify (vm);

        // check for help
        if (vm.count("help") || argc == 1)
        {
            cout << desc << extendedHelp << endl;
            return 1;
        }

        // extract the options
        SimDriver driver(vm["game"].as<string>(),
                          vm["hand"].as< vector<string> >(),
                          vm["board"].as<string>());

//        if (vm.count("quiet") == 0)
//            cout << driver.str();
    }
    catch(std::exception& e) 
    {
        cerr << "-- caught exception--\n" << e.what() << "\n";
        return 1;
    }
    catch(...) 
    {
        cerr << "Exception of unknown type!\n";
        return 1;
    }

    clock_t end = clock();
    //cout << "Total time: " << double(end - begin) / CLOCKS_PER_SEC << "s" << endl;

    return 0;
}
