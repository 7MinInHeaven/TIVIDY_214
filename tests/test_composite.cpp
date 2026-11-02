#include <gtest/gtest.h>
#include "Composite.h"

TEST(CompositeTest, AddGetRemoveChildrenCorrectly) {
    WorkGroup group("Test Group");
    Stage* stage = new Stage("Test Stage One");
    SubProcess* process = new SubProcess("Test Process One");
    WorkUnit* unit = new WorkUnit("Test Unit One");

    EXPECT_TRUE(group.add(stage));    
    EXPECT_FALSE(group.add(stage));
    EXPECT_TRUE(group.add(new Stage("Test Stage Two")));
    EXPECT_FALSE(group.add(new Stage("Test Stage Two")));
    group.getChildren();
    group.remove("Test Stage Two");
    
    EXPECT_TRUE(stage->add(process));    
    EXPECT_TRUE(stage->add(new SubProcess("Test Process Two")));
    EXPECT_FALSE(stage->add(new SubProcess("Test Process Two")));
    stage->getChildren();
    stage->remove("Test Process Two");

    WorkComponent* testStage = new Stage("Test Stage");

    EXPECT_TRUE(process->add(unit)); 
    EXPECT_FALSE(process->add(testStage));   
    EXPECT_TRUE(process->add(new WorkUnit("Test Unit Two")));
    EXPECT_FALSE(process->add(new WorkUnit("Test Unit Two")));
    process->getChildren();
    process->remove("Test Unit Two");
    process->remove(unit);
}

TEST(CompositeTest, CloningCorrect) {
    WorkGroup group("Test Group");
    Stage* stage = new Stage("Test Stage One");
    SubProcess* process = new SubProcess("Test Process One");
    WorkUnit* unit = new WorkUnit("Test Unit One");

    process->add(unit);
    stage->add(process);
    group.add(stage);

    WorkGroup* GroupClone = group.clone();
        EXPECT_EQ(GroupClone->getName(), group.getName());
        EXPECT_EQ(GroupClone->getState()->getStatus(), group.getState()->getStatus());
        EXPECT_NE(GroupClone->getState(), group.getState());

        EXPECT_EQ(GroupClone->getItem()->getPayloadData(), group.getItem()->getPayloadData());
        EXPECT_NE(GroupClone->getItem(), group.getItem());

        for(size_t i =0; i < group.getChildren().size(); i++){
            EXPECT_EQ(GroupClone->getChildren()[i]->getName(), group.getChildren()[i]->getName());
            EXPECT_NE(GroupClone->getChildren()[i], group.getChildren()[i]);
        }

    delete GroupClone;

    Stage* StageClone = stage->clone();
        EXPECT_EQ(StageClone->getName(), stage->getName());
        EXPECT_EQ(StageClone->getState()->getStatus(), stage->getState()->getStatus());
        EXPECT_NE(StageClone->getState(), stage->getState());

        EXPECT_EQ(StageClone->getItem()->getPayloadData(), stage->getItem()->getPayloadData());
        EXPECT_NE(StageClone->getItem(), stage->getItem());


        for(size_t i =0; i < stage->getChildren().size(); i++){
            EXPECT_EQ(StageClone->getChildren()[i]->getName(), stage->getChildren()[i]->getName());
            EXPECT_NE(StageClone->getChildren()[i], stage->getChildren()[i]);
        }

    delete StageClone;
    SubProcess* ProcessClone = process->clone();
        EXPECT_EQ(ProcessClone->getName(), process->getName());
        EXPECT_EQ(ProcessClone->getState()->getStatus(), process->getState()->getStatus());
        EXPECT_NE(ProcessClone->getState(), process->getState());
        
        EXPECT_EQ(ProcessClone->getItem()->getPayloadData(), process->getItem()->getPayloadData());
        EXPECT_NE(ProcessClone->getItem(), process->getItem());


        for(size_t i =0; i < process->getChildren().size(); i++){
            EXPECT_EQ(ProcessClone->getChildren()[i]->getName(), process->getChildren()[i]->getName());
            EXPECT_NE(ProcessClone->getChildren()[i], process->getChildren()[i]);
        }

    delete ProcessClone;
    WorkUnit* UnitClone = unit->clone();
        EXPECT_EQ(UnitClone->getName(), unit->getName());
        EXPECT_EQ(UnitClone->getState()->getStatus(), unit->getState()->getStatus());
        EXPECT_NE(UnitClone->getState(), unit->getState());

        EXPECT_EQ(UnitClone->getItem()->getPayloadData(), unit->getItem()->getPayloadData());
        EXPECT_NE(UnitClone->getItem(), unit->getItem());

    delete UnitClone;
}