/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/IModelObjectSelection.h"
#include "emapp/Model.h"

using namespace nanoem;
using namespace test;

const nanoem_u8_t kRightWristBoneName[] = { 0x89, 0x45, 0x8e, 0xe8, 0x8e, 0xf1, 0x0 },
                  kLeftWristBoneName[] = { 0x8d, 0xb6, 0x8e, 0xe8, 0x8e, 0xf1, 0x0 };

struct StringScope {
    StringScope(const nanoem_u8_t *name, Project *project)
        : m_project(project)
    {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        m_value = nanoemUnicodeStringFactoryCreateStringWithEncoding(
            project->unicodeStringFactory(), name, strlen((const char *) name), NANOEM_CODEC_TYPE_SJIS, &status);
        assert(status == NANOEM_STATUS_SUCCESS);
    }
    ~StringScope()
    {
        nanoemUnicodeStringFactoryDestroyString(m_project->unicodeStringFactory(), m_value);
    }

    Project *m_project = 0;
    nanoem_unicode_string_t *m_value = 0;
};

TEST_CASE("project_copy_paste_bone_parameters", "[emapp][project]")
{
    TestScope scope;
    Error error;
    ProjectPtr first = scope.createProject();
    Project *project = first->m_project;
    Model *activeModel = first->createModel();
    project->addModel(activeModel);
    project->setActiveModel(activeModel);
    activeModel->selection()->addAllBones();
    SECTION("apply only on select mode")
    {
        const Project::EditingMode modes[] = { Project::kEditingModeRotate, Project::kEditingModeMove,
            Project::kEditingModeEffect };
        for (auto it : modes) {
            project->setEditingMode(it);
            project->handleCopyAction(error);
            CHECK_FALSE(error.hasReason());
            CHECK(project->isModelClipboardEmpty());
        }
    }
    SECTION("paste")
    {
        model::Bone *bone = model::Bone::cast(activeModel->activeBone());
        bone->setLocalUserTranslation(Vector3(1, 2, 3));
        bone->setLocalUserOrientation(Quaternion(0.1f, 0.2f, 0.3f, 0.4f));
        project->setEditingMode(Project::kEditingModeSelect);
        project->handleCopyAction(error);
        CHECK_FALSE(error.hasReason());
        CHECK_FALSE(project->isModelClipboardEmpty());
        project->seek(42, true);
        CHECK(bone->localUserTranslation() == Constants::kZeroV3);
        CHECK(bone->localUserOrientation() == Constants::kZeroQ);
        project->handlePasteAction(error);
        CHECK_FALSE(error.hasReason());
        CHECK(bone->localUserTranslation() == Vector3(1, 2, 3));
        CHECK(bone->localUserOrientation() == Quaternion(0.1f, 0.2f, 0.3f, 0.4f));
        SECTION("copied data should not be cleared after paste")
        {
            project->seek(84, true);
            CHECK(bone->localUserTranslation() == Constants::kZeroV3);
            CHECK(bone->localUserOrientation() == Constants::kZeroQ);
            project->handlePasteAction(error);
            CHECK_FALSE(error.hasReason());
            CHECK(bone->localUserTranslation() == Vector3(1, 2, 3));
            CHECK(bone->localUserOrientation() == Quaternion(0.1f, 0.2f, 0.3f, 0.4f));
        }
        CHECK_FALSE(scope.hasAnyError());
    }
    SECTION("paste with reverse")
    {
        StringScope ls(kLeftWristBoneName, project), rs(kRightWristBoneName, project);
        model::Bone *leftBone = model::Bone::cast(activeModel->findBone(ls.m_value));
        model::Bone *rightBone = model::Bone::cast(activeModel->findBone(rs.m_value));
        SECTION("from left to right")
        {
            leftBone->setLocalUserTranslation(Vector3(1, 2, 3));
            leftBone->setLocalUserOrientation(Quaternion(0.1f, 0.2f, 0.3f, 0.4f));
            project->setEditingMode(Project::kEditingModeSelect);
            project->handleCopyAction(error);
            CHECK_FALSE(error.hasReason());
            CHECK_FALSE(project->isModelClipboardEmpty());
            project->seek(42, true);
            CHECK(rightBone->localUserTranslation() == Constants::kZeroV3);
            CHECK(rightBone->localUserOrientation() == Constants::kZeroQ);
            project->reversePasteAllSelectedBones(error);
            CHECK_FALSE(error.hasReason());
            CHECK(rightBone->localUserTranslation() == Vector3(-1, 2, 3));
            CHECK(rightBone->localUserOrientation() == Quaternion(0.1f, 0.2f, -0.3f, -0.4f));
            CHECK_FALSE(scope.hasAnyError());
            SECTION("copied data should not be cleared after paste")
            {
                project->seek(84, true);
                CHECK(rightBone->localUserTranslation() == Constants::kZeroV3);
                CHECK(rightBone->localUserOrientation() == Constants::kZeroQ);
                project->reversePasteAllSelectedBones(error);
                CHECK_FALSE(error.hasReason());
                CHECK(rightBone->localUserTranslation() == Vector3(-1, 2, 3));
                CHECK(rightBone->localUserOrientation() == Quaternion(0.1f, 0.2f, -0.3f, -0.4f));
            }
        }
        SECTION("from right to left")
        {
            rightBone->setLocalUserTranslation(Vector3(1, 2, 3));
            rightBone->setLocalUserOrientation(Quaternion(0.1f, 0.2f, 0.3f, 0.4f));
            project->setEditingMode(Project::kEditingModeSelect);
            project->handleCopyAction(error);
            CHECK_FALSE(error.hasReason());
            CHECK_FALSE(project->isModelClipboardEmpty());
            project->seek(42, true);
            CHECK(leftBone->localUserTranslation() == Constants::kZeroV3);
            CHECK(leftBone->localUserOrientation() == Constants::kZeroQ);
            project->reversePasteAllSelectedBones(error);
            CHECK_FALSE(error.hasReason());
            CHECK(leftBone->localUserTranslation() == Vector3(-1, 2, 3));
            CHECK(leftBone->localUserOrientation() == Quaternion(0.1f, 0.2f, -0.3f, -0.4f));
            CHECK_FALSE(scope.hasAnyError());
            SECTION("copied data should not be cleared after paste")
            {
                project->seek(84, true);
                CHECK(leftBone->localUserTranslation() == Constants::kZeroV3);
                CHECK(leftBone->localUserOrientation() == Constants::kZeroQ);
                project->reversePasteAllSelectedBones(error);
                CHECK_FALSE(error.hasReason());
                CHECK(leftBone->localUserTranslation() == Vector3(-1, 2, 3));
                CHECK(leftBone->localUserOrientation() == Quaternion(0.1f, 0.2f, -0.3f, -0.4f));
            }
        }
    }
}

