/**
 * @file Composite.h
 * @brief defines The structure for WorkComponents
 */

#ifndef COMPOSITE_H
#define COMPOSITE_H

#include "Common.h" // mo for PriorityLevel and Clock
#include "CompletedState.h"
#include "Handler.h"
#include "InProgressState.h"
#include "Memento.h"
#include "PendingState.h"
#include "WorkItem.h"
#include "WorkState.h"
#include "WorkflowSubject.h"
#include "WorkVisitor.h"
#include <iostream>
#include <string>
#include <vector>

class WorkMemento;
class WorkGroup;
class WorkComponent : public Handler, public WorkflowSubject {

/**
 * @class WorkComponent
 * @brief Abstract inteface for composite and leaf components
 */
  public:
  /**
   * @brief Creates component in Pending state, with random 8-digit hex ID
   * @param name display name of component, cannot be duplicate
   */
  WorkComponent(std::string name);
  /**
   * @brief Copy constructor used by clone(), deep-copies state and item data
   *  Copies original's ID, name and assigned worker, but no parent
   * @param other Component to copy
   */
  WorkComponent(const WorkComponent &other);
  /**
   * @brief delete object when = used to prevent improper copying
   */
  WorkComponent& operator=(const WorkComponent&) = delete;
  
  /** @brief Deletes owned state and item */
  virtual ~WorkComponent();

  // getters
  /** 
   * @brief getter for object's state
   * @return const state object to prevent changing object variable outside of object
    */
  virtual const WorkState* getState() const; // mo
  /**
   * @brief getter for object's state as a string
   * @return Object's state as a strng
   */
  virtual std::string getStatus() const;     // "Pending", "In Progress" or "Completed"
  /**
   * @brief getter for object's name
   * @return Object's name 
   */
  virtual std::string getName() const; // mo
  /**
   * @brief getter for object's active Item
   * @return Pointer to item, don't have const, we actually want editting outside
   */
  virtual WorkItem* getItem() const;
  /**
   * @brief getter for object's ID
   * @return objects randomly generated 8-digit hex id
   */
  virtual std::string getID() const;
  /**
   * @brief getter for object's workedID if it was assigned one
   * @return  worker's ID
   */
  virtual std::string getAssignedWorkerId() const;
  /**
   * @brief getter for objects data through item object. Forward call to item->getData()
   * @return return the value corresponding to the key in the item if it exists, empty string otherise
   */
  virtual std::string getData(const std::string &key) const;
  /**
   * @brief getter for object's priority
   * @return Object's PriorityLevel (enum)
   */
  virtual PriorityLevel getPriority() const;

  // setters
  /**
   * @brief Delete current state and replace with new state
   *      notifies observers of state change
   *      asks parent to refresh own state from its children
   * @param state new state to replace original 
   */
  void setState(WorkState *state);
  /**
   * @brief sets parent that this component belongs to, allow for state to propagate upward
   * @param newParent new parent component for object
   */
  virtual void setParent(WorkGroup* newParent);   // the group this component sits in
  /**
   * @brief Writes value to component's item, notifies observers
   * @param key Data key
   * @param value Value to store
   */
  virtual void setData(const std::string &key, const std::string &value); // mo
  /**
   * @brief Assign worker to this component
   * @param workerID ID of worker
   * @return True on success
   */
  virtual bool assignTask(const std::string& workerID);

  // mo added virtual functions for decorator
  /**
   * @brief go to next state, delegates to current State's handleUpdate() */
  virtual void nextState();
  /** 
   * @brief checks if object complelte
   * @return true if completed
  */
  virtual bool isComplete() const;
  /**
   * @brief transition from Pending > in Progress, if no worker assigned or not pending then fails
   * @return true if started
   */
  virtual bool startTask();
  /**
   * @brief transition from in progress > completed, if not in progress then fails
   * @return true if completed
   */
  virtual bool completeTask();

  /**
   * @brief Visitor entry pointer abstract, call matching visitor.visit() 
   * @param visitor Visitor to accept 
   */
  virtual void accept(WorkVisitor& visitor) const = 0;
  /**
   * @brief Prototype operation, creates deep copy of this component and children (if it has)
   * @return New copy, caller takes ownership
   */
  virtual WorkComponent* clone() const override = 0;

  /**
   * @brief Adds child, default to deleting child and return false
   * @param child Child to add, ownership taken even for failure
   * @return true if child was added
   */
  virtual bool add(WorkComponent* child);
  /**
   * @brief Removes and deletes child by pointer, default delete child
   * @param child Child to remove
   */
  virtual void remove(WorkComponent* child);
  /**
   * @brief Removes and deletes child by pointer, default does nothing
   * @param name Name of child to remove
   */
  virtual void remove(std::string name);
  /**
   * @brief Return copy of child list, default does returns empty
   * @return Child list
   */
  virtual const std::vector<WorkComponent*> getChildren() const;

