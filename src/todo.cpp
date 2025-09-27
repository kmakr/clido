#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Task {
    std::string description;
    bool completed = false;
};

fs::path storage_path() {
    const char *home = std::getenv("HOME");
    fs::path base = home ? fs::path(home) : fs::temp_directory_path();
    fs::path dir = base / ".clido";
    fs::create_directories(dir);
    return dir / "tasks.db";
}

std::vector<Task> load_tasks() {
    std::vector<Task> tasks;
    std::ifstream file(storage_path());
    if (!file.is_open()) {
        return tasks;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::istringstream iss(line);
        std::string status;
        if (!std::getline(iss, status, '\t')) {
            continue;
        }
        std::string description;
        if (!std::getline(iss, description)) {
            continue;
        }
        tasks.push_back(Task{description, status == "1"});
    }

    return tasks;
}

void save_tasks(const std::vector<Task> &tasks) {
    std::ofstream file(storage_path(), std::ios::trunc);
    for (const auto &task : tasks) {
        file << (task.completed ? "1" : "0") << '\t' << task.description << '\n';
    }
}

void print_usage(const std::string &program) {
    std::cout << "Usage: " << program << " <command> [options]\n\n"
              << "Commands:\n"
              << "  add <description>   Add a new task\n"
              << "  list                Show all tasks\n"
              << "  done <id>           Mark a task as completed\n"
              << "  undone <id>         Mark a task as not completed\n"
              << "  remove <id>         Remove a task\n"
              << "  clear               Remove all tasks\n"
              << "  help                Show this message\n";
}

void list_tasks(const std::vector<Task> &tasks) {
    if (tasks.empty()) {
        std::cout << "No tasks yet. Add one with 'add'.\n";
        return;
    }

    for (std::size_t i = 0; i < tasks.size(); ++i) {
        const auto &task = tasks[i];
        std::cout << (i + 1) << ". " << (task.completed ? "[x] " : "[ ] ") << task.description << '\n';
    }
}

bool parse_index(const std::string &value, std::size_t max_index, std::size_t &result) {
    try {
        std::size_t pos;
        unsigned long parsed = std::stoul(value, &pos);
        if (pos != value.size() || parsed == 0 || parsed > max_index) {
            return false;
        }
        result = static_cast<std::size_t>(parsed - 1);
        return true;
    } catch (...) {
        return false;
    }
}

int main(int argc, char *argv[]) {
    std::string program = argc > 0 ? argv[0] : "todo";

    if (argc < 2) {
        print_usage(program);
        return 1;
    }

    std::string command = argv[1];
    std::vector<Task> tasks = load_tasks();

    if (command == "add") {
        if (argc < 3) {
            std::cerr << "Error: description is required.\n";
            return 1;
        }
        std::ostringstream description;
        for (int i = 2; i < argc; ++i) {
            if (i > 2) {
                description << ' ';
            }
            description << argv[i];
        }
        tasks.push_back(Task{description.str(), false});
        save_tasks(tasks);
        std::cout << "Added task: " << description.str() << "\n";
        return 0;
    }

    if (command == "list") {
        list_tasks(tasks);
        return 0;
    }

    if (command == "done" || command == "undone" || command == "remove") {
        if (argc < 3) {
            std::cerr << "Error: id is required.\n";
            return 1;
        }
        std::size_t index;
        if (!parse_index(argv[2], tasks.size(), index)) {
            std::cerr << "Error: invalid id.\n";
            return 1;
        }

        if (command == "remove") {
            std::cout << "Removed task: " << tasks[index].description << "\n";
            tasks.erase(tasks.begin() + static_cast<long>(index));
            save_tasks(tasks);
            return 0;
        }

        tasks[index].completed = (command == "done");
        save_tasks(tasks);
        std::cout << "Updated task: " << tasks[index].description << "\n";
        return 0;
    }

    if (command == "clear") {
        tasks.clear();
        save_tasks(tasks);
        std::cout << "Cleared all tasks.\n";
        return 0;
    }

    if (command == "help") {
        print_usage(program);
        return 0;
    }

    std::cerr << "Unknown command: " << command << "\n";
    print_usage(program);
    return 1;
}

