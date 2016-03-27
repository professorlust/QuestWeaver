//
// Created by michael on 12.10.15.
//

#include <string>
#include <memory>
#include <Template/TemplateEngine.h>
#include <Template/Space/SpaceQuestTemplateFactory.h>
#include <World/Space/SpaceWorldModel.h>
#include <QuestModel/Space/ExploreRegionQuest.h>
#include "../catch.hpp"
#include "../Mock/TestQuestTemplate.h"
#include "../Mock/TestQuestTemplateFactory.h"

using namespace weave;
using namespace std;

TEST_CASE("Format check", "[template]") {
    shared_ptr<RandomStream> rs = make_shared<RandomStream>(42);
    TemplateEngine textEngine(rs, Directories(), FormatterType::TEXT);
    TemplateEngine htmlEngine(rs, Directories(), FormatterType::HTML);
    REQUIRE(textEngine.GetFormat() == FormatterType::TEXT);
    REQUIRE(htmlEngine.GetFormat() == FormatterType::HTML);
}

TEST_CASE("Template factory", "[template]") {
    shared_ptr<RandomStream> rs = make_shared<RandomStream>(42);
    TemplateEngine engine(rs, Directories(), FormatterType::TEXT);
    REQUIRE_THROWS_AS(engine.GetTemplateForNewQuest(), ContractFailedException);

    SpaceQuestTemplateFactory *factory = new SpaceQuestTemplateFactory();
    engine.RegisterTemplateFactory(unique_ptr<SpaceQuestTemplateFactory>(factory));
    REQUIRE(factory->GetTemplateKeys().size() >= 1);

    SECTION("Checking the keys") {
        vector<string> keys = {"ExploreRegionQuest"};
        for (string key : keys) {
            bool hasKey = false;
            for (string templateKey : factory->GetTemplateKeys()) {
                if (key == templateKey) {
                    hasKey = true;
                    break;
                }
            }
            REQUIRE(hasKey);
        }
    }

    SECTION("Retrieve unknown key") {
        REQUIRE_THROWS_AS(factory->CreateTemplate("SecretTemplate"), ContractFailedException);
    }

    SECTION("Retrieving template for every key") {
        for (string templateKey : factory->GetTemplateKeys()) {
            auto temp = factory->CreateTemplate(templateKey);
            REQUIRE(temp.get() != nullptr);
        }
    }
}

