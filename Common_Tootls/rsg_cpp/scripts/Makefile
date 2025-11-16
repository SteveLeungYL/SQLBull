clean:
	rm -f assets/ir_types_custom.h     
	rm -f assets/ir_types_mapping.txt  
	rm -f assets/duckdb_grammar_modi.y 
	rm -f assets/cockroach_sql_modi.y  
	rm -f assets/sqlite_parser.y  
	rm -f assets/mysql_grammar_modi.y
	rm -f assets/mariadb_grammar_modi.y
	rm -f assets/postgresql_gram_modi.y

cockroachdb:
	python3 generate_ir_types_cockroachdb.py
	$(MAKE) -e post_processing

duckdb:
	python3 generate_ir_types_duckdb.py
	$(MAKE) -e post_processing

sqlite:
	python3 generate_ir_types_sqlite.py
	$(MAKE) -e post_processing

mysqldb:
	python3 generate_ir_types_mysql.py
	python3 remove_action.py assets/mysql_grammar.y
	$(MAKE) -e post_processing

mariadb:
	python3 generate_ir_types_mariadb.py
	python3 remove_action.py assets/mariadb_grammar.y
	python3 mariadb_parser_post_processing.py
	$(MAKE) -e post_processing

postgresql:
	python3 generate_ir_types_postgresql.py
	python3 remove_action.py assets/postgresql_gram.y
	$(MAKE) -e post_processing

post_processing:
	cp assets/ir_types_custom.h ../headers

SUB_DIRS := assets
.PHONY: $(SUB_DIRS)