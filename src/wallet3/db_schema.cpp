#include "db_schema.hpp"

namespace wallet
{
  // FIXME: BLOB or TEXT for binary data below?
  void
  create_schema(SQLite::Database& db)
  {
    if (db.tableExists("outputs"))
      return;

    db.exec(
        R"(
          CREATE TABLE outputs (
            id INTEGER PRIMARY KEY,
            amount INTEGER,
            output_index INTEGER,
            unlock_time INTEGER,
            block_height INTEGER,
            block_time INTEGER,
            spending BOOLEAN,
            spent_height INTEGER,
            spent_time INTEGER,
            tx INTEGER,
            FOREIGN KEY(tx) REFERENCES transactions(id),
            key BLOB,
            rct_mask BLOB,
            key_image BLOB,
            subaddress_major INTEGER,
            subaddress_minor INTEGER,
            FOREIGN KEY(subaddress_major, subaddress_minor) REFERENCES subaddresses(major_index, minor_index)
          );

          CREATE TABLE blocks (
            id INTEGER PRIMARY KEY,
            hash TEXT
          );

          CREATE TABLE transactions (
            id INTEGER PRIMARY KEY,
            hash TEXT
          );

          -- will default scan many subaddresses, even if never used, so it is useful to mark
          -- if they have been used (for culling this list later, perhaps)
          CREATE TABLE subaddresses (
            major_index INTEGER,
            minor_index INTEGER,
            used BOOLEAN,
            PRIMARY KEY(major_index, minor_index)
          );

          -- CHECK (id = 0) restricts this table to a single row
          CREATE TABLE metadata (
            id INTEGER PRIMARY KEY CHECK (id = 0),
            db_version INTEGER,
            balance INTEGER,
            unlocked_balance INTEGER,
            last_scan_height INTEGER
          );


          -- insert metadata row as default
          INSERT INTO metadata VALUES (NULL,0,0,0);
        )");
  }

}  // namespace wallet
