
/*
 * MARSSx86 : A Full System Computer-Architecture Simulator
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Copyright 2009 Avadh Patel <apatel@cs.binghamton.edu>
 * Copyright 2009 Furat Afram <fafram@cs.binghamton.edu>
 *
 */

#ifdef MEM_TEST
#include <test.h>
#else
#include <ptlsim.h>
#define PTLSIM_PUBLIC_ONLY
#include <ptlhwdef.h>
#endif

#include <splitPhaseBus.h>
#include <memoryHierarchy.h>
#include <machine.h>

using namespace Memory;
using namespace Memory::SplitPhaseBus;

BusInterconnect::BusInterconnect(const char *name,
        MemoryHierarchy *memoryHierarchy) :
    Interconnect(name,memoryHierarchy)
    , lastAccessQueue(NULL)
    , busBusy_(false)
    , dataBusBusy_(false)
{
    memoryHierarchy_->add_interconnect(this);
    new_stats = new BusStats(name, &memoryHierarchy->get_machine());

    reads_user = reads_kernel = writes_user = writes_kernel = 0;

    SET_SIGNAL_CB(name, "_Broadcast", broadcast_, &BusInterconnect::broadcast_cb);

    SET_SIGNAL_CB(name, "_Broadcast_Complete", broadcastCompleted_,
            &BusInterconnect::broadcast_completed_cb);

    SET_SIGNAL_CB(name, "_Data_Broadcast", dataBroadcast_, &BusInterconnect::data_broadcast_cb);

    SET_SIGNAL_CB(name, "_Data_Broadcast_Complete", dataBroadcastCompleted_,
            &BusInterconnect::data_broadcast_completed_cb);

    new_stats->set_default_stats(user_stats);

    if(!memoryHierarchy_->get_machine().get_option(name, "latency", latency_)) {
        latency_ = BUS_BROADCASTS_DELAY;
    }

    if(!memoryHierarchy_->get_machine().get_option(name, "arbitrate_latency",
                arbitrate_latency_)) {
        arbitrate_latency_ = BUS_ARBITRATE_DELAY;
    }

	if (!memoryHierarchy_->get_machine().get_option(name, "disable_snoop",
				snoopDisabled_)) {
		snoopDisabled_ = false;
	}
}

BusInterconnect::~BusInterconnect()
{
    delete new_stats;
}

void BusInterconnect::register_controller(Controller *controller)
{
    BusControllerQueue *busControllerQueue = new BusControllerQueue();
    busControllerQueue->controller = controller;

    /* Set controller pointer in each queue entry */
    BusQueueEntry *entry;
    foreach(i, busControllerQueue->queue.size()) {
        entry = (BusQueueEntry*)&busControllerQueue->queue[i];
        entry->controllerQueue = busControllerQueue;
        entry = (BusQueueEntry*)&busControllerQueue->dataQueue[i];
        entry->controllerQueue = busControllerQueue;
    }

    busControllerQueue->idx = controllers.count();
    controllers.push(busControllerQueue);
}

void BusInterconnect::hit_patch_count(Controller * controller, MemoryRequest *request)
{

  
}

int BusInterconnect::access_fast_path(Controller *controller,
        MemoryRequest *request)
{
    return -1;
}

void BusInterconnect::annul_request(MemoryRequest *request)
{
    foreach(i, controllers.count()) {
        BusQueueEntry *entry;
        foreach_list_mutable(controllers[i]->queue.list(),
                entry, entry_t, nextentry_t) {
            if(entry->request->is_same(request)) {
                entry->annuled = true;
                entry->request->decRefCounter();
                controllers[i]->queue.free(entry);
            }
        }
    }
    PendingQueueEntry *queueEntry;
    foreach_list_mutable(pendingRequests_.list(), queueEntry,
            entry, nextentry) {
        if(queueEntry->request->is_same(request)) {
            queueEntry->annuled = true;
            queueEntry->request->decRefCounter();
            ADD_HISTORY_REM(queueEntry->request);
            pendingRequests_.free(queueEntry);
        }
    }
}

