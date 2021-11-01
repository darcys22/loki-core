#include <catch2/catch.hpp>

#include <wallet3/db_schema.hpp>
#include <sqlitedb/database.hpp>

TEST_CASE("DB Schema", "[wallet,db]")
{
  db::Database db{std::filesystem::path(":memory:"), ""};

  SECTION("db schema creation succeeds")
  {
    REQUIRE_NOTHROW(wallet::create_schema(db.db));
  }

  // will not throw if schema is already set up
  REQUIRE_NOTHROW(wallet::create_schema(db.db));

  REQUIRE(db.db.tableExists("blocks"));

  SECTION("metadata table does not allow row insertion")
  {
    REQUIRE_THROWS(db.prepared_exec("INSERT INTO metadata VALUES(1,0,0,0,0);"));
  }

  SECTION("Insert and fetch block")
  {
    REQUIRE_NOTHROW(db.prepared_exec("INSERT INTO blocks VALUES(?,?,?);", 42, "Adams", 0));

    std::string hash;
    REQUIRE_NOTHROW(hash = db.prepared_get<std::string>("SELECT hash FROM blocks WHERE height = 42"));

    REQUIRE(hash == "Adams");
  }

  SECTION("Insert and fetch transaction")
  {
    REQUIRE_NOTHROW(db.prepared_exec("INSERT INTO blocks VALUES(?,?,?);", 0, "foo", 0));
    REQUIRE_NOTHROW(db.prepared_exec("INSERT INTO transactions VALUES(?,?,?);", 42, 0, "footx"));

    std::tuple<std::string, int> res;
    REQUIRE_NOTHROW(res = db.prepared_get<std::string, int>("SELECT hash,block FROM transactions WHERE id = 42"));

    const auto& [hash,block] = res;
    REQUIRE(hash == "footx");
    REQUIRE(block == 0);
  }

  SECTION("Insert and fetch key image")
  {
    REQUIRE_NOTHROW(db.prepared_exec("INSERT INTO key_images VALUES(?,?);", 0, "key_image"));

    std::string image;
    REQUIRE_NOTHROW(image = db.prepared_get<std::string>("SELECT key_image FROM key_images WHERE id = 0"));

    REQUIRE(image == "key_image");

    // key image is unique
    REQUIRE_THROWS(db.prepared_exec("INSERT INTO key_images VALUES(?,?);", 0, "key_image"));
  }
}

TEST_CASE("DB Triggers", "[wallet,db]")
{
  db::Database db{std::filesystem::path(":memory:"), ""};

  REQUIRE_NOTHROW(wallet::create_schema(db.db));
  REQUIRE(db.db.tableExists("blocks"));

  REQUIRE_NOTHROW(db.prepared_exec("INSERT INTO blocks VALUES(?,?,?);", 0, "foo", 0));
  REQUIRE_NOTHROW(db.prepared_exec("INSERT INTO transactions VALUES(?,?,?);", 0, 0, "footx"));
  REQUIRE_NOTHROW(db.prepared_exec("INSERT INTO key_images VALUES(?,?);", 0, "key_image"));
  REQUIRE_NOTHROW(db.prepared_exec("INSERT INTO outputs VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?);",
        0, 42, 0, 0, 0, 0, false, 0, 0, "out_key", "rct_mask", 0, 0, 0));

  SECTION("Confirm output insert triggers")
  {
    REQUIRE(db.prepared_get<int64_t>("SELECT amount FROM outputs WHERE id = 0") == 42);
    REQUIRE(db.prepared_get<int64_t>("SELECT balance FROM metadata WHERE id = 0") == 42);
  }

  REQUIRE_NOTHROW(db.prepared_exec("INSERT INTO blocks VALUES(?,?,?);", 1, "bar", 0));
  REQUIRE_NOTHROW(db.prepared_exec("INSERT INTO transactions VALUES(?,?,?);", 1, 1, "bartx"));
  REQUIRE_NOTHROW(db.prepared_exec("INSERT INTO spends VALUES(?,?,?,?);", 0, 0, 1, 1));

  SECTION("Confirm spend insert triggers")
  {
    REQUIRE(db.prepared_get<int64_t>("SELECT balance FROM metadata WHERE id = 0") == 0);
    REQUIRE(db.prepared_get<int64_t>("SELECT spent_height FROM outputs WHERE key_image = 0") == 1);
  }


  REQUIRE(db.prepared_get<int>("SELECT COUNT(*) FROM transactions;") == 2);

  // should cascade and remove the transactions with block = 1 inserted above
  REQUIRE_NOTHROW(db.prepared_exec("DELETE FROM blocks WHERE height = 1"));

  SECTION("Output spend removal trigger")
  {
    REQUIRE(db.prepared_get<int>("SELECT COUNT(*) FROM transactions;") == 1);

    // balance should be 42, and the spend should be removed.
    // existing output's spend height should be back to 0.
    REQUIRE(db.prepared_get<int>("SELECT COUNT(*) FROM spends;") == 0);
    REQUIRE(db.prepared_get<int64_t>("SELECT balance FROM metadata WHERE id = 0") == 42);
    REQUIRE(db.prepared_get<int64_t>("SELECT spent_height FROM outputs WHERE key_image = 0") == 0);
  }

  REQUIRE_NOTHROW(db.prepared_exec("DELETE FROM blocks WHERE height = 0"));

  SECTION("Output removal trigger")
  {
    REQUIRE(db.prepared_get<int>("SELECT COUNT(*) FROM transactions;") == 0);

    // balance should be 0, and the output should be removed.
    // key image should be removed as nothing references it.
    REQUIRE(db.prepared_get<int>("SELECT COUNT(*) FROM outputs;") == 0);
    REQUIRE(db.prepared_get<int64_t>("SELECT balance FROM metadata WHERE id = 0") == 0);
    REQUIRE(db.prepared_get<int64_t>("SELECT COUNT(*) FROM key_images;") == 0);
  }
}


/*
    // SQLiteCpp could throw on a bad query, so make sure the exception caught
    // is the one we expect.
    REQUIRE_THROWS_WITH(db.prepared_get<std::string>("SELECT hash FROM blocks WHERE id = 0"),
        Catch::Matchers::Contains("got no rows"));
*/
