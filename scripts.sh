cleos -u https://eospush.tokenpocket.pro set account permission aiaidaotoken \
 active '{"threshold":1,"keys":[{"key":"EOS5A4EjGxnfZysDJXaTykYHcv5SNxgxizpKHuELTx6chHCYjEz1w","weight":1}],"accounts":[{"permission":{"actor":"aiaidaotoken","permission":"eosio.code"},"weight":1}]}' \
 -p aiaidaotoken@active

cleos -u https://eospush.tokenpocket.pro push action aiaidaotoken create \
  '{"maximum_supply":"1000000000000000.000 DOGE"}' \
       -p aiaidaotoken@active

#init(minter,admin_account,team_account,fund_account,marketing_account,divident_account,airdrop_account)
cleos -u https://eospush.tokenpocket.pro push action aiaidaotoken init \
  '["swapswapfarm","wdogdeployer","eoswdogeteam","eoswdogefund","eoswdogemakt","wdogeairdrop","wdogdivident","dogliquidity"]' -p aiaidaotoken@active

#setfees(team_fee,fund_fee,marketing_fee,burn_fee,divident_fee,airdrop_fee,liquidity_fee);
cleos -u https://eospush.tokenpocket.pro push action aiaidaotoken setfees \
  '[1500,1000,500,500,200,100,200]' -p wdogdeployer@active

cleos -u https://eospush.tokenpocket.pro push action aiaidaotoken addminter \
  '["swapswapfarm"]' -p wdogdeployer@active

cleos -u https://eospush.tokenpocket.pro push action aiaidaotoken addwhitelist \
  '["wdogdeployer"]' -p wdogdeployer@active

