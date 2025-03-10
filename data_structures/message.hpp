#ifndef BOOST_SIMULATION_MESSAGE_HPP
#define BOOST_SIMULATION_MESSAGE_HPP

#include <assert.h>
#include <iostream>
#include <string>

using namespace std;

/*******************************************/
/********** Message_initialIP_t ***********/
/*******************************************/

struct Message_initialIP_t
{

  Message_initialIP_t() {}
  Message_initialIP_t(int i_productID, string s_name, string s_category, string s_subcategory, float i_initialPrice, float i_bestPrice, float f_ranking, string s_status, bool b_sold)
      : productID(i_productID), name(s_name), category(s_category), subcategory(s_subcategory), initialPrice(i_initialPrice), bestPrice(i_bestPrice), ranking(f_ranking), status(s_status), sold(b_sold) {}

  int productID;
  string name;
  string category;
  string subcategory;
  float initialPrice;
  float bestPrice;
  float ranking;
  string status;
  bool sold;
};

istream &operator>>(istream &is, Message_initialIP_t &msg);
ostream &operator<<(ostream &os, const Message_initialIP_t &msg);

/*******************************************/
/********** Message_roundResults_t ***********/
/*******************************************/

struct Message_roundResult_t
{

  Message_roundResult_t() {}
  Message_roundResult_t(int o_productID, int o_client, float o_bestPrice, int o_round)
      : productID(o_productID), clientID(o_client), bestPrice(o_bestPrice), round(o_round) {}
  int productID;
  int clientID;
  float bestPrice;
  int round;
};

istream &operator>>(istream &is, Message_roundResult_t &msg);
ostream &operator<<(ostream &os, const Message_roundResult_t &msg);

/*******************************************/
/********** Message_bidOffer_t ***********/
/*******************************************/

struct Message_bidOffer_t
{
  Message_bidOffer_t() {}
  Message_bidOffer_t(int o_clientID, int o_productID, float o_price_proposal)
      : clientID(o_clientID), productID(o_productID), priceProposal(o_price_proposal) {}

  int clientID;
  int productID;
  float priceProposal;
};

istream &operator>>(istream &is, Message_bidOffer_t &msg);
ostream &operator<<(ostream &os, const Message_bidOffer_t &msg);


/*******************************************/
/********* Message_finalResults_t **********/
/*******************************************/

struct Message_finalResults_t
{
  Message_finalResults_t() {}
  Message_finalResults_t(int o_winnerID, int o_productID, float o_bestPrice, float o_initalPrice, int o_round)
      : winnerID(o_winnerID), productID(o_productID), bestPrice(o_bestPrice), initialPrice(o_initalPrice), numberRound(o_round) {}
  int winnerID;
  int productID;
  float bestPrice;
  float initialPrice;
  int numberRound;
};

istream &operator>>(istream &is, Message_finalResults_t &msg);
ostream &operator<<(ostream &os, const Message_finalResults_t &msg);

#endif //BOOST_SIMULATION_MESSAGE_HPP