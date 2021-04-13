#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>
#include <functional>
#include <random>
#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <ctime>
#include <fstream>
#include <regex>
#include <sstream>
#include <cmath>
#include <queue>
#include <unordered_set>


struct TooBig : public std::exception {
	const char *what () const throw () {
		return "map should be at least size 3";
	}
};

struct TooLittle : public std::exception {
	const char *what () const throw () {
		return "map is too big for me to do it in less than 1 minute";
	}
};

struct FirstReadable : public std::exception {
	const char *what () const throw () {
		return "first readable line should be the size of the puzzle";
	}
};

class TextException : public std::exception {
public:
	std::string data;

	TextException() {
		data = "Unknown text error";
	}

	TextException(std::string string) {
		data = string;
	}

	const char *what () const throw () {
		return data.c_str();
	}
};

class Node {
public:
	Node() {
		this->state = "NULL";
	}

	Node(Node *parent, std::vector<std::vector<int>> node_map, std::string state="ACTIVATED") { 
		this->parent = parent;
		this->state = state;
		this->map = node_map;
		this->g = 0;
		this->h = 0.;
	}

	Node(Node const &n) {
		this->parent = n.parent;
		this->state = n.state;
		this->map = n.map;
		this->g = n.g;
		this->h = n.h;
	}

	Node	&operator=(const Node &other) {
		if (this == &other)
        	return *this;
		this->parent = other.parent;
		this->state = other.state;
		this->map = other.map;
		this->g = other.g;
		this->h = other.h;
		return *this;
	}

	std::vector<std::vector<int>> map;
	int g;
	float h;
	Node *parent;
	std::string state;
};

inline bool operator==(const Node& f, const Node& s){ 
	return f.map == s.map; 
}

inline bool operator<(const Node& f, const Node& s) {
	if (f.h != s.h)
		return f.h < s.h;
	return f.g < s.g;
}

inline bool operator>(const Node& f, const Node& s) {
	return s < f;
}

std::size_t p = 2733851927;
std::size_t q = 2963742329;

template<> struct std::hash<Node> {
    std::size_t operator()(Node const &n) const noexcept {
		size_t h = p;
		int map_size = n.map.size();
		for (int i = 0; i != map_size; ++i) {
			for (int j = 0; j != map_size; ++j) {
				h = (h >> 1) | (h << (sizeof(size_t) * 8 - 1));
				h ^= n.map[i][j] * q;
			}
		}
		h *= q;
		h ^= h >> 16;
		return h;
    }
};

class NPuzzle {
public:
	int size;
	std::vector<std::vector<int>> current_map;
	std::vector<std::vector<int>> solution_map;

	NPuzzle() { }

	NPuzzle(int size) : size(size) {
		random_map();
		init();
	}

	NPuzzle(std::string filename) : size(0) {
		map_from_file(filename);
		init();
	}

	void init() {
		std::cout << "Here the map to solve :" << std::endl;
		print_current_map();
		generate_solution();
		std::cout << "Here the solution we aim for :" << std::endl;
		print_solution();
	}

