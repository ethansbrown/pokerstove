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
               const string& board)
        : _peval(PokerHandEvaluator::alloc (game))
        , _hands(hands)
        , _board(board)
    {
        srand(time(NULL));
        predefined_ranges = create_predefined_ranges();
    	expand_hands();



        _results = simulate();

        for (auto it=_results.begin(); it != _results.end(); ++it)
        {
            //cout << it->first << " " << it->second << endl;
            _ordered_results.push_back(std::pair<string, double>(it->first, it->second));
        }

        std::stable_sort(_ordered_results.begin(), _ordered_results.end(), 
            boost::bind(&std::pair<string, double>::second, _1) > 
            boost::bind(&std::pair<string, double>::second, _2));
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

    static vector<string> parse_hands(string hands)
    {
    	vector<string> result;
    	boost::split(result, hands, boost::is_any_of(","));
    	return result;
    }

    // removes strings from 'hands' that intersect with 'board'
    static void remove_duplicates(vector<string>& hands, const string& board)
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

    map<string, double> turn_equity_likelihood(double threshold)
    {
        map<string, double> result;
        if (_hands.size() != 2)
        {
            return result;
        }
        return result;
    }

    map<string, double> simulate()
    {
    	map<string, double> result;

    	if (_hands.size() != 2)
    	{
    		return result;
    	}

    	vector<string> first_range = parse_hands(_hands.front());
		vector<string> second_range = parse_hands(_hands.back());

		remove_duplicates(first_range, _board);
		remove_duplicates(second_range, _board);

    	for (auto it=first_range.begin(); it != first_range.end(); ++it)
    	{
    		// double value = simulate(it->str(), _hands.back(), _board, _peval);
    		// result.insert(pair<string, double>(it->str(), value));
    		double value_sum = 0;
    		double value_count = 0;
    		for (auto it2=second_range.begin(); it2 != second_range.end(); ++it2)
    		{
				//auto sim = simulate(*it, *it2, _board, _peval);
                auto sim = simulate(*it, *it2, _board, _peval);
				if (sim >= 0) 
				{
					value_sum += sim;
					value_count++;
				}
    		}
    		if (value_count > 0)
    		{
    			result.insert(pair<string, double>(*it, value_sum / value_count));
    		}
    		else
    		{
    			result.insert(pair<string, double>(*it, 0.5)); // default result in case range2 empty
    		}
    	}

    	return result;
    }

    // hand vs hand binary, returns sampled result of first_hand in {1, 0}
    static double approx_sim(const string& first_hand, const string& second_hand, const string& board, boost::shared_ptr<PokerHandEvaluator> pe)
    {
        //cout << "approx_sim" << first_hand << " " << second_hand << " " << board << endl;
        if (CardSet(first_hand).intersects(CardSet(second_hand)))
            return -1;

        CardSet excludedCards(board);
        int remaining_cards = 5 - excludedCards.size();
        excludedCards |= CardSet(first_hand);
        excludedCards |= CardSet(second_hand);

        set<CardSet> rest_of_boards = createCardSet(remaining_cards, excludedCards); 
        int random = rand() % (int)(rest_of_boards.size());
        int count = 0;

        for (auto rit=rest_of_boards.begin(); rit!=rest_of_boards.end(); count++,rit++)
        {
            if (count > random)
            {
                auto entire_board = CardSet(board) | *rit;
                PokerHandEvaluation first_eval = pe->evaluate(CardSet(first_hand), entire_board);
                PokerHandEvaluation second_eval = pe->evaluate(CardSet(second_hand), entire_board);
                /*cout << boost::format("%s, %s, %s : [%9d] /") 
                        % _hands[index]
                        % _board
                        % rit->str()
                        % eval.high().str();*/
                if (first_eval.high().code() > second_eval.high().code())
                {
                    return 1;
                }
                else 
                {
                    if (first_eval.high().code() < second_eval.high().code())
                    {
                        return 0;
                    }
                    else return 0.5;
                }
            }
        }

        return 0; 
    }

    // hand vs hand simulation, returns equity of first_hand
    static double simulate(const string& first_hand, const string& second_hand, const string& board, boost::shared_ptr<PokerHandEvaluator> pe)
    {
    	// auto first_hand = _hands[0];
    	// auto second_hand = _hands[1];
    	if (CardSet(first_hand).intersects(CardSet(second_hand)))
    		return -1;

    	// create rest_of_boards CardSet
    	CardSet excludedCards(board);
    	int remaining_cards = 5 - excludedCards.size();
    	excludedCards |= CardSet(first_hand);
    	excludedCards |= CardSet(second_hand);
    	
    	// for (auto it=_hands.begin(); it!=_hands.end(); it++)
    	// {
    	// 	excludedCards |= CardSet(*it);
    	// }
    	
        auto rest_of_boards = createCardSet(remaining_cards, excludedCards); // assumes flop count = 3, total = 5, remaining = 2

        // loop through all possible "rest-of-boards"
        double first_win_count = 0.0;
        double second_win_count = 0.0;
        for (auto rit=rest_of_boards.begin(); rit!=rest_of_boards.end(); rit++)
        {
        	auto entire_board = CardSet(board) | *rit;
        	
        	//auto eval = PokerHandEvaluator::alloc (game);

        	PokerHandEvaluation first_eval = pe->evaluate(CardSet(first_hand), entire_board);
			PokerHandEvaluation second_eval = pe->evaluate(CardSet(second_hand), entire_board);
        		/*cout << boost::format("%s, %s, %s : [%9d] /") 
                        % _hands[index]
                        % _board
                        % rit->str()
                        % eval.high().str();*/
            if (first_eval.high().code() > second_eval.high().code())
            {
            	first_win_count++;
            }
            if (first_eval.high().code() < second_eval.high().code())
            {
            	second_win_count++;
            }
        }

        return first_win_count / (first_win_count + second_win_count);
        // for (auto i=0; i<num_winner.size(); i++)
        // {
        // 	cout << _hands[i] << ": " << num_winner[i] << endl;
        // }
    }

    string str() const
    {
        std::stringstream ret;
        ret << "Total combos: " << _ordered_results.size() << endl;
        for (auto it=_ordered_results.begin(); it!=_ordered_results.end(); ++it)
        {
            const string hand = it->first;
            const double value = it->second; 
            ret << hand << ": " << value << endl; 
            //ret += boost::str(boost::format("%s: %f\n") 
            //                  % hand % value;
            //sret += hand + " " + 
        }
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

    map<string, string> create_predefined_ranges()
    {
        map<string, string> m;
        m["UTG"] = "33+,AJo+,KQo,ATs+,KTs+,QTs+,J9s+,T9s,98s,87s,76s,65s";
        m["MP"] = "22+,ATo+,KQo,A7s+,KTs+,QTs+,J9s+,T8s+,97s+,86s+,75s+,65s,54s";
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
        m["SB_vs_CO"] = "";
        m["BB_vs_CO"] = "";
        m["SB_vs_BTN"] = "";
        m["BB_vs_BTN"] = "";
        m["BB_vs_SB"] = "";
        m["IP_3b_vs_UTG"] = "KK+,AJo,KQo,AKs,A5s,A4s";
        m["IP_3b_vs_MP"] = "QQ+,AJo,KQo,AKs,A5s,A4s,T8s,97s";
        return m;
    };

private:
    boost::shared_ptr<PokerHandEvaluator> _peval;
    vector<string> _hands;
    string _board;
    map<string, double> _results;
    vector<pair<string, double> > _ordered_results;

    map<string, string> predefined_ranges; 

    void expand_hands()
    {
    	for (auto it=_hands.begin(); it != _hands.end(); ++it)
    	{
    		*it = expand_range(*it);
    	}
    }


};

int main (int argc, char ** argv)
{
    string extendedHelp = "\n"
        "   examples:\n"
        "       ./bin/ps-sim QQ-44,AQo+,AQs,AJs,ATs,KJs+,QJs,JTs,T9s,98s,87s,76s,65s AA-33,AJo+,KQo,ATs+,KTs+,QTs+,J9s+,T9s,98s,87s,76s,65s --board Ad7s5d\n"
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
            ("quiet,q",   "produce no output")
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

        if (vm.count("quiet") == 0)
            cout << driver.str();
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
    return 0;
}