TEST_CASE("Templates", "[template]") {
    uint64_t testSize = 100;
    shared_ptr<RandomStream> rs = make_shared<RandomStream>(42);
    TemplateEngine engine(rs, Directories(), FormatterType::TEXT);
    SpaceQuestTemplateFactory *factory = new SpaceQuestTemplateFactory();
    engine.RegisterTemplateFactory(unique_ptr<SpaceQuestTemplateFactory>(factory));

    SECTION("Checking properties size") {
        for (uint64_t i = 0; i < testSize; i++) {
            rs->Seed(i);
            for (string templateKey : factory->GetTemplateKeys()) {
                auto tp = factory->CreateTemplate(templateKey);
                INFO("Template Key: " + templateKey + ", Seed: " + to_string(i));
                REQUIRE(tp->GetProperties().size() > 0);
            }
        }
    }

    SECTION("Checking mandatory property exists") {
        for (uint64_t i = 0; i < testSize; i++) {
            rs->Seed(i);
            for (string templateKey : factory->GetTemplateKeys()) {
                auto tp = factory->CreateTemplate(templateKey);
                bool hasMandatoryProperty = false;
                for (auto property : tp->GetProperties()) {
                    if (property.IsMandatory()) {
                        hasMandatoryProperty = true;
                        break;
                    }
                }
                INFO("Template Key: " + templateKey + ", Seed: " + to_string(i));
                REQUIRE(hasMandatoryProperty);
            }
        }
    }

    SECTION("Checking property names") {
        for (uint64_t i = 0; i < testSize; i++) {
            rs->Seed(i);
            for (string templateKey : factory->GetTemplateKeys()) {
                auto tp = factory->CreateTemplate(templateKey);
                for (auto property : tp->GetProperties()) {
                    INFO("Template Key: " + templateKey + ", Property: " + property.GetName() + ", Seed: " +
                         to_string(i));
                    REQUIRE(!property.GetName().empty());
                }
            }
        }
    }

    SECTION("Checking property candidates") {
        WorldModel *worldModel = new SpaceWorldModel(rs);
        for (uint64_t i = 0; i < testSize; i++) {
            rs->Seed(i);
            for (string templateKey : factory->GetTemplateKeys()) {
                auto tp = factory->CreateTemplate(templateKey);
                for (auto property : tp->GetProperties()) {
                    auto candidates = tp->GetPropertyCandidates(property, *worldModel);
                    INFO("Template Key: " + templateKey + ", Property: " + property.GetName() + ", Seed: " +
                         to_string(i));
                    REQUIRE(candidates.size() > 0);
                }
            }
        }
    }

    SECTION("Checking model actions on empty model ") {
        WorldModel *worldModel = new SpaceWorldModel(rs);
        for (uint64_t i = 0; i < testSize; i++) {
            rs->Seed(i);
            for (string templateKey : factory->GetTemplateKeys()) {
                auto tp = factory->CreateTemplate(templateKey);
                for (auto property : tp->GetProperties()) {
                    auto candidates = tp->GetPropertyCandidates(property, *worldModel);
                    for (auto candidate : candidates) {
                        INFO("Template Key: " + templateKey + ", Property: " + property.GetName() + ", Seed: " +
                             to_string(i));
                        for (auto action : candidate.GetActions()) {
                            REQUIRE((action.GetActionType() == WorldActionType::CREATE ||
                                     action.GetActionType() == WorldActionType::UPDATE));
                        }
                        REQUIRE(candidate.GetValue().get() != nullptr);
                    }
                }
            }
        }
    }

    SECTION("Checking quest creation all candidates") {
        WorldModel *worldModel = new SpaceWorldModel(rs);
        for (uint64_t i = 0; i < testSize; i++) {
            rs->Seed(i);
            for (string templateKey : factory->GetTemplateKeys()) {
                auto tp = factory->CreateTemplate(templateKey);
                vector<QuestPropertyValue> questValues;
                for (auto property : tp->GetProperties()) {
                    auto candidates = tp->GetPropertyCandidates(property, *worldModel);
                    vector<WorldModelAction> actions;
                    for (auto candidate : candidates) {
                        questValues.push_back(QuestPropertyValue(property, candidate.GetValue()));
                        for (auto action : candidate.GetActions()) {
                            actions.push_back(action);
                        }
                    }
                    worldModel->Execute(actions);
                }

                INFO("Template Key: " + templateKey + ", Seed: " + to_string(i));
                try {
                    auto quest = tp->ToQuest(questValues);
                    REQUIRE(!quest->GetDescription().empty());
                    REQUIRE(!quest->GetTitle().empty());
                } catch (ContractFailedException ex) {
                    FAIL(ex.what());
                }
            }
        }
    }

    SECTION("Checking quest creation only mandatory candidates") {
        WorldModel *worldModel = new SpaceWorldModel(rs);
        for (uint64_t i = 0; i < testSize; i++) {
            rs->Seed(i);
            for (string templateKey : factory->GetTemplateKeys()) {
                auto tp = factory->CreateTemplate(templateKey);
                vector<QuestPropertyValue> questValues;
                for (auto property : tp->GetProperties()) {
                    if (!property.IsMandatory()) {
                        continue;
                    }
                    auto candidates = tp->GetPropertyCandidates(property, *worldModel);
                    vector<WorldModelAction> actions;
                    for (auto candidate : candidates) {
                        questValues.push_back(QuestPropertyValue(property, candidate.GetValue()));
                        for (auto action : candidate.GetActions()) {
                            actions.push_back(action);
                        }
                    }
                    worldModel->Execute(actions);
                }

                INFO("Template Key: " + templateKey + ", Seed: " + to_string(i));
                try {
                    auto quest = tp->ToQuest(questValues);
                    REQUIRE(!quest->GetDescription().empty());
                    REQUIRE(!quest->GetTitle().empty());
                } catch (ContractFailedException ex) {
                    FAIL(ex.what());
                }
            }
        }
    }

    SECTION("Quest descriptions") {
        SpaceWorldModel *worldModel = new SpaceWorldModel(rs);
        auto entity = worldModel->CreateLocation().GetEntity();

        vector<TemplateQuestProperty> properties;
        TemplateQuestProperty mandy(true, "mandy");
        TemplateQuestProperty sandy(true, "sandy");
        TemplateQuestProperty opti(false, "opti");
        TemplateQuestProperty peti(false, "peti");
        properties.push_back(mandy);
        properties.push_back(sandy);
        properties.push_back(opti);
        properties.push_back(peti);

        vector<TemplateQuestDescription> descriptions;
        vector<string> conditions;
        vector<QuestPropertyValue> propertyValues;


        SECTION("Normal quest description substitution") {
            conditions.push_back("mandy");
            conditions.push_back("sandy");
            descriptions.push_back(TemplateQuestDescription(conditions, "Why %mandy loves %sandy..."));
            TestQuestTemplate testTemplate(properties, descriptions);
            propertyValues.push_back(QuestPropertyValue(mandy, entity));
            propertyValues.push_back(QuestPropertyValue(sandy, entity));
            auto quest = testTemplate.ToQuest(propertyValues, "");
            string description = quest->GetDescription();
            REQUIRE_FALSE(description.empty());
            REQUIRE(description.find("mandy") == string::npos);
            REQUIRE(description.find("sandy") == string::npos);
        }

        SECTION("Impossible quest description substitution") {
            conditions.push_back("mandy");
            conditions.push_back("opti");
            descriptions.push_back(TemplateQuestDescription(conditions, "Why %mandy loves %opti..."));
            TestQuestTemplate testTemplate(properties, descriptions);
            propertyValues.push_back(QuestPropertyValue(mandy, entity));
            propertyValues.push_back(QuestPropertyValue(sandy, entity));
            REQUIRE_THROWS_AS(testTemplate.ToQuest(propertyValues, ""), ContractFailedException);
        }
    }

    SECTION("Read faulty template file") {
        Directories dirs;
        dirs.templateDirectory = "Test/Resources/";
        dirs.modDirectory = "../Test/Resources/";
        TemplateEngine tempEngine(rs, dirs, FormatterType::TEXT);
        TestQuestTemplateFactory *factory = new TestQuestTemplateFactory("missingAttribute.qt");
        tempEngine.RegisterTemplateFactory(unique_ptr<TestQuestTemplateFactory>(factory));
        REQUIRE_THROWS_AS(factory->GetTemplateKeys(), ContractFailedException);
    }
}