	void	map_from_file(std::string filename) {
		int i = 0;
		std::string line;
		std::fstream map_file(filename);
		if (!map_file.is_open()) {
			std::cout << "input file " << filename << " doesn't exist" << std::endl;
			exit(1);
		}
		std::vector<int> all_num;
		std::vector<std::vector<int>> readed = std::vector<std::vector<int>>();
		std::vector<std::string> line_split;
		std::string token;
		int sp_index;
		size_t pos;
		while (std::getline(map_file, line)) {
			i++;
			pos = 0;
			line = std::regex_replace(line, std::regex("#"), " # ");
			line_split = std::vector<std::string>({});
			while (line[pos] == ' ')
				pos++;
			line.erase(0, pos);
			std::cout << line << std::endl;
			while ((pos = line.find(" ")) != std::string::npos) {
				token = line.substr(0, pos);
				line_split.push_back(token);
				while (line[pos] == ' ')
					pos++;
				line.erase(0, pos);
			}
			if (line.size() > 0)
				line_split.push_back(line);
			if (line_split[0][0] == '#')
				continue;
			else if (this->size == 0 && line_split.size() > 1 && line_split[1][0] != '#') {
				throw FirstReadable();
			} else if (this->size == 0) {
				this->size = std::stoi(line_split[0]);
				if (this->size < 3)
					throw TooLittle();
				else if (this->size > 7)
					throw TooBig();
				continue;
			}
			try {
				sp_index = std::find(line_split.begin(), line_split.end(), "#") - line_split.begin();
			} catch (std::exception & e) {
				sp_index = -1;
			}
			std::vector<int> current_row;
			try {
				int windex = 0;
				while (windex != sp_index && (unsigned int)windex < line_split.size()) {
					current_row.push_back(std::stoi(line_split[windex]));
					all_num.push_back(std::stoi(line_split[windex]));
					windex++;
				}
			} catch (std::exception & e) {
				std::stringstream err;
				err << "You forgot an uncommented string on line " << i << std::endl;
				throw TextException(err.str());
			}
			if (current_row.size() != (unsigned int)this->size) {
				std::stringstream err;
				err << "Each lines should have " << this->size << " values" << std::endl;
				throw TextException(err.str());
			}
			readed.push_back(current_row);
		}
		if (readed.size() != (unsigned int)this->size) {
			std::stringstream err;
			err << "The map must be of size " << this->size << "x" << this->size << std::endl;
			throw TextException(err.str());
		}
		for (int i = 0; i < this->size * this->size; i++) {
			if (std::count(all_num.begin(), all_num.end(), i) != 1) {
				std::stringstream err;
				err << "The map should contain one value of each number in the range [0, " << this->size * this->size << "[" << std::endl;
				throw TextException(err.str());
			}
		}
		this->current_map = readed;
	}

