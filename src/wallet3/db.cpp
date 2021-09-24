#include "db.hpp"

namespace wallet
{

  namespace
  {

    //FIXME: BLOB or TEXT for binary data below?
    void InitDB(std::shared_ptr<SQLite::Database> db)
    {
      db->exec("CREATE TABLE outputs ("
              "id INTEGER PRIMARY KEY,"
              "amount INTEGER,"
              "output_index INTEGER,"
              "unlock_time INTEGER,"
              "block_height INTEGER,"
              "block_time INTEGER,"
              "spending BOOLEAN,"
              "spent_height INTEGER,"
              "spent_time INTEGER,"
              "tx_hash BLOB,"
              "key BLOB,"
              "rct_mask BLOB,"
              "key_image BLOB,"
              "FOREIGN KEY(subaddress_major, subaddress_minor) REFERENCES subaddresses(major_index, minor_index)"
              ")");

      db->exec("CREATE TABLE blocks ("
              "id INTEGER PRIMARY KEY,"
              "hash BLOB,"
              ")");

      db->exec("CREATE TABLE subaddresses ("
              "major_index INTEGER,"
              "minor_index INTEGER,"
              // will default scan many subaddresses, even if never used, so it is useful to mark
              // if they have been used (for culling this list later, perhaps)
              "used BOOLEAN,"
              "PRIMARY KEY(major_index, minor_index)"
              ")");

      // CHECK (id = 0) restricts this table to a single row
      db->exec("CREATE TABLE metadata ("
              "id INTEGER PRIMARY KEY CHECK (id = 0),"
              "db_version INTEGER,"
              "balance INTEGER,"
              "unlocked_balance INTEGER,"
              "last_scan_height INTEGER,"
              ")");


      // insert metadata row as default
      db->exec("INSERT INTO metadata VALUES (NULL,0,0,0)");

    }


    std::shared_ptr<SQLite::Database> OpenOrCreateDB(std::string_view filename, std::string_view password, bool create)
    {
      auto flags = SQLite::OPEN_READWRITE;
      if (create) flags |= SQLite::OPEN_CREATE;

      auto db = std::make_shared<SQLite::Database>(filename, flags);

      db->key(std::string{password});

      if (create) InitDB(db);

      // TODO: confirm correct schema exists if opening existing db

      return db;
    }


  } // namespace wallet::{anonymous}


  std::shared_ptr<SQLite::Database> CreateDB(std::string_view filename, std::string_view password)
  {
    return OpenOrCreateDB(filename, password, /* create = */ true);
    // TODO: error reporting/handling, e.g. file alredy exists
  }

  std::shared_ptr<SQLite::Database> OpenDB(std::string_view filename, std::string_view password)
  {
    return OpenOrCreateDB(filename, password, /* create = */ false);
    // TODO: error reporting/handling, e.g. catching wrong password, file does not exist
  }

}

