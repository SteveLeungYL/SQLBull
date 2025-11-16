package covtest

import (
	"context"
	"fmt"
	"os"
	"strings"
	"testing"
    "encoding/binary"
	"globalcov"
	"github.com/cockroachdb/cockroach/pkg/base"
	"github.com/cockroachdb/cockroach/pkg/testutils/serverutils"
	"github.com/cockroachdb/cockroach/pkg/testutils/sqlutils"
)

var FORKSRV_FD uintptr = 198

const maxQueryExec int = 100000

const initQuery = `
RESET ALL;
SET sql_safe_updates = false;
SET CLUSTER SETTING debug.panic_on_failed_assertions = true;
SET CLUSTER SETTING diagnostics.reporting.enabled    = false;
SET CLUSTER SETTING diagnostics.reporting.send_crash_reports = false;
SET CLUSTER SETTING sql.metrics.statement_details.enabled = 'off';
SET CLUSTER SETTING sql.metrics.statement_details.plan_collection.enabled = 'off';
SET CLUSTER SETTING sql.stats.automatic_collection.enabled = 'off';
SET CLUSTER SETTING timeseries.storage.enabled = 'off';
SET experimental_enable_temp_tables = 'on';
`

const cleanupQuery = `
ROLLBACK;
RESET ALL;
SET sql_safe_updates = false;
`

// Execute the query string, return the results as string.
func executeQuery(sqlStr string, sqlRun *sqlutils.SQLRunner) string {

	// Separate the input queries. If single query fails,
	// the query sequence execution can still continue.
	testingQueryArray := strings.Split(sqlStr, "\n")
	resStr := ""

	for _, testingQuery := range testingQueryArray {

		// Higher level query running command. Will throw error if failed: resRows := sqlRun.Query(t, testingQuery)

		// QueryContext is the most fundamental function to execute one query
		// and returns results. If error happens, this function would only
		// return the error rather than throw a Fatal Error.
		resRows, err := sqlRun.DB.QueryContext(context.Background(), testingQuery)

		if err != nil {
			resStr += fmt.Sprintf("%s\n", err)
		} else {
			tmpR, err := sqlutils.RowsToStrMatrix(resRows)

			if err != nil {
				// Should not happen.
			} else {
				r := sqlutils.MatrixToStr(tmpR)
				resStr += fmt.Sprintf("%s", r)
			}
		}
	}

	return resStr

}

func TestCov(t *testing.T) {

	// The Server startup params can be reused multiple times.
	params := base.TestServerArgs{}

	// Setup the server testing env.
	s, sqlDB, _ := serverutils.StartServer(t, params)
	defer s.Stopper().Stop(context.Background())

	sqlRun := sqlutils.MakeSQLRunner(sqlDB)

    // Initialize the Database transaction. 
    executeQuery(initQuery, sqlRun)

	controlPipe := os.NewFile(FORKSRV_FD, "pipe")
	tmpCtrlRead := []byte{0, 0, 0, 0}
	// Status Write Pipe.
	statusPipe := os.NewFile(FORKSRV_FD+1, "pipe")

	// Forkserver, stop the process right before the query processing.
	// Notify the fuzzer that the server is ready.
	_, _ = statusPipe.Write([]byte{0, 0, 0, 0})
	_ = statusPipe.Sync()

	for per_cycle := 0; per_cycle < maxQueryExec; per_cycle++ {

        // Control Read Pipe.
		// Wait for the input signal.
		_, err := controlPipe.Read(tmpCtrlRead)
		if err != nil {
			t.Fatal("controlPipe reading failed.\n")
		}

        tmpCtrlReadInt := binary.LittleEndian.Uint32(tmpCtrlRead)

        // DEBUG:
        //fmt.Printf("\n\n\nGetting tmpCtrlReadInt: %d \n\n\n", tmpCtrlReadInt)

        if tmpCtrlReadInt == 2 {
            //fmt.Printf("\n\n\nExit the current CockroachDB server. \n\n\n")
            break;
        }

		if tmpCtrlReadInt == 3 {
			// Log the code coverage.
			globalcov.SaveGlobalCov()
			// Clean up the coverage log.
			globalcov.ResetGlobalCov()

			// Notify the fuzzer that the coverage output has succeeded.
			_, err := statusPipe.Write([]byte{0, 0, 0, 0})
			_ = statusPipe.Sync()
			if err != nil {
				t.Fatalf("StatusPipe writing failed. Error: %s", err.Error())
			}

			// Avoid exiting the loop in this situation.
			per_cycle--;
			continue;
		}

        if tmpCtrlReadInt == 1 {
            // Reset the database.
            executeQuery(cleanupQuery, sqlRun)
        } // else. 

		// Read query from local file.
		inRaw, err := os.ReadFile("./input_query.sql")
		if err != nil {
			t.Fatal("input_query.sql not existed. ")
		}

		outFile, outErr := os.Create("./query_res_out.txt")
		if outErr != nil {
			panic(outErr)
		}


		// Execute the query
        if string(inRaw) != "" {
            queryRes := executeQuery(string(inRaw), sqlRun)
            _, _ = outFile.WriteString(queryRes)
        } else {
            _, _ = outFile.WriteString("")
        }

		outFile.Close()

		if per_cycle < (maxQueryExec - 1) {
			// Notify the fuzzer that the execution has succeed.
			_, err := statusPipe.Write([]byte{0, 0, 0, 0})
			_ = statusPipe.Sync()
			if err != nil {
				t.Fatalf("StatusPipe writing failed. Error: %s", err.Error())
			}
		} else {
			break
		}
	}

	// Notify the fuzzer that the execution has succeed, and the CockroachDB needs rerun.
	_, _ = statusPipe.Write([]byte{1, 0, 0, 0})
	_ = statusPipe.Sync()
}

