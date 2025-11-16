#ifndef SRC_QUERY_PLAN_HANDLER_H
#define SRC_QUERY_PLAN_HANDLER_H

#include <string>
#include <vector>

std::vector<std::string> retrieve_query_plan(const std::string cmd_in, const std::string in);
std::vector<std::string> retrieve_query_plan_opt(const std::string in);

#endif //SRC_QUERY_PLAN_HANDLER_H
