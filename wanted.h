#pragma once
#include <string>
#include <vector>
#include <optional>

struct WantedFile {
    std::string url;
    std::string name;
};

struct WantedImage {
    std::string large;
    std::string caption;
    std::string thumb;
    std::string original;
};

// Nullable JSON strings/arrays that were absent become "" / an empty vector.
// Nullable JSON numbers use std::optional<int> so "present but null" is distinguishable from 0.
struct WantedPerson {
    std::string uid;
    std::string title;
    std::string description;
    std::string details;
    std::string status;
    std::string personClassification;
    std::string posterClassification;
    std::string path;
    std::string url;
    std::string pathId;
    std::string publication;
    std::string modified;
    std::string warningMessage;
    std::string caution;
    std::string remarks;
    std::string additionalInformation;

    std::string race;
    std::string raceRaw;
    std::string sex;
    std::string hair;
    std::string hairRaw;
    std::string eyes;
    std::string eyesRaw;
    std::string complexion;
    std::string build;
    std::string weight;
    std::string nationality;
    std::string placeOfBirth;
    std::string scarsAndMarks;
    std::string ncic;
    std::string rewardText;

    std::optional<int> ageMin;
    std::optional<int> ageMax;
    std::optional<int> heightMin;
    std::optional<int> heightMax;
    std::optional<int> weightMin;
    std::optional<int> weightMax;

    int rewardMin = 0;
    int rewardMax = 0;

    std::vector<std::string> subjects;
    std::vector<std::string> fieldOffices;
    std::vector<std::string> aliases;
    std::vector<std::string> occupations;
    std::vector<std::string> languages;
    std::vector<std::string> datesOfBirthUsed;
    std::vector<std::string> possibleStates;
    std::vector<std::string> possibleCountries;

    std::vector<WantedFile> files;
    std::vector<WantedImage> images;
};

// Parses a raw JSON document (as returned by the FBI Wanted API) and returns
// one WantedPerson per entry in its top-level "items" array.
std::vector<WantedPerson> parseWantedPeople(const std::string& jsonText);