bool BusInterconnect::controller_request_cb(void *arg)
{
    Message *message = (Message*)arg;

    memdebug("Bus received message: ", *message, endl);

    bool kernel = message->request->is_kernel();

    //Controller *sender = (Controller*)message->sender;
    //printf("split_bus: sender = %s\n", sender->get_name());
    /*
     * check if the request is already in pendingRequests_ queue
     * then update the hasData array in that queue
     */
    PendingQueueEntry *pendingEntry;
    foreach_list_mutable(pendingRequests_.list(), pendingEntry,
            entry, nextentry) {
        if(pendingEntry->request == message->request) {
            //printf("split_bus: message request is pending.\n");
            memdebug("Bus Response received for: ", *pendingEntry);
            int idx = -1;
            Controller *sender = (Controller*)message->sender;
            foreach(i, controllers.count()) {
                if(sender == controllers[i]->controller) {
                    idx = i;
                    break;
                }
            }

            pendingEntry->responseReceived[idx] = true;

            /* If response has data mark this controller */
            if(message->hasData) {
                //printf("split_bus: has data.\n");
                pendingEntry->controllerWithData = sender;
            }

            if(sender->is_private()) {
                //printf("split_bus: private.\n");
                pendingEntry->shared |= message->isShared;

                /*
                 * If same level cache responed with data indicating via shared
                 * flag, then we can emit 'annul' signal to all other
                 * controllers that are working on this request
                 */
                if(message->hasData) {
                    foreach(x, controllers.count()) {
                        if(!pendingEntry->responseReceived[x]) {
                            controllers[x]->controller->annul_request(pendingEntry->request);
                            pendingEntry->responseReceived[x] = true;
                        }
                    }
                }
            }

            //printf("split_bus: data_busy:%d\n", dataBusBusy_);
            if(!dataBusBusy_) {
                bool all_set = true;
                foreach(x, pendingEntry->responseReceived.count()) {
                    //printf("pending entry %d response received: %d\n", x, pendingEntry->responseReceived[x]);
                    all_set &= pendingEntry->responseReceived[x];
                }
                if(all_set || (snoopDisabled_ && pendingEntry->controllerWithData)) {
                    dataBusBusy_ = true;
                    //printf("Broadcasting...\n");
                    marss_add_event(&dataBroadcast_, 1,
                            pendingEntry);
                }
            } else {
                N_STAT_UPDATE(new_stats->bus_not_ready, ++, kernel);
            }

            return true;
        }
    }

    /* its a new request, add entry into controllerqueues */
    BusControllerQueue* busControllerQueue = NULL;
    foreach(i, controllers.count()) {
        if(controllers[i]->controller ==
                (Controller*)message->sender) {
            busControllerQueue = controllers[i];
            break;
        }
    }

    if (busControllerQueue->queue.isFull()) {
        N_STAT_UPDATE(new_stats->bus_not_ready, ++, kernel);
        //printf("split_bus queue full.\n");
        memdebug("Bus queue is full\n");
        return false;
    }

    BusQueueEntry *busQueueEntry;
    busQueueEntry = busControllerQueue->queue.alloc();
    if(busControllerQueue->queue.isFull()) {
        memoryHierarchy_->set_interconnect_full(this, true);
    }
    busQueueEntry->request = message->request;
    message->request->incRefCounter();
    busQueueEntry->hasData = message->hasData;


    if(!is_busy()) {
        /* address bus */
        //printf("Broadcasting...\n");
        marss_add_event(&broadcast_, 1, NULL);
        set_bus_busy(true);
    } else {
        N_STAT_UPDATE(new_stats->bus_not_ready, ++, kernel);
        //printf("Bus is busy!\n");
        memdebug("Bus is busy\n");
    }

    return true;
}

BusQueueEntry* BusInterconnect::arbitrate_round_robin()
{
    memdebug("BUS:: doing arbitration.. \n");
    BusControllerQueue *controllerQueue;
    int i;
    if(lastAccessQueue)
        i = lastAccessQueue->idx;
    else
        i = 0;

    do {
        i = (i + 1) % controllers.count();
        controllerQueue = controllers[i];

        if(controllerQueue->queue.count() > 0) {
            BusQueueEntry *queueEntry = (BusQueueEntry*)
                controllerQueue->queue.peek();
            assert(queueEntry);
            assert(!queueEntry->annuled);
            lastAccessQueue = controllerQueue;
            return queueEntry;
        }
    } while(controllerQueue != lastAccessQueue);

    return NULL;
}