	void	random_map() {
		std::vector<int> all_num(size * size);
		std::iota(all_num.begin(), all_num.end(), 0);
		std::srand(std::time(0));
		std::random_shuffle(all_num.begin(), all_num.end());
		for (int y = 0; y < size; y++)
			current_map.push_back(std::vector<int>(size));
		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				current_map[y][x] = all_num[y * size + x];
			}
		}
	}

	void	generate_solution() {
		solution_map = std::vector<std::vector<int>>();
		for (int y = 0; y < size; y++)
			solution_map.push_back(std::vector<int>(size));
		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				solution_map[y][x] = 0;
			}
		}
		int current = 1;
		int x = 0, y = 0;
		for (int i = 0; i < this->size * 2 - 1; i++) {
			int res = i % 4;
			if (i > 0) {
				x += (res % 2 == 0 ? (res == 0 ? 1 : -1) : 0);
				y += (res % 2 == 1 ? (res == 1 ? 1 : -1) : 0);
			}
			while ((  (x < size && res == 0) 
				   || (y < size && res == 1)
				   || (x >= 0 && res == 2)
				   || (y >= 0 && res == 3)) && solution_map[y][x] == 0) {
				solution_map[y][x] = current;
				current += 1;
				x += (res % 2 == 0 ? (res == 0 ? 1 : -1) : 0);
				y += (res % 2 == 1 ? (res == 1 ? 1 : -1) : 0);
			}
			x += (res % 2 == 0 ? (res == 0 ? -1 : 1) : 0);
			y += (res % 2 == 1 ? (res == 1 ? -1 : 1) : 0);
		}
		solution_map[y][x] = 0;
	}

	void	print_solution() {
		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				std::cout << solution_map[y][x] << "  ";
			}
			std::cout << std::endl;
		}
	}

	void	print_current_map() {
		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				std::cout << current_map[y][x] << "  ";
			}
			std::cout << std::endl;
		}
	}

	void	print_map(std::vector<std::vector<int>> data) {
		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				std::cout << data[y][x] << "  ";
			}
			std::cout << std::endl;
		}
	}

	int inversion_num() {
		std::vector<int> ordered_map;
		int x = 0, y = 0;
		for (int i = 0; i < this->size * 2 - 1; i++) {
			int res = i % 4;
			if (i > 0) {
				x += (res % 2 == 0 ? (res == 0 ? 1 : -1) : 0);
				y += (res % 2 == 1 ? (res == 1 ? 1 : -1) : 0);
			}
			while ((  (x < size && res == 0) 
				   || (y < size && res == 1)
				   || (x >= 0 && res == 2)
				   || (y >= 0 && res == 3)) && std::count(ordered_map.begin(), ordered_map.end(), current_map[y][x]) == 0) {
				ordered_map.push_back(current_map[y][x]);
				x += (res % 2 == 0 ? (res == 0 ? 1 : -1) : 0);
				y += (res % 2 == 1 ? (res == 1 ? 1 : -1) : 0);
			}
			x += (res % 2 == 0 ? (res == 0 ? -1 : 1) : 0);
			y += (res % 2 == 1 ? (res == 1 ? -1 : 1) : 0);
		}
		int reverse_global = 0;
		int curr_ind = 0;
		std::unordered_map<float, float> map_index;
		for (unsigned int i = 0; i < ordered_map.size(); i++)
			map_index.insert({ordered_map[i], i});
		for (std::unordered_map<float, float>::iterator it = map_index.begin(); it != map_index.end(); ++it) {
			curr_ind = map_index[it->first];
			for (int smaller = 1; smaller < it->first; smaller++)
				if (map_index[smaller] > curr_ind)
					reverse_global += 1;
		}
		return reverse_global;
	}

	bool is_solvable() {
		return !(inversion_num() % 2);
	}

	float hamming(std::vector<std::vector<int>> data, std::string _type="hamming") {
		(void)_type;
		int hamming = 0;
		for (int _y = 0; _y < size; _y++) {
			for (int _x = 0; _x < size; _x++) {
				hamming += data[_y][_x] == solution_map[_y][_x] ? 0 : 1 ;		
			}
		}
		return hamming;
	}

	float manhattan_loc(int x, int y, std::vector<std::vector<int>> data) {
		int sx = -1, sy;
		for (int _y = 0; _y < size; _y++) {
			for (int _x = 0; _x < size; _x++) {
				if (solution_map[_y][_x] == data[y][x]) {
					sx = _x;
					sy = _y;
					break;
				}
			} if (sx >= 0) 
				break;
		}
		return std::abs(x - sx) + std::abs(y - sy);
	}

	float euclidian_loc(int x, int y, std::vector<std::vector<int>> data) {
		int sx = -1, sy;
		for (int _y = 0; _y < size; _y++) {
			for (int _x = 0; _x < size; _x++) {
				if (solution_map[_y][_x] == data[y][x]) {
					sx = _x;
					sy = _y;
					break;
				}
			} if (sx >= 0) 
				break;
		}
		return sqrtf(powf(x - sx, 2) + powf(y - sy, 2));
	}

	typedef float (NPuzzle::*fptr)(int, int, std::vector<std::vector<int>>);
	typedef float (NPuzzle::*fptr2)(std::vector<std::vector<int>>,std::string);

	float distance(std::vector<std::vector<int>> data, std::string _type="manhattan") {
		fptr dist_funct;
		if (_type == "manhattan")
			dist_funct = manhattan_loc;
		else
			dist_funct = euclidian_loc;
		float sum = 0;
		for (int _y = 0; _y < size; _y++) {
			for (int _x = 0; _x < size; _x++) {
				sum += (this->*dist_funct)(_x, _y, data);
			}
		}
		return sum;
	}

	std::pair<std::vector<std::vector<std::vector<int>>>, std::pair<int, int>> solve(std::string metric="manhattan") {
	//void solve(std::string metric="manhattan") {
		fptr2 metric_fn;
		if (metric == "hamming")
			metric_fn = hamming;
		else
			metric_fn = distance;
		
		std::cout << metric << " selected" << std::endl;
		float start_metric = (this->*metric_fn)(current_map, metric);
		Node start_node = Node(nullptr, current_map);
		start_node.g = 0;
		start_node.h = start_metric;
		Node end_node = Node(nullptr, solution_map);
		end_node.g = 0;
		end_node.h = 0.;

		std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_list;
		open_list.push(start_node);
		std::unordered_set<Node> closed_set;
		std::unordered_map<Node, Node> open_set;
		open_set[start_node] = start_node;

		Node current_node;
		while (open_list.size()) {
			while (true) {
				try {
					current_node = open_list.top();
					open_list.pop();
				} catch (std::exception &e) {
					current_node = Node();
					break;
				}
				open_set.erase(current_node);
				closed_set.emplace(current_node);
				break;
			}
			if (current_node.state == "NULL") {
				throw TextException("All path has been explored, there is no solution");
			}

			if (current_node == end_node) {
				Node *current = new Node(current_node);
				std::vector<std::vector<std::vector<int>>> path;
				std::cout << "puzzle solved !! :" << std::endl;
				while (current != nullptr) {
					path.push_back(current->map);
					current = current->parent;
				}
				std::reverse(path.begin(), path.end());
				/*for (std::vector<std::vector<int>> pmap : path) {
					print_map(pmap);
				}*/
				return {path, {open_set.size(), closed_set.size()}};
			}
			
			std::vector<Node> children = std::vector<Node>();
			int x = -1, y;
			for (int _y = 0; _y < size; _y++) {
				for (int _x = 0; _x < size; _x++) {
					if (current_node.map[_y][_x] == 0) {
						x = _x;
						y = _y;
						break;
					}
				} if (x >= 0) 
					break;
			}
			
			for (int a : {-1, 1}) {
				int newx = x + a;
				if (newx >= 0 && newx < size) {
					Node new_node = Node(current_node);
					new_node.map[y][x] = new_node.map[y][newx];
					new_node.map[y][newx] = 0;
					new_node.parent = new Node(current_node);
					children.push_back(new_node);
				}
			} for (int b : {-1, 1}) {
				int newy = y + b;
				if (newy >= 0 && newy < size) {
					Node new_node = Node(current_node);
					new_node.map[y][x] = new_node.map[newy][x];
					new_node.map[newy][x] = 0;
					new_node.parent = new Node(current_node);
					children.push_back(new_node);
				}
			}
			
			for (Node child : children) {
				child.g = current_node.g + 1;
				child.h = (this->*metric_fn)(child.map, metric);
				if (closed_set.find(child) != closed_set.end())
					continue;
				else if (open_set.find(child) != open_set.end()) {
					Node old = open_set[child];
					if (child < old) {
						closed_set.emplace(old);
						open_list.push(child);
						open_set[child] = child;
					}
				} else {
					open_list.push(child);
					open_set[child] = child;
				}
			}
			
		}
		throw TextException("All path has been explored, there is no solution");
	}

};

