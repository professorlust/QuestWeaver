//
// Created by michael on 15.08.15.
//

#include <Template/Space/SpaceQuestTemplateFactory.h>
#include <Template/Space/ExploreRegionTemplate.h>

using namespace std;
using namespace Json;
using namespace weave;

std::shared_ptr<Template> SpaceQuestTemplateFactory::createFromJsonValues(const Json::Value &root) const {
    string title = extractTitle(root);
    vector<TemplateQuestProperty> properties = extractProperties(root);
    vector<TemplateQuestDescription> descriptions = extractDescriptions(root);

    const string &templateKey = root["key"].asString();
    if (templateKey == "ExploreRegionQuest") {
        auto exploreTemplate = make_shared<ExploreRegionTemplate>(title, properties, descriptions);

        return exploreTemplate;
    } else {
        throw ContractFailedException("Unknown Space template key " + templateKey + "\n");
    }
}

const char *SpaceQuestTemplateFactory::getTemplateFile() const {
    return "Space/ExploreRegionTemplate.qt";
}


