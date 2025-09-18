/**
 * @file UndoSystem.h
 * @brief Implements an undo/redo system for tracking changes to entity components.
 *
 * This system allows changes to entity components to be undone and redone, supporting both
 * value modifications and component removals. It maintains undo and redo stacks to manage
 * change history efficiently.
 *
 * Features:
 * - Tracks changes to individual component variables.
 * - Supports undo/redo operations with a fixed history size.
 * - Allows component removal and restoration.
 * - Provides a debug function to print stack details.
 *
 * @Authors: Edwin Leow
 * Copyright 2024, Digipen Institute of Technology
 */
#pragma once
#ifndef _UNDOSYSTEM_H_
#define _UNDOSYSTEM_H_

#include "pch.h"
#include "ComponentList.h"
#include <functional>
#include <Coordinator.h>
#include <imgui.h>

extern Framework::Coordinator ecsInterface;

namespace Framework
{
    /**
     * @brief Base class for undo actions.
     *
     * Provides an interface for performing and reverting changes.
     */
    class IUndoAction
    {
    public:
        virtual ~IUndoAction() = default;
        virtual void Undo() = 0;
        virtual void Redo() = 0;
        virtual void Print() const = 0;     // Virtual Print Function
    };

    /**
     * @brief Represents an undoable action on a component variable.
     *
     * Stores the previous and new values of a variable, allowing changes to be undone and redone.
     *
     * @tparam T Type of the variable being modified.
     */
    template <typename T>
    class UndoAction : public IUndoAction
    {
    public:
        UndoAction(Entity entity, const std::string& componentName, const std::string& varName, T& var, T prevValue, T newValue)
            : mEntity(entity), mComponentName(componentName), mVarName(varName), mVar(var), mPrevValue(prevValue), mNewValue(newValue) {}

        void Undo() override
        {
            mVar = mPrevValue; // Only change the specific variable
        }

        void Redo() override
        {
            mVar = mNewValue; // Only change the specific variable
        }

        void Print() const override
        {
            std::cout << "Undo Action: Entity[" << mEntity << "], Component[" << mComponentName
                << "], Variable[" << mVarName << "]\n"
                << "  Previous Value: " << ValueToString(mPrevValue) << "\n"
                << "  New Value: " << ValueToString(mNewValue) << std::endl;
        }

    private:
        Entity mEntity;                 // Entity the action applies to
        std::string mComponentName;     // Name of the Component
        std::string mVarName;           // Name of the variable
        T& mVar;                        // Reference to the specific variable
        T mPrevValue;                   // Prev value before the change
        T mNewValue;                    // New value after the change

        // Convert value to string
        template <typename U>
        std::string ValueToString(U value) const
        {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }

        // Specialization for glm::vec2
        std::string ValueToString(glm::vec2 value) const
        {
            std::ostringstream oss;
            oss << "(" << value.x << ", " << value.y << ")";
            return oss.str();
        }

        // Specialization for glm::vec3
        std::string ValueToString(glm::vec3 value) const
        {
            std::ostringstream oss;
            oss << "(" << value.x << ", " << value.y << ", " << value.z << ")";
            return oss.str();
        }
    };

    /**
     * @brief Represents an undoable action for removing a component.
     *
     * Stores a copy of the removed component and allows it to be restored.
     *
     * @tparam T Type of the component being removed.
     */
    template <typename T>
    class UndoRemoveComponent : public IUndoAction
    {
    public:
        UndoRemoveComponent(Entity entity, const T& removedComponent) // Pass by const reference
            : mEntity(entity), mRemovedComponent(removedComponent) {} // Store a copy

        void Undo() override
        {
            ecsInterface.AddComponent<T>(mEntity, mRemovedComponent); // Restore the exact component
        }

        void Redo() override
        {
            ecsInterface.RemoveComponent<T>(mEntity); // Remove the component again
        }

