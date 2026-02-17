#include "csv.hpp"
#include <fstream>
#include <iostream>
#include <exception>
#include <vector>
#include <unordered_map>
#include <stdio.h>
#include <filesystem>
#include <algorithm>
#include <pthread.h>
#include <chrono>

namespace fs = std::filesystem;
using namespace std;
using namespace csv;

struct Row {
    string Name;
    string UID;
    size_t Number_attempts;
    size_t Duration_seconds;
    string Question;
};

const size_t ALPHA = 1; //match score
const size_t BETA = 0; //mismatch score
const size_t GAMMA = 0; //gap score
const size_t NUM_THREADS = 4; //number of threads 
const double THRES_MULT = 3; //number of standard deviations above the average to place threshold

size_t num_comp = 0; //global to keep track of number computed

//Computing table of match scores will be threaded
struct ThreadContext {
    char* str1;
    char* str2;
    size_t i;
    size_t j;
    size_t total_comparisons;
    size_t* max_score;
    vector<vector<size_t>>* match_scores;
    size_t* match_score_mean;
    pthread_mutex_t* mean_mutex;
};

//DP algorithm to find max number of matching characters
//Using c_string to eliminate overhead
size_t match_score(char* str1, size_t str1_len, char* str2, size_t str2_len) {
	if(str1_len < 1 || str2_len < 1) {
		return 0;
	}

	size_t* prev_line = new size_t[str1_len + 1];
	size_t* curr_line = new size_t[str1_len + 1];

	for(size_t i = 0; i < str1_len+1; i++) {
		prev_line[i] = i * GAMMA;
	}

	for(size_t i = 0; i < str2_len + 1; i++) {
		curr_line[0] = 0;
		for(size_t j = 1; j < str1_len + 1; j++) {
			size_t v_gap = curr_line[j - 1] + GAMMA;
			size_t h_gap = prev_line[j] + GAMMA;
			size_t mm_gap = prev_line[j - 1] + (str2[i-1] == str1[j - 1] ? ALPHA : BETA);
			size_t temp = mm_gap > h_gap ? mm_gap : h_gap;
			curr_line[j] = temp > v_gap ? temp : v_gap;
		}	
		size_t* tmp = prev_line;
        prev_line = curr_line;
        curr_line = tmp;
	}

	size_t dist = curr_line[str1_len];
	delete[] prev_line;
    delete[] curr_line;

	return dist;
}

//Thread function that gets match score table
void* compare_pair(void* arg_ptr) {
    ThreadContext* args = (ThreadContext*)arg_ptr;
    size_t score = match_score(args->str1, strlen(args->str1), args->str2, strlen(args->str2));

    (*args->match_scores)[args->i][args->j] = score;
    (*args->match_scores)[args->j][args->i] = score;

    pthread_mutex_lock(args->mean_mutex);
    *args->match_score_mean += score;
    num_comp++;
    cout << "\r" << "Processed: " << num_comp << "/" << args->total_comparisons;
    if(score > *args->max_score) {
        *args->max_score = score;
    }
    pthread_mutex_unlock(args->mean_mutex);

    delete args;
    return nullptr;
}

