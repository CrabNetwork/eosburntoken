#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>

using std::string;
using namespace eosio;

class [[eosio::contract("token")]] token : public contract {
public:
   using contract::contract;

   name SWAP_CONTRACT = name("eosaidaoswat");
   
   [[eosio::action]] 
   void init(name minter, name admin_account, name team_account, name fund_account, name marketing_account, name divident_account, name airdrop_account, name liquidity_account); 
   [[eosio::action]] 
   void addminter(const name minter);
   [[eosio::action]] 
   void addwhitelist(name account);
   [[eosio::action]] 
   void rmwhitelist(name account);
   [[eosio::action]] 
   void setaccounts(name admin_account, name dev_account, name fund_account, name marketing_account, name divident_account, name airdrop_account, name liquidity_account);
   [[eosio::action]] 
   void setfees(uint64_t team_fee, uint64_t fund_fee, uint64_t marketing_fee, uint64_t burn_fee, uint64_t divident_fee, uint64_t airdrop_fee, uint64_t liquidity_fee);
   [[eosio::action]] 
   void close(const name &owner, const symbol &symbol);
   [[eosio::action]] 
   void create(const asset &maximum_supply);
   [[eosio::action]] 
   void transfer(name from, name to, asset quantity, string memo);
   [[eosio::action]] 
   void safetransfer(name from, name to, asset quantity, string memo);
   [[eosio::action]] 
   void mint(const name &owner, const name &to, const asset &quantity, const string &memo);
   [[eosio::action]] 
   void burn(const name &owner, const asset &quantity, const string &memo);



private:
   struct [[eosio::table]] account {
      asset balance;

      uint64_t primary_key() const { return balance.symbol.code().raw(); }
   };

   struct [[eosio::table]] currency_stats {
      asset supply;
      asset max_supply;
      name issuer;

      uint64_t primary_key() const { return supply.symbol.code().raw(); }
   };

   struct [[eosio::table]] minter {
      name minter;
      uint64_t primary_key() const { return minter.value; }
   };

   struct [[eosio::table]] whitelist {
      name account;
      uint64_t primary_key() const { return account.value; }
   };

   struct [[eosio::table]] config_t {
      name admin_account;
      name team_account;
      name fund_account;
      name marketing_account;
      name divident_account;
      name airdrop_account;
      name liquidity_account;
      uint64_t team_fee;
      uint64_t fund_fee;
      uint64_t marketing_fee;
      uint64_t burn_fee;
      uint64_t divident_fee;
      uint64_t airdrop_fee;
      uint64_t liquidity_fee;
   }; 

   typedef eosio::singleton<"configs"_n, config_t> configs;
   typedef multi_index<name("configs"), config_t> configs_for_abi;
   typedef eosio::multi_index<"accounts"_n, account> accounts;
   typedef eosio::multi_index<"stat"_n, currency_stats> stats;
   typedef eosio::multi_index<"minters"_n, minter> minters;
   typedef eosio::multi_index<"whitelists"_n, whitelist> whitelists;

   configs _configs = configs(_self, _self.value);
   minters _minters = minters(_self, _self.value);
   whitelists _whitelists = whitelists(_self, _self.value);
   
   void sub_balance(const name &owner, const asset &value);
   void add_balance(const name &owner, const asset &value, const name &ram_payer);
   void transfer_standard(name from, name to, asset quantity, string memo, name payer);
   void transfer_by_fee(name from, name to, asset quantity, string memo, name payer);

   std::vector<string> split( const string str, const string delim ) {
      std::vector<string> tokens;
      if ( str.size() == 0 ) return tokens;
      size_t prev = 0, pos = 0;
      do {
         pos = str.find(delim, prev);
         if (pos == string::npos) pos = str.length();
         string token = str.substr(prev, pos-prev);
         if (token.length() > 0) tokens.push_back(token);
         prev = pos + delim.length();
      }
      while (pos < str.length() && prev < str.length());
      return tokens;
   }
};