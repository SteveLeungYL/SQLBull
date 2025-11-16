# Installation and Run Instructions

To run specific DBMS fuzzing: 

```bash
cd <DBMS_name>/scripts
bash setup_<DBMS_name>.sh
sudo docker run -it --memory="10g" --memory-swap="10g" sqlbull_<DBMS_name> /bin/bash
# Inside the container. 
python3 run_parallel.py -c <CPU_CORE_START_ID> -n <NUM_OF_CONCURRENT_RUN>.
```