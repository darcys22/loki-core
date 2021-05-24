// Copyright (c) 2021, The Oxen Project
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "db_sqlite.h"

#include <sqlite_orm/sqlite_orm.h>

#include <string>
#include <iostream>
#include <cassert>


#undef OXEN_DEFAULT_LOG_CATEGORY
#define OXEN_DEFAULT_LOG_CATEGORY "blockchain.db.sqlite"

namespace cryptonote
{

BlockchainSQLite::BlockchainSQLite(){};

void BlockchainSQLite::load_database(std::optional<fs::path> file)
{
  if (m_storage)
    throw std::runtime_error("Reloading database not supported");  // TODO

  // sqlite_orm treats empty-string as an indicator to load a memory-backed database, which we'll
  // use if file is an empty-optional
  std::string fileString;
  if (file.has_value())
  {
    fileString = file->string();
    MINFO("Loading sqliteDB from file " << fileString);
  }
  else
  {
    MINFO("Loading memory-backed sqliteDB");
  }

  m_storage = std::make_unique<sqliteDBStorage>(initStorage(fileString));
  m_storage->sync_schema(true);

}

// tuple (Address, amount)
bool BlockchainSQLite::add_sn_payments(cryptonote::network_type nettype, std::vector<std::tuple<std::string, uint64_t>>& payments)
{

  using namespace sqlite_orm;

  for (auto& payment: payments) {
    if (cryptonote::is_valid_address(std::get<0>(payment), nettype))
    {
      auto prev_entry = m_storage->get_pointer<cryptonote::batch_sn_payments>(std::get<0>(payment));
      if( auto prev_entry = m_storage->get_pointer<cryptonote::batch_sn_payments>(std::get<0>(payment))){
          MINFO("Record found for SN reward contributor, adding to database");
          m_storage->update_all(
              set(
                c(&cryptonote::batch_sn_payments::amount) = (prev_entry->amount + std::get<1>(payment))),
              where(c(&cryptonote::batch_sn_payments::address) = prev_entry->address));
      } else {
          MINFO("No record found for SN reward contributor, adding to database");
          m_storage->replace(cryptonote::batch_sn_payments{std::get<0>(payment), std::get<1>(payment)});
      }



    } else {
      return false;
    }
  };
  return true;
}

// tuple (Address, amount)
bool BlockchainSQLite::subtract_sn_payments(cryptonote::network_type nettype, std::vector<std::tuple<std::string, uint64_t>>& payments)
{

  using namespace sqlite_orm;
  
  for (auto& payment: payments) {
    if (cryptonote::is_valid_address(std::get<0>(payment), nettype))
    {
      auto prev_entry = m_storage->get<cryptonote::batch_sn_payments>(std::get<0>(payment));
      if ((int(prev_entry.amount) - std::get<1>(payment)) < 0)
        return false;
      m_storage->update_all(
          set(
            c(&cryptonote::batch_sn_payments::amount) = (prev_entry.amount - std::get<1>(payment))),
          where(c(&cryptonote::batch_sn_payments::address) = prev_entry.address));
    } else {
      return false;
    }
  };
  return true;
}

std::optional<std::vector<cryptonote::reward_payout>> BlockchainSQLite::get_sn_payments()
{

  MINFO(__FILE__ << ":" << __LINE__ << " TODO sean remove this - QWERTYUIOP get_sn_payments start ");
  using namespace sqlite_orm;

  //SELECT
    //address,
    //amount
  auto result = m_storage->select(
      columns(
        &cryptonote::batch_sn_payments::address,
        &cryptonote::batch_sn_payments::amount)
      );

  std::vector<cryptonote::reward_payout> payments;

  MINFO(__FILE__ << ":" << __LINE__ << " TODO sean remove this - QWERTYUIOP payments size: " << payments.size());
  for(auto &payment : result) {
    std::cout << std::get<0>(payment) << '\t' << std::get<1>(payment) << std::endl;
    cryptonote::address_parse_info info;
    if (cryptonote::get_account_address_from_str(info, cryptonote::network_type::DEVNET, std::get<0>(payment)))
    {
      //cryptonote::reward_payout pmt = {cryptonote::reward_type::snode, info.address, std::get<1>(payment)};
      cryptonote::reward_payout pmt = {cryptonote::reward_type::snode, info.address, 500};
      payments.push_back(pmt);
    }
    else
      return std::nullopt;
  }

  MINFO(__FILE__ << ":" << __LINE__ << " TODO sean remove this - QWERTYUIOP get_sn_payments end");
  return payments;
}

std::vector<std::tuple<std::string, uint64_t>> BlockchainSQLite::calculate_rewards(const cryptonote::block& block, std::vector<std::tuple<std::string, uint64_t>> contributors)
{
  uint64_t distribution_amount = block.reward;
  uint64_t total_contributed_to_winner_sn = 0;
  std::vector<std::tuple<std::string, uint64_t>> payments;
  for (auto & contributor : contributors)
  {
    total_contributed_to_winner_sn += std::get<1>(contributor);
  }
  for (auto & contributor : contributors)
  {
    payments.emplace_back(std::get<0>(contributor),std::get<1>(contributor)/total_contributed_to_winner_sn*distribution_amount);
  }
  return payments;
}

//TODO sean change contributors to vector of payouts
bool BlockchainSQLite::add_block(cryptonote::network_type nettype, const cryptonote::block &block, const std::vector<cryptonote::transaction> &txs, std::vector<std::tuple<std::string, uint64_t>> contributors)
{

  auto hf_version = block.major_version;
  if (hf_version < cryptonote::network_version_19)
    return true;

  bool contains_coinbase = false;
  for(auto & tx : txs)
  {
    if(is_coinbase(tx))
    {
      //TODO sean - reduce the amounts in the database by the coinbase payment
      contains_coinbase = true;
    }
  }

  std::vector<std::tuple<std::string, uint64_t>> payments = calculate_rewards(block, contributors);

  return add_sn_payments(nettype, payments);
}


bool BlockchainSQLite::pop_block(cryptonote::network_type nettype, const cryptonote::block &block, const std::vector<cryptonote::transaction> &txs, std::vector<std::tuple<std::string, uint64_t>> contributors)
{
  auto hf_version = block.major_version;
  //if (hf_version <= cryptonote::network_version_19)
    //return true;

  std::vector<std::tuple<std::string, uint64_t>> payments = calculate_rewards(block, contributors);

  return subtract_sn_payments(nettype, payments);
}

bool BlockchainSQLite::validate_batch_sn_reward_tx(uint8_t hf_version, uint64_t blockchain_height, cryptonote::transaction const &tx, std::string *reason)
{
  return true;
}

}
