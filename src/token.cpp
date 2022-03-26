#include <token.hpp>

void token::init(name minter, name admin_account, name team_account, name fund_account, name marketing_account, name divident_account, name airdrop_account, name liquidity_account) {
   require_auth(get_self());
   config_t config = _configs.get_or_create(get_self(), config_t{});
   config.admin_account = admin_account;
   config.team_account = team_account;
   config.fund_account = fund_account;
   config.marketing_account = marketing_account;
   config.divident_account = divident_account;
   config.airdrop_account = airdrop_account;
   config.liquidity_account = liquidity_account;
   _configs.set(config, _self);

   auto minter_itr = _minters.find(minter.value);
   if(minter_itr == _minters.end()) {
      _minters.emplace(get_self(), [&](auto &s) {
         s.minter = minter;
      });
   }
}

void token::addminter(const name minter) {
   config_t config = _configs.get();
   require_auth(config.admin_account);
   auto minter_itr = _minters.find(minter.value);
   if(minter_itr == _minters.end()) {
      _minters.emplace(get_self(), [&](auto &s) {
         s.minter = minter;
      });
   }
}

void token::addwhitelist(name account) {
   config_t config = _configs.get();
   require_auth(config.admin_account);
   auto whitelist_itr = _whitelists.find(account.value);
   if(whitelist_itr == _whitelists.end()) {
      _whitelists.emplace(get_self(), [&](auto &s) {
         s.account = account;
      });
   }
}

void token::rmwhitelist(name account) {
   config_t config = _configs.get();
   require_auth(config.admin_account);
   auto whitelist_itr = _whitelists.require_find(account.value, "Whitelist account not exists!");
   _whitelists.erase(whitelist_itr);
}

void token::setaccounts(name admin_account, name team_account, name fund_account, name marketing_account, name divident_account, name airdrop_account, name liquidity_account) {
   config_t config = _configs.get();
   require_auth(config.admin_account);
   config.admin_account = admin_account;
   config.team_account = team_account;
   config.fund_account = fund_account;
   config.marketing_account = marketing_account;
   config.divident_account = divident_account;
   config.airdrop_account = airdrop_account;
   config.liquidity_account = liquidity_account;
   _configs.set(config, _self);
}

void token::setfees(uint64_t team_fee, uint64_t fund_fee, uint64_t marketing_fee, uint64_t burn_fee, uint64_t divident_fee, uint64_t airdrop_fee, uint64_t liquidity_fee) {
   config_t config = _configs.get();
   require_auth(config.admin_account);
   config.team_fee = team_fee;
   config.fund_fee = fund_fee;
   config.marketing_fee = marketing_fee;
   config.burn_fee = burn_fee;
   config.airdrop_fee = airdrop_fee;
   config.divident_fee = divident_fee;
   config.liquidity_fee = liquidity_fee;
   _configs.set(config, _self);
}

void token::create(const asset &maximum_supply) {
   require_auth(get_self());

   auto sym = maximum_supply.symbol;
   check(sym.is_valid(), "invalid symbol name");
   check(maximum_supply.is_valid(), "invalid supply");
   check(maximum_supply.amount > 0, "max-supply must be positive");

   stats statstable(get_self(), sym.code().raw());
   auto existing = statstable.find(sym.code().raw());
   check(existing == statstable.end(), "token with symbol already exists");

   statstable.emplace(get_self(), [&](auto &s) {
      s.supply.symbol = maximum_supply.symbol;
      s.max_supply = maximum_supply;
   });

   accounts acnts(get_self(), get_self().value);
   auto it = acnts.find(sym.code().raw());
   if (it == acnts.end()) {
      acnts.emplace(get_self(), [&](auto &a) {
         a.balance = asset{0, sym};
      });
   }
}

