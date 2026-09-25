#include "client.h"
#include "wanted.h"

#include <cctype>
#include <exception>
#include <iostream>
#include <Windows.h>

std::string toLower(std::string field_office) {
    for (size_t i = 0; i < field_office.length(); i++) {
        field_office[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(field_office[i])));
    }

    return field_office;
}

int getConsoleWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns = 10;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }

    return columns;

}

void printDiv() {
    int width = getConsoleWidth();
    for (int i = 0; i < width; i++) {
        std::cout << "-";
    }
    std::cout << "\n";
}

namespace {

    void printPerson(const WantedPerson& person) {
        std::cout << "Name:   " << person.title << '\n';

        if (!person.subjects.empty()) {
            std::cout << "Crime:  ";
            for (size_t i = 0; i < person.subjects.size(); ++i) {
                std::cout << person.subjects[i];
                if (i + 1 < person.subjects.size()) std::cout << ", ";
            }
            std::cout << '\n';
        }
        else if (!person.description.empty()) {
            std::cout << "Crime:  " << person.description << '\n';
        }

        if (person.rewardMax > 0) {
            std::cout << "Reward: $" << person.rewardMin << " - $" << person.rewardMax << '\n';
        }
        if (!person.rewardText.empty()) {
            std::cout << "Reward Details: " << person.rewardText << '\n';
        }

        printDiv();
    }

}

int main() {
    std::string field_office;

    for (;;) {

        std::cout << "Enter Field Office ('q' to quit): ";
        std::cin >> field_office;

        if (toLower(field_office) == "q") {
            break;
        }

        std::cout << std::endl;

        try {
            ApiClient api;
            const auto response = api.request(toLower(field_office));

            std::cout << "HTTP status: " << response.statusCode << '\n';

            std::vector<WantedPerson> people = parseWantedPeople(response.body);
            std::cout << "Found " << people.size() << " result(s)\n\n";

            for (const auto& person : people) {
                printPerson(person);
            }
        }
        catch (const std::exception& error) {
            std::cerr << "Request failed: " << error.what() << '\n';
        }
    }

    return 0;
}