int main(int argc, char* argv[]) {

    if(argc != 3) {
    	cout << "Usage: ./bin/exec instance_questions_file folder_name" << endl;
    	return -1;
    }
    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    double total_duration = 0.0;
    double curr_dur = 0.0; //temporary storage for total


// Section 1: Read student questions times and find suspicious ones
	CSVReader reader(argv[1]);

    vector<string> questions = {
    	/* fill with questions needed */ 
    	"SP26-HW1/sp26-hw1-sql-q1",
        "SP26-HW1/sp26-hw1-sql-q2",
        "SP26-HW1/sp26-hw1-sql-q3",
        "SP26-HW1/sp26-hw1-sql-q4",
        "SP26-HW1/sp26-hw1-sql-q5",
        "SP26-HW1/sp26-hw1-sql-q6",
        "SP26-HW1/sp26-hw1-sql-q7",
        "SP26-HW1/sp26-hw1-sql-q8",
        "SP26-HW1/sp26-hw1-sql-q9",
        "SP26-HW1/sp26-hw1-sql-q10",
        "SP26-HW1/sp26-hw1-sql-q11",
        "SP26-HW1/sp26-hw1-sql-q12",
        "SP26-HW1/sp26-hw1-sql-q13",
        "SP26-HW1/sp26-hw1-sql-q14",
        "SP26-HW1/sp26-hw1-sql-q15"
    };

    //map questions to its indices
    unordered_map<string,size_t> question_order;
    for (size_t i = 0; i < questions.size(); i++) {
        question_order[questions[i]] = i;
    }
    cout << "\n[LOG] Calculating Odd Times: ";
    //This uses csv.hpp API to get columns
    vector<Row> filtered_rows;
    for (auto& row : reader) {
        try {
            string name = row["Name"].get<>();
            string uid  = row["UID"].get<>();
            string question = row["Question"].get<>();
            string attempts_str = row["Number attempts"].get<>();
            string duration_str = row["Duration seconds"].get<>();
            string score_str    = row["Question % score"].get<>();

            if (attempts_str.empty() || duration_str.empty() || score_str.empty()) {
                continue; // skip empty numeric fields
            }

            size_t attempts = stoi(attempts_str);
            size_t duration = stoi(duration_str);
            size_t score = stoi(score_str);

            //Conditions for suspicious times, change accordingly
            bool cond1 = (question_order.find(question) != question_order.end()) &&
                         score == 100 &&
                         (double(duration)/attempts <= 25.0) &&
                         attempts < 10;

            bool cond2 = (question_order.find(question) != question_order.end()) &&
                         attempts <= 2 &&
                         score == 100 &&
                         duration < 120;

            if (cond1 || cond2) {
                filtered_rows.push_back({name, uid, attempts, duration, question});
            }
        } catch (const exception& e) {
            cerr << endl << "[ERROR] Skipping row due to exception: " << e.what() << endl;
            return -1;
        }
    }
    cout << "Done" << endl;

    if(filtered_rows.empty()) {
        cout << "[LOG] No students with suspicious times found" << endl;
    } else {
        // Sort by order in questions
        sort(filtered_rows.begin(), filtered_rows.end(), [&](const Row &a, const Row &b){
            return question_order[a.Question] < question_order[b.Question];
        });

        ofstream odd_times("weird_times.txt");
        if(!odd_times) {
            cerr << "[ERROR] Failed to Create File for Suspicious Times" << endl;
            return -1;
        }
        cout << "[LOG] weird_times.txt Created" << endl;
        string curr_q = filtered_rows[0].Question;
        odd_times << curr_q << "\n" << endl;
        for (auto &r : filtered_rows) {
            if(curr_q != r.Question) {
                curr_q = r.Question;
                odd_times << "\n" << curr_q << "\n" << endl;
            }
            string netid = r.UID.substr(0, r.UID.find('@'));
            odd_times << netid << ":" << (netid.length() > 6 ? "\t" : "\t\t") << "Attempts:" << r.Number_attempts << "\tTime:" << r.Duration_seconds << "\n";
        }
        odd_times.close();
    }
    cout << "[LOG] Odd Response Times Filled" << endl;
    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    curr_dur = chrono::duration_cast<chrono::milliseconds>(end - begin).count()*1.0/1000.0;
    cout << "[LOG] Runtime Duration: " << curr_dur << "[s]" << endl;
    total_duration += curr_dur;
    begin = end;

//Section 2: Run an alignment program that finds students with high matching scores
    
    //Folder of best files needs to be extracted and plugged into exec function
    string path = "./" + string(argv[2]) + "/";
    if(!fs::exists(path)) {
    	cerr << "[ERROR] Invalid Directory of Folders" << endl;
    	return -1;
    }

    //Get student directories and list of net ids
    vector<string> student_dir_list;
    vector<string> net_id_list;
    for (const auto &entry : fs::directory_iterator(path)) {
        if (fs::is_directory(entry.path())) {
        	string curr_student = entry.path().filename().string();
        	net_id_list.push_back(curr_student.substr(0, curr_student.find('@')));
            student_dir_list.push_back(path + curr_student);
        }
    }

    //Everything is sorted so students' netIDs match
    sort(net_id_list.begin(), net_id_list.end(), [](string a, string b) {
        return a < b;
    });
    sort(student_dir_list.begin(), student_dir_list.end(), [](string a, string b) {
        return a < b;
    });

    //Range through each question, and perform tasks accordingly
    for(size_t ques_idx = 0; ques_idx < questions.size(); ques_idx++) {
        
        //Get list of directories(rewrite if schemas change)
    	vector<char*> curr_question_responses;
    	for(auto student : student_dir_list) {
    		for (const auto &entry : fs::directory_iterator(student)) {
    			string curr_path = entry.path().string();
                size_t first = curr_path.find('_');
                size_t second = curr_path.find('_', first + 1);
                size_t third = curr_path.find('_', second + 1);
                if (first == string::npos || second == string::npos || third == string::npos) {
                    continue;   
                }
                // Name of question goes across a '/' character but between 2nd and 3rd '_' character of full directory
                string q = curr_path.substr(second + 1, third - second - 1);
                if(questions[ques_idx] == q) {
                    size_t size = 0;
                    ifstream file(curr_path, std::ios::binary | std::ios::ate);
                    if (!file) {
                        cerr << "[ERROR] Could Not Read Reponse File of Student: " << curr_path.substr(curr_path.find('/', 2), first) << endl;
                        return -1;
                    }
                    size = static_cast<size_t>(file.tellg());
                    file.seekg(0);
                    char* buffer = new char[size + 1];
                    file.read(buffer, size);
                    buffer[size] = '\0';
                    curr_question_responses.push_back(buffer);
                }
    		}
    	}

        size_t len = curr_question_responses.size();
        vector<vector<size_t>> curr_question_match_scores(len, vector<size_t>(len, 0)); // match score table
        
        if(len < 2) {
            return -1;
        }

        size_t match_score_mean = 0;
        size_t total_comparisons = len*(len-1)/2;
        num_comp = 0;
        size_t max_score = 0;

        cout << "\n[LOG] " << questions[ques_idx] << ": " << endl;

        // Use multiple threads to compute entire score table
        // Uses cstring to speed up process
        pthread_t threads[NUM_THREADS];
        size_t active_threads = 0;
        pthread_mutex_t mean_mutex = PTHREAD_MUTEX_INITIALIZER;
        for(size_t i = 0; i < len; i++) {
            for(size_t j = i+1; j < len; j++) {
                ThreadContext* args = new ThreadContext {
                                        curr_question_responses[i],
                                        curr_question_responses[j],
                                        i, j,
                                        total_comparisons,
                                        &max_score,
                                        &curr_question_match_scores,
                                        &match_score_mean,
                                        &mean_mutex
                                    };
                pthread_create(&threads[active_threads++], nullptr, compare_pair, args);
                if(active_threads == NUM_THREADS) {
                    for(size_t t = 0; t < NUM_THREADS; t++) {
                        pthread_join(threads[t], nullptr);
                    }
                    active_threads = 0;
                }
            }
        }
        for(size_t t = 0; t < active_threads; t++) {
            pthread_join(threads[t], nullptr);
        }
        cout << endl;

        match_score_mean /= total_comparisons;
        cout << "Mean: " << match_score_mean << endl;

        size_t floor_stdev = 0;
        for(size_t i = 0; i < len; i++) {
            for(size_t j = i+1; j < len; j++) {
                size_t temp = curr_question_match_scores[i][j];
                size_t diff = (temp > match_score_mean ? temp - match_score_mean : match_score_mean - temp);
                floor_stdev += diff*diff;
            }
        }
        floor_stdev /= total_comparisons;
        size_t root = 1;
        while(root*root <= floor_stdev) {
            root++;
        }
        floor_stdev = root-1;
        cout << "Standard Deviation: " << floor_stdev << endl;
        cout << "Max: " << max_score << endl;

        size_t threshold = match_score_mean + (size_t)(THRES_MULT*floor_stdev);

        //Go through all scores and compute highest matching student pairs and record how often a student's answer was flagged
        unordered_map<string, size_t> net_id_freq_map;
        unordered_map<string, size_t> score_map;
        for(size_t i = 0; i < len; i++) {
            for(size_t j = i+1; j < len; j++) {
                size_t score = curr_question_match_scores[i][j];
                if(score >= threshold) {
                    score_map[(net_id_list[i] + "-" + net_id_list[j])] = score;
                    net_id_freq_map[net_id_list[i]]++;
                    net_id_freq_map[net_id_list[j]]++;
                }
            }
        }

        // highest matching student pairs is sorted based on how high the match score is
        vector<pair<string,size_t>> net_id_freq_vec(net_id_freq_map.begin(), net_id_freq_map.end());
        sort(net_id_freq_vec.begin(), net_id_freq_vec.end(), [](auto &a, auto &b) {
            return a.second > b.second;
        });
        // frequency of student's answer being flagged sorted highest to lowest
        vector<pair<string,size_t>> score_vec(score_map.begin(), score_map.end());
        sort(score_vec.begin(), score_vec.end(), [](auto &a, auto &b) {
            return a.second > b.second;
        });
        
        string qname = questions[ques_idx];
        size_t pos = qname.find_last_of('/');
        std::string safe_name;
        if (pos != string::npos) {
            safe_name = qname.substr(pos + 1);
        } else {
            safe_name = qname;
        }
        std::ofstream high_match(safe_name + "_high_matches.txt");
        if(!high_match) {
            cerr << "[ERROR] Failed to Create Match Table File" << endl;
            goto cleanup;
        }

        //Writing to file
        high_match << "\nFlagged Response Pairs\n";
        for(auto& score: score_vec) {
            high_match << score.first << ": " << to_string(score.second) + "\n";
        }
        high_match << "\nNetID Frequency Map\n";
        for(auto& freq: net_id_freq_vec) {
            high_match << freq.first << ": " << to_string(freq.second) + "\n";
        }
        high_match.close();

        end = std::chrono::steady_clock::now();
        curr_dur = chrono::duration_cast<chrono::milliseconds>(end - begin).count()*1.0/1000.0;
        cout << "[LOG] Runtime Duration: " << curr_dur << "[s]" << endl;
        total_duration += curr_dur;
        begin = end;
        cleanup:
            for(auto& resp : curr_question_responses) {
                delete[] resp;
            }

    }
    cout << "[LOG] Total Runtime Duration: " << total_duration << "[s]" << endl;
    return 0;
}