int main(int ac, char **av) {
	int metric_type = -1;
	bool use_premaid_map = false;
	std::string mapf;
	if (ac > 1) {
		use_premaid_map = true;
		mapf = std::string(av[1]);
	}
	while (metric_type == -1) {
		int in;
		std::cout << "Choose a metric function" << std::endl << "1 - Hamming" << std::endl << "2 - Euclidian" << std::endl << "3 - Manhattan" << std::endl;
		std::cin >> in;
		if (in <= 3 && in > 0)
			metric_type = in;
	}
	NPuzzle pzl;
	if (use_premaid_map) {
		pzl = NPuzzle(mapf);
	} else {
		int map_size = 0;
		while (map_size < 3 || map_size > 6) {
			std::cout << "Choose a size for the map (> 3) " << std::endl;
			std::cin >> map_size;
			if (map_size > 6)
				std::cout << "It is too big for me to do it in less than 1 minute";
		}
		pzl = NPuzzle(map_size);
	}
	if (!pzl.is_solvable()) {
		std::cout << "Puzzle not solvable" << std::endl;
		return 1;
	}
	std::pair<std::vector<std::vector<std::vector<int>>>, std::pair<int, int>> ans;
	if (metric_type == 1) {
		ans = pzl.solve("hamming");
	} else if (metric_type == 2) {
		ans = pzl.solve("euclidian");
	} else {
		ans = pzl.solve("manhattan");
	}
	std::vector<std::vector<std::vector<int>>> path = ans.first;
	for (std::vector<std::vector<int>> map : path) {
		std::cout << std::endl;
		pzl.print_map(map);
	}
	std::cout << "space complexity: " << ans.second.first << " time complexity: " << ans.second.second << std::endl;
	return 0;
}
/*
with map :
4
0  5  9  14
6  7  4  8
12 2  15 1
10 13 11 3

Hamming : space complexity: 8618 time complexity: 8424
Euclidian : space complexity: 2005 time complexity: 1517
Manhattan : space complexity: 3951 time complexity: 2893
*/

/*
with map :
6
0 5 27 4 19 11
7 26 18 10 20 31
9 23 30 12 17 25
22 2 15 35 6 28
8 16 21 29 14 1
24 34 3 32 13 33

Hamming : Too big to test
Euclidian : space complexity: 373160 time complexity: 213622
Manhattan : Too big to test
*/