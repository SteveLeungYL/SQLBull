//
// Created by XXX on 9/4/24.
//

#ifndef RSG_CPP_FUZZING_SEQUENCE_QUEUE_H
#define RSG_CPP_FUZZING_SEQUENCE_QUEUE_H

#include "fuzzer_configurations.h"
#include "query_sequence.h"
#include "utils.h"
#include <algorithm>
#include <utility>

class FuzzingSequenceQueue {
public:
    int current_chosen_seq_idx = 0;
    int current_queue_iter = 0;

    // First layer, different tree began from each scratch.
    // Second layer, the same tree derived/mutated from the same root tree.
    vector<vector<QuerySequence*>> all_saved_query_sequences;
    vector<pair<int, double>> all_scores; // <hit_count, reward_score>, mapped to all_saved_query_sequences.

    void append_new_sequence_from_scratch(QuerySequence* in)
    {
#ifdef DEBUG
        if (this->all_saved_query_sequences.size() != this->all_scores.size()) {
            cerr << "Error: this->all_saved_query_sequences.size() != this->all_scores.size()\n\n\n";
            abort();
        }
#endif
        vector<QuerySequence*> new_in;
        new_in.push_back(in);
        all_saved_query_sequences.push_back(new_in);
        all_scores.push_back(pair<int, double>(0, 0.0));
    }

    void append_new_sequence_to_existing_group(QuerySequence* in, bool clear_prev_seq = true)
    {
        if (clear_prev_seq) {
            for (auto& tmp: all_saved_query_sequences[current_chosen_seq_idx]){
                delete tmp;
            }
            in->previous_sequence = nullptr;
            all_saved_query_sequences[current_chosen_seq_idx].clear();
        }

        all_saved_query_sequences[current_chosen_seq_idx].push_back(in);

        // Give reward to the specific group.
        pair<int, double>& reward_pair = all_scores[current_chosen_seq_idx];
        int hc = reward_pair.first;
        double rc = reward_pair.second;
        reward_pair.second = ((double(hc - 1) / double(hc)) * rc + (1.0 / double(hc)) * 0.0);
    }

#define rand_float (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
    [[nodiscard]] QuerySequence* get_random_query_sequence_from_queue(int idx = -1)
    {
        if (idx != -1) {
            vector<QuerySequence*> v_res = this->all_saved_query_sequences[idx];
            this->current_chosen_seq_idx = idx;
            return vector_rand_ele(v_res);
        }

        // Use MAB to choose query sequence group.
        if (rand_float < FuzzerConfigurations::epsilon) {
            // Purely Random Query Sequence.
            this->current_chosen_seq_idx = get_rand_int(this->all_saved_query_sequences.size());
            //            return vector_rand_ele(this->all_saved_query_sequences[current_chosen_seq_idx])->deep_copy();
            return this->all_saved_query_sequences[current_chosen_seq_idx].back();
        } else {
            // choose the highest reward.
            vector<double> v_reward;
            v_reward.reserve(this->all_scores.size());
            for (auto& cur_score : this->all_scores) {
                v_reward.push_back(cur_score.second);
            }
            auto max_iter = max_element(v_reward.begin(), v_reward.end());
            this->current_chosen_seq_idx = std::distance(v_reward.begin(), max_iter); // absolute index of max reward.
            //            return vector_rand_ele(this->all_saved_query_sequences[current_chosen_seq_idx])->deep_copy();
            return this->all_saved_query_sequences[current_chosen_seq_idx].back();
        }
    }

    [[nodiscard]] QuerySequence* get_next_query_sequence_from_queue(int idx = -1)
    {
        // Iterate the queue one by one.
        if (idx != -1) {
            vector<QuerySequence*> v_res = this->all_saved_query_sequences[idx];
            this->current_chosen_seq_idx = idx;
            return vector_rand_ele(v_res);
        }

        if (current_queue_iter == this->all_saved_query_sequences.size()) {
            // Finish a round. Restart the queue.
            current_queue_iter = 0;
        }

        this->current_chosen_seq_idx = current_queue_iter++;
        return this->all_saved_query_sequences[current_chosen_seq_idx].back();
    }

    [[nodiscard]] int get_saved_query_sequence_size()
    {
        return this->all_saved_query_sequences.size();
    }

    void repeat_previous_query_seq()
    {
        this->current_queue_iter--;
    }

    FuzzingSequenceQueue() {};
    ~FuzzingSequenceQueue()
    {
        for (auto& tmp : this->all_saved_query_sequences) {
            for (auto* cur_query_seq : tmp) {
                delete cur_query_seq;
            }
        }
    }
};

#endif // RSG_CPP_FUZZING_SEQUENCE_QUEUE_H
