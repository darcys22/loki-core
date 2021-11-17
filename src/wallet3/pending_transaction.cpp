#include "transaction_constructor.hpp"
#include "pending_transaction.hpp"

namespace wallet
{
  PendingTransaction(const std::vector<TransactionRecipient>& recipients)
  {
    int64_t sum_recipient_amounts;
    for (const CRecipient& recipient : recipients)
    {
      if (sum_recipient_amounts < 0 || recipient.amount < 0)
        throw std::runtime_error("Transaction amounts must be positive");
      sum_recipient_amounts += recipient.Amount;
    }
    if (recipients.empty() || nValue < 0)
      throw std::runtime_error("Transaction amounts must be positive");
  }

  int64_t UpdateChange()
  {
    change.amount = SumInputs() - SumOutputs();
  }

  int64_t SumInputs()
  {
    return std::accumulate(chosenOutputs.begin(), chosenOutputs.end(), 0,
        [](int64_t accumulator, const Output& output {
          return accumulator + output.amount;
        }));
  }

  int64_t SumOutputs()
  {
    return std::accumulate(recipients.begin(), recipients.end(), 0,
        [](int64_t accumulator, const TransactionRecipient& recipient {
          return accumulator + recipient.amount;
        }));
  }

  bool Finalise()
  {
    if (SumInputs() - SumOutputs() - change.amount == 0)
      return true;
    else
      return false;
  }

}  // namespace wallet