  // defaults live in WorkComponent.cpp; WorkUnit overrides the memento pair
  /**
   * @brief Memento method, captures component's ID, state and WorkerID, only WorkUnit calls, default returns null
   * @return New Memento with object's state, or nullptr by default
   */
  virtual WorkMemento *createMemento();
  /**
   * @brief Memento method, restore perviously captured memento, does nothing by default
   * @param memento Memento to restore from
   */
  virtual void setMemento(WorkMemento *memento);


protected:
  WorkState *state; ///< Current lifecycle state (owning)
  WorkItem *item; ///< Key/value data for component (owning)
  std::string name; ///< Display name
  std::string taskID; ///< Random 8-digit hex ID
  std::string assignedWorkerID; ///< Assigned worker, empty if none
  WorkGroup* parent; ///<Containing group (non-owning), or nullptr

  /** @brief Generates random 8-digit hex string (std::mt19937) */
  std::string generateRandomID() const;
};

/**
 * @class WorkGroup
 * @brief Composite, owns and manages child components
 *    instantiable but still acts as interface for other composites
 */

class WorkGroup : public WorkComponent {
public:
  /** 
   * @brief creates empty group with name 
   * @param name Display name
   */
  WorkGroup(std::string name);
  /**
   * @brief Copy constructor, deep copies group and every child, re-parenting clones to copy
   * @param other Group to copy
   */
  WorkGroup(const WorkGroup &other);
  /**
   * @brief delete object when = used to prevent improper copying
   */
  WorkGroup& operator=(const WorkGroup&) = delete;
  /**
   * @brief Destructor, deletes all children
   */
  ~WorkGroup() override;

  /**
   * @brief Adds child to child if nesting is valid and name unique among siblings
   *    Takes ownership of child, even if rejected, it is deleted
   *    Set child's parent on successful add
   * @param child Child to add
   * @return True if added, false if rejected
   */
  bool add(WorkComponent* child) override;
  /**
   * @brief removes and deletes child, refreshes group state
   * @param child Child to remove
   */
  void remove(WorkComponent *child) override;
  /**
   * @brief removes and deletes child with given name, refreshes group state
   * @param name Child's name
   */
  void remove(std::string name) override;
  /**
   * @brief Return copy of child list
   * @return copy of child list
   */
  const std::vector<WorkComponent *> getChildren() const override;

  /**
   * @brief Visitor entry, call matching visitor.visit() 
   * @param visitor Visitor to accept 
   */
  virtual void accept(WorkVisitor &visitor) const override;
  /**
   * @brief Prototype operation, creates deep copy of this component and children
   * @return New copy, caller takes ownership
   */
  virtual WorkGroup *clone() const;

  /**
   * @brief Chain of Responsibility method, step for group
   *      print group's status ifgroup not complete and interactive, offers to complete every tsk in it through commands
   *      If incomplete afterwards, chain auses, otherwise request passed to next handler
   * @param item request traveling along chain
   */
  void handleRequest(WorkItem item);
  /**
   * @brief computes group's state from its child and applies it if it changed
   *      Does nothing on empty group
   */
  void refreshState();   // state follows the children
  /**
   * @brief Enables or disables console prompting in handleRequest
   * @param on False when facade drives chain and prompts must not attempt
   */
  void setInteractive(bool on);   // false: handleRequest never prompts (used when the facade drives the chain)
private:

  //WorkMemento *createMemento() override;
  //void setMemento(WorkMemento *memento) override;
  /** 
   * @brief Always fails, group's state must follow children
   * @return false
   */
  bool startTask() override;
  /**
   * @brief Always fails, group's state must follow children
   * @return false
   */
  bool completeTask() override;
  /**
   * @brief Set group's name
   * @param workerID used as new name
   * @return true
   */
  bool assignTask(const std::string &workerId) override;

  std::vector<WorkComponent*> children; ///< children (owning)
  bool interactive; ///< Boolean flag for interactive prompts
  /**
   * @brief Recursively prints name of all sub-tasks beneath component
   * @param component Starting point
   */
    void printTasks(WorkComponent* component);
    friend class WorkflowBuilder;
};


/** 
 * @class Stage 
 * @brief Composite class, sits below WorkGroup, above SubProcess
 */
class Stage : public WorkGroup {
public: 
  /**
   * @brief Creates empty stage
   * @param name Display name
   */
  Stage(std::string name);

  /**
   * @brief Visitor entry, call matching visitor.visit() 
   * @param visitor Visitor to accept 
   */
  void accept(WorkVisitor &visitor) const override;
  /**
   * @brief Prototype operation, creates deep copy of this component and children
   * @return New copy, caller takes ownership
   */
  Stage *clone() const override;
};

/** 
 * @class SubProcess 
 * @brief Composite class, sits below Stage, above WorkUnit
 */
class SubProcess : public WorkGroup {
public:
  /**
   * @brief Creates empty sub process
   * @param name Display name
   */
  SubProcess(std::string name);

  /**
   * @brief Visitor entry, call matching visitor.visit() 
   * @param visitor Visitor to accept 
   */
  void accept(WorkVisitor &visitor) const override;
  /**
   * @brief Prototype operation, creates deep copy of this component and children
   * @return New copy, caller takes ownership
   */
  SubProcess *clone() const override;
};