bool BusInterconnect::can_broadcast(BusControllerQueue *queue, MemoryRequest *request)
{
    bool isFull = false;
    foreach(i, controllers.count()) {
        if(controllers[i]->controller == queue->controller)
            continue;
        isFull |= controllers[i]->controller->is_full(true, request);
    }
    if(isFull) {
        return false;
    }
    return true;
}

bool BusInterconnect::broadcast_cb(void *arg)
{
    //printf("Inside broadcast cb.\n");
    BusQueueEntry *queueEntry;
    if(arg != NULL)
        queueEntry = (BusQueueEntry*)arg;
    else {
        queueEntry = arbitrate_round_robin();
        marss_add_event(&broadcast_, arbitrate_latency_, queueEntry);
        return true;
    }

    if(queueEntry == NULL || queueEntry->annuled) { // nothing to broadcast
        set_bus_busy(false);
        return true;
    }

    /*
     * first check if pendingRequests_ queue is full or not
     * if its full dont' broadcast
     */
    if(pendingRequests_.isFull() &&
            queueEntry->request->get_type() != MEMORY_OP_UPDATE) {
        memdebug("Bus cant do addr broadcast, pending queue full\n");
        marss_add_event(&broadcast_,
                latency_, NULL);
        return true;
    }

    /*
     * now check if any of the other controller's receive queue is
     * full or not if its full the don't broadcast untill it has a free
     * entry and  pass the queue entry as argument to the broadcast
     * signal so next time it doesn't need to arbitrate
     */
    if(!can_broadcast(queueEntry->controllerQueue, queueEntry->request)) {
        memdebug("Bus cant do addr broadcast\n");
        set_bus_busy(true);
        marss_add_event(&broadcast_,
                latency_, NULL);
        return true;
    }

    set_bus_busy(true);

    marss_add_event(&broadcastCompleted_,
            latency_, queueEntry);

    return true;
}

bool BusInterconnect::broadcast_completed_cb(void *arg)
{
    //printf("Inside broadcast complete cb.\n");
    assert(is_busy());
    BusQueueEntry *queueEntry = (BusQueueEntry*)arg;

    if(queueEntry == NULL || queueEntry->annuled) {
        broadcast_cb(NULL);
        return true;
    }

	if(!can_broadcast(queueEntry->controllerQueue, queueEntry->request)) {
		set_bus_busy(true);
		marss_add_event(&broadcastCompleted_,
				2, NULL);
		return true;
	}

    memdebug("Broadcasing entry: ", *queueEntry, endl);

    /* now create an entry into pendingRequests_ */
    PendingQueueEntry *pendingEntry = NULL;
    if(queueEntry->request->get_type() != MEMORY_OP_UPDATE &&
			queueEntry->request->get_type() != MEMORY_OP_EVICT) {
        pendingEntry = pendingRequests_.alloc();
        assert(pendingEntry);
        pendingEntry->request = queueEntry->request;
        pendingEntry->request->incRefCounter();
        pendingEntry->controllerQueue = queueEntry->controllerQueue;
        pendingEntry->set_num_controllers(controllers.count());

        ADD_HISTORY_ADD(pendingEntry->request);
        memdebug("Created pending entry: ", *pendingEntry, endl);
    }

    Message& message = *memoryHierarchy_->get_message();
    message.sender = this;
    message.request = queueEntry->request;
    message.hasData = queueEntry->hasData;
    message.origin = NULL;

    Controller *controller = queueEntry->controllerQueue->controller;

    foreach(i, controllers.count()) {
        //printf("Trying to send message to %d\n", i);
        //cout << message << endl;
        if(controller != controllers[i]->controller) {
            //printf("Sending...\n");
            bool ret = controllers[i]->controller->
                get_interconnect_signal()->emit(&message);
            assert(ret);
        } else {
            /*
             * its the originating controller, mark its
             * response received flag to true
             */
            //printf("is original controller.\n");
            if(pendingEntry)
                pendingEntry->responseReceived[i] = true;
        }
    }

    bool kernel = queueEntry->request->is_kernel();

    /* Free the entry from queue */
    queueEntry->request->decRefCounter();
    if(!queueEntry->annuled) {
        queueEntry->controllerQueue->queue.free(queueEntry);
    }
    if(!queueEntry->controllerQueue->queue.isFull()) {
        memoryHierarchy_->set_interconnect_full(this, false);
    }

    /* Update bus stats */
    N_STAT_UPDATE(new_stats->addr_bus_cycles, += latency_,
            kernel);
    if(pendingEntry) {
        switch(pendingEntry->request->get_type()) {
            case MEMORY_OP_READ: N_STAT_UPDATE(new_stats->broadcasts.read, ++, kernel);
                                 break;
            case MEMORY_OP_WRITE: N_STAT_UPDATE(new_stats->broadcasts.write, ++, kernel);
                                  break;
            default: assert(0);
        }
    } else { // On memory update we don't use any pending entry
        N_STAT_UPDATE(new_stats->broadcasts.update, ++, kernel);
        N_STAT_UPDATE(new_stats->broadcast_cycles.update, ++, kernel);
    }

    /* Free the message */
    memoryHierarchy_->free_message(&message);

    /*
     * call broadcast_cb that will check if any pending
     * requests are there or not
     */
    broadcast_cb(NULL);

    return true ;
}

