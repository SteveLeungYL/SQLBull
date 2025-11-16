package main

import (
        "github.com/cockroachdb/cockroach/pkg/sql/parser"
        "os"
)

func ParseHelper(inData string) {
        // The return res from parser.Parse is an array of Statement structure.
        // Each element is one statement from the query.
        _, _ = parser.Parse(inData)
}

func main() {
  rawIn, err := os.ReadFile("sql_in.sql")
  if err != nil {
    return
  }
  testSql := string(rawIn)
  ParseHelper(testSql)
}