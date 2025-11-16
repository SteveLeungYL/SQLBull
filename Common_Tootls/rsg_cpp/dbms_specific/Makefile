clean:
	rm -rf *.output *.o .deps *.gch
	@$(MAKE) clean -C cockroachdb
	@$(MAKE) clean -C duckdb
	@$(MAKE) clean -C sqlite
	@$(MAKE) clean -C mysqldb
	@$(MAKE) clean -C mariadb
	@$(MAKE) clean -C postgresql

cockroachdb:
	@$(MAKE) -C cockroachdb

duckdb:
	@$(MAKE) -C duckdb

sqlite:
	@$(MAKE) -C sqlite

mysqldb:
	@$(MAKE) -C mysqldb

mariadb:
	@$(MAKE) -C mariadb

postgresql:
	@$(MAKE) -C postgresql

DBMS_SUB_DIRS := cockroachdb duckdb mysqldb mariadb sqlite postgresql
.PHONY: $(DBMS_SUB_DIRS)