/*
 * This function check the pending request queue for an entry that has all the
 * responses received and setup the data bus to broadcast the response of that
 * entry.
 */
void BusInterconnect::set_data_bus()
{
    /* check if any other pending request received all the responses */
    PendingQueueEntry *pendingEntry;
    foreach_list_mutable(pendingRequests_.list(), pendingEntry,
            entry, nextentry) {
        bool all_set = true;
        foreach(x, pendingEntry->responseReceived.count()) {
            all_set &= pendingEntry->responseReceived[x];
        }
        if(all_set || (snoopDisabled_ && pendingEntry->controllerWithData)) {
            dataBusBusy_ = true;
            marss_add_event(&dataBroadcast_, 1,
                    pendingEntry);
            return;
        }
    }

    /* There is no pending entry with the response ready*/
    dataBusBusy_ = false;
}

bool BusInterconnect::data_broadcast_cb(void *arg)
{
    //printf("broadcast_cb HERE\n");
    PendingQueueEntry *pendingEntry = (PendingQueueEntry*)arg;
    assert(pendingEntry);

    /*
     * now check if any of the other controller's receive queue is
     * full or not if its full the don't broadcast untill it has a free
     * entry and  pass the queue entry as argument to the broadcast
     * signal so next time it doesn't need to arbitrate
     */
    if(!can_broadcast(pendingEntry->controllerQueue, pendingEntry->request)) {
        marss_add_event(&dataBroadcast_,
                latency_, arg);
        return true;
    }

    if(pendingEntry->annuled) {
        set_data_bus();
        return true;
    }

    marss_add_event(&dataBroadcastCompleted_,
            latency_, pendingEntry);

    return true;
}

bool BusInterconnect::data_broadcast_completed_cb(void *arg)
{
    //printf("broadcast_complete_cb HERE\n");
    PendingQueueEntry *pendingEntry = (PendingQueueEntry*)arg;
    assert(pendingEntry);

    if(pendingEntry->annuled) {
        set_data_bus();
        return true;
    }

    Message& message = *memoryHierarchy_->get_message();
    message.sender = this;
    message.request = pendingEntry->request;
    message.hasData = true;
    message.isShared = pendingEntry->shared;
    message.origin = NULL;

    foreach(i, controllers.count()) {
        if(pendingEntry->controllerWithData == controllers[i]->controller) {
            /* Don't send the data message back to the responding controller */
            continue;
        }

        bool ret = controllers[i]->controller->
            get_interconnect_signal()->emit(&message);
        assert(ret);
    }

    /* Update bus stats */
    bool kernel = pendingEntry->request->is_kernel();

    N_STAT_UPDATE(new_stats->data_bus_cycles, += latency_, kernel);
    W64 delay = sim_cycle - pendingEntry->initCycle;
    assert(delay > (W64)latency_);
    switch(pendingEntry->request->get_type()) {
        case MEMORY_OP_READ: N_STAT_UPDATE(new_stats->broadcast_cycles.read, += delay, kernel);
                             break;
        case MEMORY_OP_WRITE: N_STAT_UPDATE(new_stats->broadcast_cycles.write, += delay, kernel);
                              break;
        default: assert(0);
    }

    pendingEntry->request->decRefCounter();
    pendingRequests_.free(pendingEntry);
    ADD_HISTORY_REM(pendingEntry->request);

    memoryHierarchy_->free_message(&message);

    set_data_bus();

    return true;
}

