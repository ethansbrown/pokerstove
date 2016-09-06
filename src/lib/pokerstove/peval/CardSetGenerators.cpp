/**
 * Copyright (c) 2013 Andrew Prock. All rights reserved.
 */
#include "CardSetGenerators.h"
#include "Card.h"
#include <pokerstove/util/combinations.h>
#include <boost/algorithm/string.hpp>

namespace pokerstove
{

std::set<CardSet>
createCardSet(const std::string& comma_separated_hands_string)
{
    std::vector<std::string> result;
    boost::split(result, comma_separated_hands_string, boost::is_any_of(","));
    return createCardSet(result);
}

std::set<CardSet>
createCardSet(const std::vector<std::string>& hands)
{
    std::set<CardSet> ret;
    for (std::vector<std::string>::const_iterator it=hands.begin(); it != hands.end(); ++it)
    {
        ret.insert(CardSet(*it));
    }
    return ret;
}

std::set<CardSet>
createCardSet(size_t numCards, Card::Grouping grouping)
{
    std::set<CardSet> ret;
    combinations cards(52,numCards);
    do
    {
        CardSet hand;
        for (size_t i=0; i<numCards; i++)
        {
            hand.insert (Card(cards[i]));
        }
        switch (grouping)
        {
        case Card::RANK_SUIT:
            ret.insert(hand);
            break;
        case Card::SUIT_CANONICAL:
            ret.insert(hand.canonize());
            break;
        case Card::RANK:
            ret.insert(hand.canonizeRanks());
            break;
        };

    }
    while (cards.next());
    return ret;
}

std::set<CardSet>
createCardSet(size_t numCards,  const CardSet& excludedCards, Card::Grouping grouping)
{
    std::set<CardSet> ret;
    combinations cards(52,numCards);
    do
    {
        CardSet hand;
        for (size_t i=0; i<numCards; i++)
        {
            hand.insert (Card(cards[i]));
        }

        if(hand.intersects(excludedCards))
            continue;

        switch (grouping)
        {
        case Card::RANK_SUIT:
            ret.insert(hand);
            break;
        case Card::SUIT_CANONICAL:
            ret.insert(hand.canonize());
            break;
        case Card::RANK:
            ret.insert(hand.canonizeRanks());
            break;
        };

    }
    while (cards.next());
    return ret;
}

std::vector<CardSet>
createCardSetVector(size_t numCards,  const CardSet& excludedCards, Card::Grouping grouping)
{
    std::vector<CardSet> ret;
    combinations cards(52,numCards);
    do
    {
        CardSet hand;
        for (size_t i=0; i<numCards; i++)
        {
            hand.insert (Card(cards[i]));
        }

        if(hand.intersects(excludedCards))
            continue;

        switch (grouping)
        {
        case Card::RANK_SUIT:
            ret.push_back(hand);
            break;
        case Card::SUIT_CANONICAL:
            ret.push_back(hand.canonize());
            break;
        case Card::RANK:
            ret.push_back(hand.canonizeRanks());
            break;
        };

    }
    while (cards.next());
    return ret;
}

}