/**
 * @class WorkUnit
 * @brief Leaf: single unit of work that can be assigned, started and completed
 */
class WorkUnit : public WorkComponent {
public:

  /**
   * @brief creates work unit with pending state
   * @param taskID Display name of unit
   */
  WorkUnit(const std::string &taskId);
  /**
   * @brief Visitor entry, call matching visitor.visit() 
   * @param visitor Visitor to accept 
   */
  void accept(WorkVisitor &visitor) const override;
  /**
   * @brief Prototype operation, creates deep copy of this component
   * @return New copy, caller takes ownership
   */
  WorkUnit *clone() const override;
  /**
   * @brief Memento method, captures id, state and workerID
   * @return new memento, caller takes ownership
   */
  WorkMemento *createMemento() override;
  /**
   * @brief Memento method, restores id, workerID and state
   * @param memento Memento to restore from, null is ignored
   */
  void setMemento(WorkMemento *memento) override;
  // startTask / completeTask / assignTask / getAssignedWorkerId are inherited from WorkComponent
};

#endif

// #ifndef COMPOSITE_H
// #define COMPOSITE_H

// #include "Common.h" // mo for PriorityLevel and Clock
// #include "CompletedState.h"
// #include "Handler.h"
// #include "InProgressState.h"
// #include "Memento.h"
// #include "PendingState.h"
// #include "WorkItem.h"
// #include "WorkState.h"
// #include "WorkflowSubject.h"
// #include "WorkVisitor.h"
// #include <iostream>
// #include <string>
// #include <vector>

// class WorkMemento;
// class WorkComponent : public Handler, public WorkflowSubject {

    
// public:
//   WorkComponent(std::string name);
//   WorkComponent(const WorkComponent &other);
//   virtual ~WorkComponent();

//   // getters
//   virtual const WorkState* getState() const; // mo
//   virtual std::string getName() const; // mo
//   virtual WorkItem* getItem() const;
//   virtual std::string getID() const;
//   virtual std::string getAssignedWorkerId() const;
//   virtual std::string getData(const std::string &key) const;
//   virtual PriorityLevel getPriority() const;

//   // setters
//   void setState(WorkState *state);
//   virtual void setData(const std::string &key, const std::string &value); // mo
//   virtual bool assignTask(const std::string& workerID);

//   // mo added virtual functions for decorator
//   virtual void nextState();
//   virtual bool isComplete() const;
//   virtual bool startTask();
//   virtual bool completeTask();

//   virtual void accept(WorkVisitor& visitor) const = 0;
//   virtual WorkComponent* clone() const override = 0;

//   virtual bool add(WorkComponent* child);
//   virtual void remove(WorkComponent* child);
//   virtual void remove(std::string name);
//   virtual const std::vector<WorkComponent*> getChildren() const;

//   //changed commands to be connected to each task
//   virtual WorkMemento *createMemento() =0;
//   virtual void setMemento(WorkMemento *memento) =0;
//   virtual bool startTask() =0;
//   virtual bool completeTask() =0;
//   virtual bool assignTask(const std::string &workerId) =0;


// protected:
//   WorkState *state;
//   WorkItem *item;
//   std::string name;
//   std::string taskID;
//   std::string assignedWorkerID;

//   std::string generateRandomID() const;
// };

// class WorkGroup : public WorkComponent {
// public:
//   WorkGroup(std::string name);
//   WorkGroup(const WorkGroup &other);
//   ~WorkGroup() override;

//   bool add(WorkComponent* child) override;
//   void remove(WorkComponent *child) override;
//   void remove(std::string name) override;
//   const std::vector<WorkComponent *> getChildren() const override;

//   virtual void accept(WorkVisitor &visitor) const override;
//   virtual WorkGroup *clone() const;

//   void handleRequest(WorkItem item);
// private:

//   //WorkMemento *createMemento() override;
//   //void setMemento(WorkMemento *memento) override;
//   bool startTask() override;
//   bool completeTask() override;
//   bool assignTask(const std::string &workerId) override;
//   std::vector<WorkComponent*> children;
//     void printTasks(WorkComponent* component);
//     friend class WorkflowBuilder;

//   // ensure proper division of composite classes
// protected:

    
// };

// class Stage : public WorkGroup {
// public:
//   Stage(std::string name);

//   void accept(WorkVisitor &visitor) const override;
//   Stage *clone() const override;
// };

// class SubProcess : public WorkGroup {
// public:
//   SubProcess(std::string name);

//   void accept(WorkVisitor &visitor) const override;
//   SubProcess *clone() const override;
// };

// class WorkUnit : public WorkComponent {
// public:
//   WorkMemento *createMemento();
//   void setMemento(WorkMemento *memento);
//   WorkUnit(const std::string &taskId);
//   std::string getTaskId();
//   std::string getAssignedWorkerId();
//   void accept(WorkVisitor &visitor) const override;
//   WorkUnit *clone() const override;
//   WorkMemento *createMemento() override;
//   void setMemento(WorkMemento *memento) override;
//   bool startTask() override;
//   bool completeTask() override;
//   bool assignTask(const std::string &workerId) override;
// };

// #endif