        void Print() const override
        {
            std::cout << "Undo Remove: Restoring component to entity " << mEntity << std::endl;
        }

    private:
        Entity mEntity;         // Entity from which the component was removed
        T mRemovedComponent;    // Full copy of the removed component
    };

    /**
     * @brief Manages undo and redo actions.
     *
     * Stores a stack of actions and provides methods to undo and redo changes.
     */
    class UndoRedoManager
    {
    public:

        static constexpr size_t MAX_UNDO_REDO = 100; // Maximum number of undo/redo steps

        /**
         * @brief Records an undo action for a component variable change.
         *
         * @tparam T Type of the variable being modified.
         * @param entity Entity being modified.
         * @param componentName Name of the component.
         * @param varName Name of the variable.
         * @param var Reference to the variable.
         * @param prevValue Previous value before the change.
         * @param newValue New value after the change.
         */
        template <typename T>
        void PushUndo(Entity entity, const std::string& componentName, const std::string& varName, T& var, T prevValue, T newValue)
        {
            undoStack.push_back(std::make_unique<UndoAction<T>>(entity, componentName, varName, var, prevValue, newValue));
            redoStack.clear(); // Clear redo stack whenever a new change is made
            
            // Limit the undo stack size
            if (undoStack.size() > MAX_UNDO_REDO)
            {
                undoStack.erase(undoStack.begin()); // Remove the oldest undo action
            }
        }

        /**
         * @brief Records an undo action for removing a component.
         *
         * @tparam T Type of the component being removed.
         * @param entity Entity from which the component was removed.
         * @param removedComponent Copy of the removed component.
         */
        template <typename T>
        void PushUndoComponent(Entity entity, T removedComponent)
        {
            undoStack.push_back(std::make_unique<UndoRemoveComponent<T>>(entity, removedComponent));
            redoStack.clear(); // Clear redo stack whenever a new change is made
        
            // Limit the undo stack size
            if (undoStack.size() > MAX_UNDO_REDO)
            {
                undoStack.erase(undoStack.begin()); // Remove the oldest undo action
            }
        }

        // @brief Undoes the last recorded action.
        void Undo()
        {
            if (!undoStack.empty())
            {
                auto action = std::move(undoStack.back());
                undoStack.pop_back();
                action->Undo();
                redoStack.push_back(std::move(action));
                
                // Limit the redo stack size
                if (redoStack.size() > MAX_UNDO_REDO)
                {
                    redoStack.erase(redoStack.begin()); // Remove the oldest redo action
                }
            }
        }

        // @brief Redoes the last undone action.
        void Redo()
        {
            if (!redoStack.empty())
            {
                auto action = std::move(redoStack.back());
                redoStack.pop_back();
                action->Redo();
                undoStack.push_back(std::move(action));
                
                // Limit the undo stack size
                if (undoStack.size() > MAX_UNDO_REDO)
                {
                    undoStack.erase(undoStack.begin()); // Remove the oldest undo action
                }
            }
        }

        // @brief Prints details of the undo and redo stacks.
        void PrintStackDetails() const
        {
            std::cout << "Undo Stack:\n";
            for (const auto& action : undoStack)
            {
                action->Print();
            }

            std::cout << "Redo Stack:\n";
            for (const auto& action : redoStack)
            {
                action->Print();
            }
        }

        // @brief Checks if there are actions available to undo.
        bool CanUndo() const { return !undoStack.empty(); }

        // @brief Checks if there are actions available to redo.
        bool CanRedo() const { return !redoStack.empty(); }

        // @brief Clears both the undo and redo stacks.
        void ClearUndoRedo() 
        {
            undoStack.clear();
            redoStack.clear();
        }

    private:
        std::vector<std::unique_ptr<IUndoAction>> undoStack;    // Stack of undo actions
        std::vector<std::unique_ptr<IUndoAction>> redoStack;    // Stack of redo actions
    };
}
#endif // !_UNDOSYSTEM_H_