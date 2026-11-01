/**
 * @file Handler.h
 * @brief Abstract handler for the Chain of Responsibility pattern.
 */

#ifndef HANDLER_H
#define HANDLER_H

#include <string>
#include "WorkItem.h"

/**
 * @class Handler
 * @brief Base class of every link in the chain; holds a pointer to the next handler.
 */
class Handler
{
    protected: 
        /** @brief Next handler in the chain, or nullptr if this is the last one. 
        */
        Handler* successor = nullptr;;
    public: 
        /**
         * @brief Links the handler that receives the request after this one.
         * @param nextPhase The next handler, or nullptr to end the chain here.
         * @note The chain does not own its handlers; it only stores the pointer.
         */
        void setNext(Handler* nextPhase);

        /**
         * @brief Creates a deep copy of this handler (Prototype pattern).
         * @return A newly allocated copy. The caller owns it.
         * @note The copy is not linked to any successor; the new owner must re-link it.
         */
        virtual Handler* clone() const = 0;

        /**
         * @brief Processes the request, then forwards it along the chain.
         * @param item The work item travelling down the chain. 
         */
        virtual void handleRequest(WorkItem item);
};

#endif