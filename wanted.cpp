#include "wanted.h"
#include "json.h"

std::string getString(const JsonValue& obj, const std::string& key) {
    const JsonValue* v = obj.find(key);
    if (v && v->type == JsonType::String) return v->stringValue;
    return "";
}

int getInt(const JsonValue& obj, const std::string& key, int defaultValue = 0) {
    const JsonValue* v = obj.find(key);
    if (v && v->type == JsonType::Number) return static_cast<int>(v->numberValue);
    return defaultValue;
}

std::optional<int> getOptionalInt(const JsonValue& obj, const std::string& key) {
    const JsonValue* v = obj.find(key);
    if (v && v->type == JsonType::Number) return static_cast<int>(v->numberValue);
    return std::nullopt;
}

std::vector<std::string> getStringArray(const JsonValue& obj, const std::string& key) {
    std::vector<std::string> result;
    const JsonValue* v = obj.find(key);
    if (v && v->type == JsonType::Array) {
        result.reserve(v->arrayValue.size());
        for (const auto& item : v->arrayValue) {
            if (item.type == JsonType::String) result.push_back(item.stringValue);
        }
    }
    return result;
}

WantedFile parseFile(const JsonValue& obj) {
    WantedFile file;
    file.url = getString(obj, "url");
    file.name = getString(obj, "name");
    return file;
}

WantedImage parseImage(const JsonValue& obj) {
    WantedImage image;
    image.large = getString(obj, "large");
    image.caption = getString(obj, "caption");
    image.thumb = getString(obj, "thumb");
    image.original = getString(obj, "original");
    return image;
}

WantedPerson parsePerson(const JsonValue& obj) {
    WantedPerson person;

    person.uid = getString(obj, "uid");
    person.title = getString(obj, "title");
    person.description = getString(obj, "description");
    person.details = getString(obj, "details");
    person.status = getString(obj, "status");
    person.personClassification = getString(obj, "person_classification");
    person.posterClassification = getString(obj, "poster_classification");
    person.path = getString(obj, "path");
    person.url = getString(obj, "url");
    person.pathId = getString(obj, "pathId");
    person.publication = getString(obj, "publication");
    person.modified = getString(obj, "modified");
    person.warningMessage = getString(obj, "warning_message");
    person.caution = getString(obj, "caution");
    person.remarks = getString(obj, "remarks");
    person.additionalInformation = getString(obj, "additional_information");

    person.race = getString(obj, "race");
    person.raceRaw = getString(obj, "race_raw");
    person.sex = getString(obj, "sex");
    person.hair = getString(obj, "hair");
    person.hairRaw = getString(obj, "hair_raw");
    person.eyes = getString(obj, "eyes");
    person.eyesRaw = getString(obj, "eyes_raw");
    person.complexion = getString(obj, "complexion");
    person.build = getString(obj, "build");
    person.weight = getString(obj, "weight");
    person.nationality = getString(obj, "nationality");
    person.placeOfBirth = getString(obj, "place_of_birth");
    person.scarsAndMarks = getString(obj, "scars_and_marks");
    person.ncic = getString(obj, "ncic");
    person.rewardText = getString(obj, "reward_text");

    person.ageMin = getOptionalInt(obj, "age_min");
    person.ageMax = getOptionalInt(obj, "age_max");
    person.heightMin = getOptionalInt(obj, "height_min");
    person.heightMax = getOptionalInt(obj, "height_max");
    person.weightMin = getOptionalInt(obj, "weight_min");
    person.weightMax = getOptionalInt(obj, "weight_max");

    person.rewardMin = getInt(obj, "reward_min");
    person.rewardMax = getInt(obj, "reward_max");

    person.subjects = getStringArray(obj, "subjects");
    person.fieldOffices = getStringArray(obj, "field_offices");
    person.aliases = getStringArray(obj, "aliases");
    person.occupations = getStringArray(obj, "occupations");
    person.languages = getStringArray(obj, "languages");
    person.datesOfBirthUsed = getStringArray(obj, "dates_of_birth_used");
    person.possibleStates = getStringArray(obj, "possible_states");
    person.possibleCountries = getStringArray(obj, "possible_countries");

    if (const JsonValue* filesArr = obj.find("files")) {
        if (filesArr->type == JsonType::Array) {
            person.files.reserve(filesArr->arrayValue.size());
            for (const auto& f : filesArr->arrayValue) person.files.push_back(parseFile(f));
        }
    }

    if (const JsonValue* imagesArr = obj.find("images")) {
        if (imagesArr->type == JsonType::Array) {
            person.images.reserve(imagesArr->arrayValue.size());
            for (const auto& img : imagesArr->arrayValue) person.images.push_back(parseImage(img));
        }
    }

    return person;
}

std::vector<WantedPerson> parseWantedPeople(const std::string& jsonText) {
    std::vector<WantedPerson> people;

    JsonValue root = parseJson(jsonText);

    if (const JsonValue* itemsArr = root.find("items")) {
        if (itemsArr->type == JsonType::Array) {
            people.reserve(itemsArr->arrayValue.size());
            for (const auto& itemObj : itemsArr->arrayValue) {
                people.push_back(parsePerson(itemObj));
            }
        }
    }

    return people;
}
