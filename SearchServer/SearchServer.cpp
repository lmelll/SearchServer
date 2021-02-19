#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
	string s;
	getline(cin, s);
	return s;
}

int ReadLineWithNumber() {
	int result;
	cin >> result;
	ReadLine();
	return result;
}

vector<string> SplitIntoWords(const string& text) {
	vector<string> words;
	string word;
	for (const char c : text) {
		if (c == ' ') {
			words.push_back(word);
			word = "";
		}
		else {
			word += c;
		}
	}
	words.push_back(word);

	return words;
}

struct Query {
	vector<string> plus_words;
	vector<string> minus_words;
};
struct Document {
	int id;
	double relevance;
};

class SearchServer {
public:
	void SetStopWords(const string& text) {
		for (const string& word : SplitIntoWords(text)) {
			stop_words_.insert(word);
		}
	}

	void AddDocument(int document_id, const string& document) {
		vector <string> _document = SplitIntoWordsNoStop(document);
		double freqs = 0;
		for (const string& word : _document) {
			freqs = count_if(_document.begin(), _document.end(), [word](auto& documents) {
				return word == documents; });

			freqs = freqs / _document.size();
			word_to_document_freqs_[word].insert({ document_id, freqs });

		}
		document_count_ += 1;
	}

	vector<Document> FindTopDocuments(const string& query) const {
		auto matched_documents = FindAllDocuments(query);

		sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
			return lhs.relevance > rhs.relevance; });
		if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
			matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
		}
		return matched_documents;
	}

private:
	map<string, map<int, double>> word_to_document_freqs_;
	set<string> stop_words_;
	int document_count_ = 0;

	Query ParseQuery(const string& query) const {
		vector<string> _query = SplitIntoWordsNoStop(query);
		Query parse_query;
		for (auto& word : _query) {
			if (word[0] == '-') {
				word = word.substr(1);
				if (count_if(stop_words_.begin(), stop_words_.end(), [word](const string& stop_word) {return word != stop_word; })) {
					parse_query.minus_words.push_back(word);
				}
			}
			else {

				parse_query.plus_words.push_back(word);
			}

		}

		return parse_query;
	}

	vector<string> SplitIntoWordsNoStop(const string& text) const {
		vector<string> words;
		for (const string& word : SplitIntoWords(text)) {
			if (stop_words_.count(word) == 0) {
				words.push_back(word);
			}
		}
		return words;
	}

	vector<Document> FindAllDocuments(const string& query) const {
		const Query query_words = ParseQuery(query);
		double idf = 0;
		double count_idf = 0;
		double tf_idf = 0;
		map <int, double> document_to_relevance;
		for (auto& query_word : query_words.plus_words) {
			if (word_to_document_freqs_.count(query_word) == 0) {
				continue;
			}
			for (auto [document_id, TF] : word_to_document_freqs_.at(query_word)) {
				count_idf = word_to_document_freqs_.at(query_word).size();

				idf = log(document_count_ / count_idf);
				tf_idf = TF * idf;
				document_to_relevance[document_id] += tf_idf;

			}
		}

		vector<Document> matched_documents;
		for (auto [document_id, relevance] : document_to_relevance) {
			matched_documents.push_back({ document_id, relevance });
		}

		for (auto& word : query_words.minus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			for (const auto [document_id, TF] : word_to_document_freqs_.at(word)) {
				matched_documents.erase(remove_if(matched_documents.begin(), matched_documents.end(), [document_id](auto& document) { return document.id == document_id; }), matched_documents.end());
			}
		}



		return matched_documents;
	}
};

SearchServer CreateSearchServer() {
	SearchServer search_server;
	search_server.SetStopWords(ReadLine());

	const int document_count = ReadLineWithNumber();
	for (int document_id = 0; document_id < document_count; ++document_id) {
		search_server.AddDocument(document_id, ReadLine());
	}

	return search_server;
}


int main() {
	const SearchServer search_server = CreateSearchServer();

	const string query = ReadLine();
	for (auto [document_id, relevance] : search_server.FindTopDocuments(query)) {
		cout << "{ document_id = " << document_id << ", relevance = " << relevance << " }" << endl;
	}
}