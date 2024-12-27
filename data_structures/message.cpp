#include <math.h> 
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>

#include "message.hpp"

/***************************************************/
/************* Output stream ************************/
/***************************************************/

ostream& operator<<(ostream& os, const Message_initialIP_t& msg) {
  os << msg.productID << " " << msg.name << " " << msg.category << " " << msg.subcategory << " " << msg.initialPrice << " "  << msg.bestPrice << " " << msg.ranking << " " <<  msg.status << " " <<  msg.sold;
  return os;
}

/***************************************************/
/************* Input stream ************************/
/***************************************************/

istream& operator>> (istream& is, Message_initialIP_t& msg) {
  is >> msg.productID;
  is >> msg.name;
  is >> msg.category;
  is >> msg.subcategory;
  is >> msg.initialPrice;
  is >> msg.bestPrice;
  is >> msg.ranking;
  is >> msg.status;
  is >> msg.sold;

  return is;
}


/***************************************************/
/************* Output stream ************************/
/***************************************************/


ostream& operator<<(ostream& os, const Message_bidOffer_t&msg){
  os << " ClientID: " << msg.clientID << " ProductID: " << msg.productID << " Price Proposal: " << msg.priceProposal;
  return os;
}


/***************************************************/
/************* Input stream ************************/
/***************************************************/


istream& operator>> (istream& is, Message_bidOffer_t&msg){
  is >> msg.clientID;
  is >> msg.productID;
  is >> msg.priceProposal; 
  return is;
}


/***************************************************/
/************* Output stream ************************/
/***************************************************/

ostream& operator<<(ostream& os, const Message_roundResult_t& msg) {
  os << " ProductID: " << msg.productID
     << " WinnerID: " << msg.winnerID 
     << " Best Price: " << msg.bestPrice
     << " N° Round: " << msg.round;
  return os;
}

/***************************************************/
/************* Input stream ************************/
/***************************************************/

istream& operator>>(istream& is, Message_roundResult_t& msg) {
  is >> msg.productID;
  is >> msg.winnerID;
  is >> msg.bestPrice;
  is >> msg.round;
  return is;
}

/***************************************************/
/************* Output stream ************************/
/***************************************************/

ostream& operator<<(ostream& os, const Message_finalResults_t& msg) {
  os << " ProductID: " << msg.productID
     << " WinnerID: " << msg.clientID 
     << " Best Price: " << msg.bestPrice
     << " Initial Price: " << msg.initialPrice
     << " N° Round: " << msg.numberRound;
  return os;
}

/***************************************************/
/************* Input stream ************************/
/***************************************************/

istream& operator>>(istream& is, Message_finalResults_t& msg) {
  is >> msg.productID;
  is >> msg.clientID;
  is >> msg.bestPrice;
  is >> msg.initialPrice;
  is >> msg.numberRound;
  return is;
}
