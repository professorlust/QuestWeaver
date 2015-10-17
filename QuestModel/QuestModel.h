//
// Created by michael on 10.08.15.
//

#pragma once

#include <list>
#include <vector>
#include "Quest.h"

namespace weave {
    class QuestModel {
    public:
        std::list<std::shared_ptr<Quest>> getActiveQuests() const;

        std::shared_ptr<Quest> registerQuest(const Quest &newQuest);

    private:
        std::vector<std::shared_ptr<Quest>> quests;
    };
}