TEST_CASE("project_copy_paste_bone_parameters_redo", "[emapp][project]")
{
    TestScope scope;
    Error error;
    SECTION("paste")
    {
        {
            ProjectPtr first = scope.createProject();
            Project *project = first->withRecoverable();
            Model *activeModel = first->createModel();
            project->addModel(activeModel);
            project->setActiveModel(activeModel);
            activeModel->selection()->addAllBones();
            model::Bone *firstBone = model::Bone::cast(activeModel->activeBone());
            firstBone->setLocalUserTranslation(Vector3(1, 2, 3));
            firstBone->setLocalUserOrientation(Quaternion(0.1f, 0.2f, 0.3f, 0.4f));
            project->setEditingMode(Project::kEditingModeSelect);
            project->handleCopyAction(error);
            CHECK_FALSE(error.hasReason());
            project->seek(42, true);
            project->handlePasteAction(error);
            CHECK_FALSE(error.hasReason());
        }
        {
            ProjectPtr second = scope.createProject();
            Project *secondProject = second->m_project;
            scope.recover(secondProject);
            model::Bone *secondBone = model::Bone::cast(secondProject->activeModel()->activeBone());
            CHECK(secondBone->localUserTranslation() == Vector3(1, 2, 3));
            CHECK(secondBone->localUserOrientation() == Quaternion(0.1f, 0.2f, 0.3f, 0.4f));
        }
    }
    SECTION("paste with reverse")
    {
        SECTION("from left to right")
        {
            {
                ProjectPtr first = scope.createProject();
                Project *firstProject = first->withRecoverable();
                Model *activeModel = first->createModel();
                firstProject->addModel(activeModel);
                firstProject->setActiveModel(activeModel);
                activeModel->selection()->addAllBones();
                StringScope ls(kLeftWristBoneName, firstProject);
                model::Bone *firstBone = model::Bone::cast(activeModel->findBone(ls.m_value));
                firstBone->setLocalUserTranslation(Vector3(1, 2, 3));
                firstBone->setLocalUserOrientation(Quaternion(0.1f, 0.2f, 0.3f, 0.4f));
                firstProject->setEditingMode(Project::kEditingModeSelect);
                firstProject->handleCopyAction(error);
                CHECK_FALSE(error.hasReason());
                firstProject->seek(42, true);
                firstProject->reversePasteAllSelectedBones(error);
                CHECK_FALSE(error.hasReason());
            }
            {
                ProjectPtr second = scope.createProject();
                Project *secondProject = second->m_project;
                scope.recover(secondProject);
                StringScope rs(kRightWristBoneName, secondProject);
                model::Bone *secondBone = model::Bone::cast(secondProject->activeModel()->findBone(rs.m_value));
                CHECK(secondBone->localUserTranslation() == Vector3(-1, 2, 3));
                CHECK(secondBone->localUserOrientation() == Quaternion(0.1f, 0.2f, -0.3f, -0.4f));
                CHECK_FALSE(scope.hasAnyError());
            }
        }
        SECTION("from right to left")
        {
            {
                ProjectPtr first = scope.createProject();
                Project *firstProject = first->withRecoverable();
                Model *activeModel = first->createModel();
                firstProject->addModel(activeModel);
                firstProject->setActiveModel(activeModel);
                activeModel->selection()->addAllBones();
                StringScope rs(kRightWristBoneName, firstProject);
                model::Bone *firstBone = model::Bone::cast(activeModel->findBone(rs.m_value));
                firstBone->setLocalUserTranslation(Vector3(1, 2, 3));
                firstBone->setLocalUserOrientation(Quaternion(0.1f, 0.2f, 0.3f, 0.4f));
                firstProject->setEditingMode(Project::kEditingModeSelect);
                firstProject->handleCopyAction(error);
                CHECK_FALSE(error.hasReason());
                firstProject->seek(42, true);
                firstProject->reversePasteAllSelectedBones(error);
                CHECK_FALSE(error.hasReason());
            }
            {
                ProjectPtr second = scope.createProject();
                Project *secondProject = second->m_project;
                scope.recover(secondProject);
                StringScope ls(kLeftWristBoneName, secondProject);
                model::Bone *secondBone = model::Bone::cast(secondProject->activeModel()->findBone(ls.m_value));
                CHECK(secondBone->localUserTranslation() == Vector3(-1, 2, 3));
                CHECK(secondBone->localUserOrientation() == Quaternion(0.1f, 0.2f, -0.3f, -0.4f));
                CHECK_FALSE(scope.hasAnyError());
            }
        }
    }
}