void token::mint(const name &owner, const name &to, const asset &quantity, const string &memo) {
   auto sym = quantity.symbol;
   check(sym.is_valid(), "invalid symbol name");
   check(memo.size() <= 256, "memo has more than 256 bytes");

   stats statstable(_self, sym.code().raw());
   auto existing = statstable.find(sym.code().raw());
   check(existing != statstable.end(), "token with symbol does not exist, create token before issue");
   const auto &st = *existing;

   auto minter_itr = _minters.require_find(owner.value, "owner not exists!");

   require_auth(owner);
   check(quantity.is_valid(), "invalid quantity");
   check(quantity.amount > 0, "must issue positive quantity");
   check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");

   config_t config = _configs.get();
   uint64_t team_fees = config.team_fee * quantity.amount / 10000;
   uint64_t fund_fees = config.fund_fee * quantity.amount / 10000;
   uint64_t marketing_fees = config.marketing_fee * quantity.amount / 10000;
   uint64_t total_amount = team_fees + fund_fees + marketing_fees + quantity.amount;
   check(total_amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

   asset total_mint(total_amount, sym);
   statstable.modify(st, same_payer, [&](auto &s) {
      s.supply += total_mint;
   });

   add_balance(owner, quantity, owner);
   if(team_fees > 0) add_balance(config.team_account, asset(team_fees, sym), owner);
   if(fund_fees > 0) add_balance(config.fund_account, asset(fund_fees, sym), owner);
   if(marketing_fees > 0) add_balance(config.marketing_account, asset(marketing_fees, sym), owner);

   if (to != owner) {
      SEND_INLINE_ACTION(*this, transfer, {{owner, "active"_n}}, {owner, to, quantity, memo});
   }
}

void token::burn(const name &owner, const asset &quantity, const string &memo) {
   require_auth(owner);
   auto sym = quantity.symbol;
   check(sym.is_valid(), "invalid symbol name");
   check(memo.size() <= 256, "memo has more than 256 bytes");

   stats statstable(get_self(), sym.code().raw());
   auto existing = statstable.find(sym.code().raw());
   check(existing != statstable.end(), "token with symbol does not exist");
   const auto &st = *existing;

   check(quantity.is_valid(), "invalid quantity");
   check(quantity.amount > 0, "must retire positive quantity");

   check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");

   statstable.modify(st, same_payer, [&](auto &s) {
      s.supply -= quantity;
   });

   sub_balance(owner, quantity);
}

void token::transfer(name from, name to, asset quantity, string memo) {
   check(from != to, "cannot transfer to self");
   require_auth(from);
   check(is_account(to), "to account does not exist");
   auto sym = quantity.symbol;
   stats statstable(get_self(), sym.code().raw());
   const auto &st = statstable.get(sym.code().raw());

   check(quantity.is_valid(), "invalid quantity");
   check(quantity.amount > 0, "must transfer positive quantity");
   check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
   check(memo.size() <= 256, "memo has more than 256 bytes");

   auto payer = has_auth(to) ? to : from;
   sub_balance(from, quantity);

   bool is_swap = false;
   bool is_deposit = false;
   if (to == SWAP_CONTRACT) {
      std::vector<string> s = split(memo, ",");
      is_swap = s.size() == 3 && s[0] == "swap";
      is_deposit = s.size() == 2 && (s[0] == "deposit" || s[0] == "withdraw");
   }

   auto from_whitelist_itr = _whitelists.find(from.value);
   auto to_whitelist_itr = _whitelists.find(to.value);
   if(is_swap) {
      transfer_by_fee(from, to, quantity, memo, payer);
   } else if(from == SWAP_CONTRACT && !is_deposit) {
      transfer_by_fee(from, to, quantity, memo, payer);
   } else if(is_deposit || from_whitelist_itr != _whitelists.end() || to_whitelist_itr != _whitelists.end()) {
      transfer_standard(from, to, quantity, memo, payer);
   } else {
      transfer_by_fee(from, to, quantity, memo, payer);
   }
}

void token::transfer_standard(name from, name to, asset quantity, string memo, name payer) {
   add_balance(to, quantity, payer);
   require_recipient(from);
   require_recipient(to);
}

void token::transfer_by_fee(name from, name to, asset quantity, string memo, name payer) {
   auto sym = quantity.symbol;
   config_t config = _configs.get();
   asset burn_quantity = asset(config.burn_fee * quantity.amount / 10000, sym);
   asset airdrop_quantity = asset(config.airdrop_fee * quantity.amount / 10000, sym);
   asset divident_quantity = asset(config.divident_fee * quantity.amount / 10000, sym);
   asset liquidity_quantity = asset(config.liquidity_fee * quantity.amount / 10000, sym);
   asset to_quantity = quantity - burn_quantity - airdrop_quantity - divident_quantity - liquidity_quantity;

   if (burn_quantity.amount > 0) add_balance(name("eosio.null"), burn_quantity, payer);
   if (airdrop_quantity.amount > 0) add_balance(config.airdrop_account, airdrop_quantity, payer);
   if (divident_quantity.amount > 0) add_balance(config.divident_account, divident_quantity, payer);
   if (liquidity_quantity.amount > 0) add_balance(config.liquidity_account, liquidity_quantity, payer);
   if (to_quantity.amount > 0) add_balance(to, to_quantity, payer);

   auto data = make_tuple(from, to, to_quantity, memo);
   action(permission_level{get_self(), name("active")}, get_self(), name("safetransfer"), data).send();
}

void token::safetransfer(name from, name to, asset quantity, string memo) {
   require_auth(get_self());
   require_recipient(from);
   require_recipient(to);
}

void token::sub_balance(const name &owner, const asset &value) {
   accounts from_acnts(get_self(), owner.value);

   const auto &from = from_acnts.get(value.symbol.code().raw(), "no balance object found");

   auto balance = from.balance.amount;
   check(balance >= value.amount, "overdrawn balance");
  
   from_acnts.modify(from, owner, [&](auto &a) {
      a.balance -= value;
   });
}

void token::add_balance(const name &owner, const asset &value, const name &ram_payer) {
   accounts to_acnts(get_self(), owner.value);
   auto to = to_acnts.find(value.symbol.code().raw());
   if (to == to_acnts.end()) {
      to_acnts.emplace(ram_payer, [&](auto &a) {
         a.balance = value;
      });
   } else {
      to_acnts.modify(to, same_payer, [&](auto &a) {
         a.balance += value;
      });
   }
}

void token::close(const name &owner, const symbol &symbol) {
   require_auth(owner);
   accounts acnts(get_self(), owner.value);
   auto it = acnts.find(symbol.code().raw());
   check(it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect.");
   check(it->balance.amount == 0, "Cannot close because the balance is not zero.");
   acnts.erase(it);
}
