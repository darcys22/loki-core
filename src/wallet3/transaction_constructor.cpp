#include "transaction_constructor.hpp"
#include "pending_transaction.hpp"
#include <sqlitedb/database.hpp>

namespace wallet
{
  PendingTransaction
  TransactionConstructor::CreateTransaction(const std::vector<TransactionRecipient>& recipients, int64_t feePerKB) const
  {
    PendingTransaction txNew(recipients);
    std::cout << __FILE__ << ":" << __LINE__ << " (" << __func__ << ") TODO sean remove this - " << "AAAAAAAAAA" << " - debug" << std::endl;
    SelectInputsAndFinalise(txNew);
    return txNew;
  }
  
  // SelectInputs will choose some available unspent outputs from the database and allocate to the transaction
  // can be called multiple times and will add until enough is sufficient
  void
  TransactionConstructor::SelectInputs(PendingTransaction& ptx) const
  {
    std::cout << __FILE__ << ":" << __LINE__ << " (" << __func__ << ") TODO sean remove this - " << "AAAAAAAAAA" << " - debug" << std::endl;
    // Fail early
    int64_t estimated_fee = EstimateFee();
    //int64_t estimated_fee = estimate_fee(2, fake_outs_count, min_outputs, extra.size(), clsag, base_fee, fee_percent, fixed_fee,     fee_quantization_mask);
    int64_t transaction_total = ptx.SumOutputs() + estimated_fee;
    int64_t wallet_balance = db->prepared_get<int>("SELECT SUM(amount) FROM outputs");

    // Check that we actually have enough in the outputs to build this transaction.
    if (wallet_balance < transaction_total)
      throw std::runtime_error("Insufficient Wallet Balance");


    int64_t shortfall = transaction_total- ptx.SumInputs();

    // If we already have enough inputs return
    if (shortfall < 0)
      return;

    // Prefer a single output if suitable
    SQLite::Statement single_output{db->db, "SELECT * FROM outputs WHERE amount > ? ORDER BY amount ASC LIMIT 1"};
    single_output.bind(1, shortfall);
    while (single_output.executeStep())
    {
      std::cout << single_output.getColumn(1).getInt64() << std::endl;
      //ptx.chosenOutputs.emplace_back(
          //single_output.getColumn(1).getInt64(),
          //single_output.getColumn(2).getInt64(),
          //single_output.getColumn(3).getInt64(),
          //single_output.getColumn(4).getInt64(),
          //single_output.getColumn(5).getInt64(),
          //single_output.getColumn(6),
          //single_output.getColumn(7).getInt64());
      //ptx.UpdateChange();
      return;
    }

    // Else select some random outputs
    // TODO sean amount > dust to prevent increasing fee higher than input amount
    SQLite::Statement many_outputs{db->db, "SELECT * FROM outputs ORDER BY amount"};
    while (shortfall > 0 && many_outputs.executeStep())
    {
      if (many_outputs.executeStep()) {
        int64_t output_amount = many_outputs.getColumn(1).getInt64();
        std::cout << output_amount << std::endl;
        //ptx.chosenOutputs.emplace_back(
            //many_outputs.getColumn(1).getInt64(),
            //many_outputs.getColumn(2).getInt64(),
            //many_outputs.getColumn(3).getInt64(),
            //many_outputs.getColumn(4).getInt64(),
            //many_outputs.getColumn(5).getInt64(),
            //many_outputs.getColumn(6),
            //many_outputs.getColumn(7).getInt64());
        shortfall -= output_amount;
      } else {
        throw std::runtime_error("Insufficient Wallet Balance");
      }
    }
    ptx.UpdateChange();

  }

  void
  TransactionConstructor::SelectInputsAndFinalise(PendingTransaction& ptx) const
  {
    while (true)
    {
      if (ptx.Finalise())
        break;
      else
        SelectInputs(ptx);
    }
  }

  int64_t 
  TransactionConstructor::EstimateFee() const
  {
    return 0;
  }

}  // namespace wallet
