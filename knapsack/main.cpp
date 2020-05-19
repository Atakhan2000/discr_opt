// использовал метод Branch and Bound

#include<cstdio>
#include<vector>
#include<cassert>
#include<algorithm>
#include<tuple>

using namespace std;


auto print_vec(const vector<int> & vec, FILE * f = stdout) {
	for(auto i = 0; i < vec.size(); ++i) {
		fprintf(f, "%d", vec[i]);
		if(i + 1 == vec.size()) fprintf(f, "\n");
		else fprintf(f, " ");
	}
}

auto load_item(const char * filename) {
	auto f = fopen(filename, "r");
	assert(f);

	int item_count, capacity;
	fscanf(f, "%d %d", &item_count, &capacity);
	vector<Item> items;
	for(auto i = 0; i < item_count; ++i) {
		int value;
		int weight;
		fscanf(f, "%d %d", &value, &weight);

		items.push_back(Item{ i, value, weight });
	}

	fclose(f);

	return make_tuple(items, capacity);
}

struct Item {
	int index;
	int value;
	int weight;

	bool operator < (Item rhs) const {
		return static_cast<double>(value) / weight > static_cast<double>(rhs.value) / rhs.weight;
	}
};


auto get_expectation(const vector<Item> & items, int capacity, int start) {
	auto expectation = 0.0;
	for(auto i = start; i < items.size(); ++i) {
		auto item = items[i];
		if(capacity >= item.weight) {
			expectation += item.value;
			capacity -= item.weight;
		}
		// если текущей емкости недостаточно для переноса всего предмета,то положу ее часть в рюкзак
		// и добавлю ту же часть его значения к expectation
		else {
			expectation += static_cast<double>(item.value) * capacity / item.weight;
			break;
		}
	}

	return expectation;
}

// находит макс значение value и решает брать/не брать элемент
auto search(const vector<Item> & items,  int capacity) {
	auto max_value = 0.0;
	auto max_taken = vector<int>(items.size(), 0);

	auto start_value = 0.0;
	auto start_capacity = capacity;
	auto start_expectation = get_expectation(items, capacity, 0);
	auto start_taken = vector<int>(items.size(), 0);
	auto start_pos = 0;

	using StackElem = tuple<double, int, double, vector<int>, int>;
	vector<StackElem> stack;
	stack.push_back(make_tuple(start_value, start_capacity, start_expectation, start_taken, start_pos));
	while(!stack.empty()) {
		auto [cur_value, cur_capacity, cur_expectation, cur_taken, cur_pos] = stack.back();
		stack.pop_back();

		if(cur_capacity < 0) continue;
		
		if(cur_expectation <= max_value) continue;

		// если max value меньше чем current value, обновляю max value и кладем предмет
		if(max_value < cur_value) {
			max_value = cur_value;
			max_taken = cur_taken;
		}

		if(cur_pos >= items.size()) continue;

		auto cur_item = items[cur_pos];
    
        auto notake_value = cur_value;
        auto notake_capacity = cur_capacity;
        auto notake_expectation = notake_value + get_expectation(items, notake_capacity, cur_pos + 1);
        auto notake_taken = cur_taken;
        
        stack.push_back(make_tuple(notake_value, notake_capacity, notake_expectation, notake_taken, cur_pos + 1));
    

        auto take_value = cur_value + cur_item.value;
        auto take_capacity = cur_capacity - cur_item.weight;
        auto take_expectation = take_value + get_expectation(items, take_capacity, cur_pos + 1);
        auto take_taken = cur_taken;
        take_taken[cur_item.index] = 1;
        
        stack.push_back(make_tuple(take_value, take_capacity, take_expectation, take_taken, cur_pos + 1));
	}
	return make_tuple(static_cast<int>(max_value), max_taken);
}




auto save_item(const char * filename, int value, const vector<int> & taken) {
	// запись результата в сpp_output.txt, чтобы solver.py мог прочитать результат
	auto f = fopen(filename, "w");
	assert(f);

	fprintf(f, "%d 1\n", value);
	print_vec(taken, f);
	fclose(f);
}

int main(int argc, char * argv[]) {
    if(argc < 2) {
        printf("Usage: ./main <data-file>\n");
        printf("Example: ./main data/ks_30_0\n");
        exit(-1);
    }

	//чтение из python_input.txt
	
	auto [items, capacity] = load_item(argv[1]);

	// // сортируем элементы так, чтобы они находились в порядке убывания плотности значений
	sort(items.begin(), items.end());

	auto [value, taken] = search(items, capacity);

	printf("%d 1\n", value);
	print_vec(taken);

	save_item("cpp_output.txt", value, taken);

	return 0;
}