TEST_CASE("Directory Change", "[template]") {
    shared_ptr<RandomStream> rs = make_shared<RandomStream>(42);
    TemplateEngine engine(rs, Directories(), FormatterType::TEXT);
    unique_ptr<SpaceQuestTemplateFactory> factory(new SpaceQuestTemplateFactory());

    SECTION("Unknown dir pre reg") {
        Directories unknown;
        unknown.templateDirectory = "Test/Unknown/";
        unknown.modDirectory = "../Test/Unknown/";
        engine.ChangeDirectories(unknown);
        engine.RegisterTemplateFactory(move(factory));
        REQUIRE_THROWS_AS(engine.GetTemplateForNewQuest(), ContractFailedException);
    }

    SECTION("Unknown dir post reg") {
        engine.RegisterTemplateFactory(move(factory));
        Directories unknown;
        unknown.templateDirectory = "Test/Unknown/";
        unknown.modDirectory = "../Test/Unknown/";
        engine.ChangeDirectories(unknown);
        REQUIRE_THROWS_AS(engine.GetTemplateForNewQuest(), ContractFailedException);
    }

    SECTION("Pre register") {
        Directories unknown;
        unknown.templateDirectory = "Test/Resources/";
        unknown.modDirectory = "../Test/Resources/";
        engine.ChangeDirectories(unknown);
        engine.RegisterTemplateFactory(move(factory));
        REQUIRE(engine.GetTemplateForNewQuest() != nullptr);
    }
}

TEST_CASE("Explore quest", "[quest]") {
    shared_ptr<RandomStream> rs = make_shared<RandomStream>(42);
    TemplateEngine engine(rs, Directories(), FormatterType::TEXT);
    SpaceQuestTemplateFactory *factory = new SpaceQuestTemplateFactory();
    engine.RegisterTemplateFactory(unique_ptr<SpaceQuestTemplateFactory>(factory));
    SpaceWorldModel world(rs);

    // Create a new explore region quest manually
    auto questTemplate = factory->CreateTemplate("ExploreRegionQuest");
    vector<QuestPropertyValue> questPropertyValues;
    TemplateQuestProperty property(true, "solarSystem");
    auto addSystemActions = world.CreateSolarSystem();
    world.Execute(addSystemActions);
    auto solarSystem = addSystemActions[addSystemActions.size() - 1].GetEntity();
    questPropertyValues.push_back(QuestPropertyValue(property, solarSystem));
    auto quest = questTemplate->ToQuest(questPropertyValues);

    SECTION("Exploration Ticks nothing") {
        QuestTickResult tickResult = quest->Tick(1, world);
        REQUIRE(tickResult.GetQuestChange().GetActionType() == QuestActionType::KEEP);
        REQUIRE(tickResult.GetWorldChanges().size() == 0);

        tickResult = quest->Tick(2, world);
        REQUIRE(tickResult.GetQuestChange().GetActionType() == QuestActionType::KEEP);
        REQUIRE(tickResult.GetWorldChanges().size() == 0);
    }

    SECTION("Exploration Tick partial") {
        MetaData metaData;
        metaData.SetValue(ExploreRegionQuest::metaDataMarker, 50);
        WorldModelAction metaDataAction(WorldActionType::KEEP, solarSystem, metaData);
        world.Execute({metaDataAction});

        QuestTickResult tickResult = quest->Tick(1, world);
        REQUIRE(tickResult.GetQuestChange().GetActionType() == QuestActionType::KEEP);
        REQUIRE(tickResult.GetWorldChanges().size() == 0);
    }

    SECTION("Exploration Tick success") {
        MetaData metaData;
        metaData.SetValue(ExploreRegionQuest::metaDataMarker, 100);
        WorldModelAction metaDataAction(WorldActionType::UPDATE, solarSystem, metaData);
        world.Execute({metaDataAction});

        QuestTickResult tickResult = quest->Tick(1, world);
        REQUIRE(tickResult.GetQuestChange().GetActionType() == QuestActionType::SUCCEED);
        REQUIRE(tickResult.GetWorldChanges().size() == 0);
    }
}
