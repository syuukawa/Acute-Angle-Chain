/**
* @file account.hpp
* @copyright defined in Acute-Angle-Chain/LICENSE.txt
* @brief Defines types and ABI for account API interactions
*
*/
#pragma once
#include <aaciolib/account.h>
#include <aaciolib/print.hpp>
#include <aaciolib/asset.hpp>


namespace aacio { namespace account {
  /**
  *  @defgroup accountcppapi Account C++ API
  *  @brief C++ API for querying account data, e.g. account balance
  *  @ingroup accountapi
  *
  * @{
  */

/**
*  @brief The binary structure expected and populated by native balance function.
*  @details
*  Example:
*  @code
*  account_balance test1_balance;
*  test1_balance.account = N(test1);
*  if (account_api::get(test1_balance))
*  {
*     aacio::print("test1 balance=", test1_balance.aac_balance, "\n");
*  }
*  @endcode
*  @{
*/
struct PACKED(account_balance) {
  /**
  * @brief Name of the account who's balance this is
  * @details Name of the account who's balance this is
  */
  account_name account;

  /**
  * @brief Balance for this account
  * @details Balance for this account
  */
  asset aac_balance;

  /**
  * @brief Staked balance for this account
  * @details Staked balance for this account
  */
  asset staked_balance;

  /**
  * @brief Unstaking balance for this account
  * @details Unstaking balance for this account
  */
  asset unstaking_balance;

  /**
  * @brief Time at which last unstaking occurred for this account
  * @details Time at which last unstaking occurred for this account
  */
  time last_unstaking_time;
};
/// @} account_balance

/**
 *  @brief Retrieve a populated balance structure
 *  @details Retrieve a populated balance structure
 *  @param acnt - account
 *  @return true if account's balance is found
 */
bool get(account_balance& acnt)
{
   return account_balance_get(&acnt, sizeof(account_balance));
}

/// @}  aacio
} }