/**
 * @brief Dump Split Bus Interconnect Configuration in YAML Format
 *
 * @param out YAML Object
 */
void BusInterconnect::dump_configuration(YAML::Emitter &out) const
{
	out << YAML::Key << get_name() << YAML::Value << YAML::BeginMap;

	YAML_KEY_VAL(out, "type", "interconnect");
	YAML_KEY_VAL(out, "latency", latency_);
	YAML_KEY_VAL(out, "arbitrate_latency", arbitrate_latency_);
	if (controllers.size() > 0)
		YAML_KEY_VAL(out, "per_cont_queue_size",
				controllers[0]->queue.size());

	out << YAML::EndMap;
}

void BusInterconnect::reset_lastcycle_stats()
{
	new_stats->broadcast_cycles.read(user_stats) = reads_user;
	new_stats->broadcast_cycles.read(kernel_stats) = reads_kernel;
	new_stats->broadcast_cycles.write(user_stats) = writes_user;
        new_stats->broadcast_cycles.write(kernel_stats) = writes_kernel;
}

void BusInterconnect::dump_mcpat_configuration(root_system *mcpatData, W32 count)
{
	int idx = mcpatData->number_of_NoCs;
	system_NoC *noc = &(mcpatData->NoC[idx]);
	noc->clockrate = mcpatData->target_core_clockrate;
	noc->type = 0;
	noc->input_ports = 1;
	noc->output_ports = 1;
	noc->duty_cycle = 1;
	noc->has_global_link = false;
	noc->link_latency = latency_ * mcpatData->target_core_clockrate;	
	noc->link_throughput = 1;
	noc->input_buffer_entries_per_vc = controllers[0]->queue.size();
	noc->flit_bits = 256;
	mcpatData->number_of_NoCs++;
	double coverage = 1.0/mcpatData->number_of_NoCs;
	for (int i = 0; i < mcpatData->number_of_NoCs; i++) {
		system_NoC *n = &(mcpatData->NoC[i]);
		n->chip_coverage = coverage;
	}
	cout << " num noc: " << mcpatData->number_of_NoCs << " noc chip coverage: " << noc->chip_coverage << endl;
}

void BusInterconnect::dump_mcpat_stats(root_system *mcpatData, W32 idx)
{
	system_NoC *noc = &(mcpatData->NoC[idx]);
	W64 ruser = new_stats->broadcast_cycles.read(user_stats);
	W64 rkernel = new_stats->broadcast_cycles.read(kernel_stats);
	W64 wuser = new_stats->broadcast_cycles.write(user_stats);
        W64 wkernel = new_stats->broadcast_cycles.write(kernel_stats);
	noc->total_accesses = (ruser + rkernel + wuser + wkernel) - (reads_user + reads_kernel + writes_user + writes_kernel);
	reads_user = ruser;
	reads_kernel = rkernel;
	writes_user = writes_user;
	writes_kernel = writes_kernel;
}

struct SplitPhaseBusBuilder : public InterconnectBuilder
{
    SplitPhaseBusBuilder(const char* name) :
        InterconnectBuilder(name)
    { }

    Interconnect* get_new_interconnect(MemoryHierarchy& mem,
            const char* name)
    {
        return new SplitPhaseBus::BusInterconnect(name, &mem);
    }
};

SplitPhaseBusBuilder splitPhaseBusBuilder("split_bus");
