/*
 * This material is based upon work supported by the Defense Advanced
 * Research Projects Agency under Contract No. N66001-11-C-4017.
 *
 * Copyright 2014 - Raytheon BBN Technologies Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef CURVEBALL_SENTINELDETECTOR_HH
#define CURVEBALL_SENTINELDETECTOR_HH
#include <click/element.hh>
#include <click/ipflowid.hh>
#include <click/timer.hh>
#include "bloomfilter.hh"
#include "flowtable.hh"
#include "dr2dpencoder.hh"
CLICK_DECLS


class DHBlacklistEntry {

  public:

    DHBlacklistEntry() {}
    DHBlacklistEntry(const IPAddress & addr):
        _addr(addr) {}
    DHBlacklistEntry(const IPAddress & addr, const IPAddress & mask):
        _addr(addr), _mask(mask) {}
    ~DHBlacklistEntry() {}

    const IPAddress addr() const { return _addr; }
    const IPAddress mask() const { return _mask; } 

  private:

    IPAddress	_addr;
    IPAddress	_mask;
};


class SentinelDetector : public Element { public:

    SentinelDetector(int sentinel_length = 0);
    ~SentinelDetector();

    const char *class_name() const	{ return "SentinelDetector"; }
    const char *port_count() const	{ return "2/3"; }
    const char *processing() const	{ return PUSH; }
    const char *flow_code()  const	{ return COMPLETE_FLOW; }

    int configure(Vector<String> &, ErrorHandler *);
    int initialize(ErrorHandler *);
    void cleanup(CleanupStage);

    // Installs the element's handlers.
    void add_handlers();

    // Process any configured timers that have fired.
    void run_timer(Timer *timer);

    // Uploads a new sentinel bloom filter.
    void update_sentinel_filter(const BloomFilter * filter);

    // uploads decoy host blacklist
    void update_dh_blacklist(const Vector<DHBlacklistEntry> & blacklist);

    // Accessor to the sentinel filter; for debugging.
    const BloomFilter * filter() const { return _sentinels; }

    // Removes an entry from the flow table.
    void remove_flow(const IPFlowID &flow_key)
             { _flow_table.remove_flow(flow_key); }

    // Handle incoming udp flow notification.
    void incoming_udp_notification(const IPFlowID &flow_key,
                                   const String &sentinel);

  protected:

    // Handles tcp server-side traffic.
    void process_server_packet(Packet *p);
    void process_server_ack(Packet *p, FlowEntry *entry);

    // Generate UDP notification message.
    void generate_udp_notification(const Packet *p,
                                   const char *sentinel,
                                   unsigned int sentinel_len);

    // Returns true if the DR has been notified that another DR has
    // already seen and is handling the flow.
    bool seen_flow(const IPFlowID &flow_key, const char *buf, int len);

    // determine if the decoy host has been blacklisted
    bool is_blacklisted(const IPAddress & decoy_host);

    // Callback used to process read handlers.
    static String read_handler(Element *, void *);

    // Valid Curveball sentinels.
    const BloomFilter *	_sentinels;

    // Length (in bytes) of sentinel to extract from packet.
    int			_sentinel_length;

    // Known string sentinel that marks packets for Curveball redirection.
    String 		_sentinel;

    // deocy host blacklist
    Vector<DHBlacklistEntry> _dh_blacklist;

    // Table that manages Curveball flows.
    FlowTable		_flow_table;

    // Time interval to identify inactive Curveball flows.
    uint32_t		_timeout_in_sec;

    // Timer used to periodically inspect flow table for inactive flows.
    Timer		_flow_timer;

    // Element used to encode messages to the decoy proxy.
    DR2DPEncoder *	_encoder;

    // Destination port of inspected traffic.
    uint16_t		_port;

    // Indicates whether or not reverse flow traffic is redirected.
    bool		_forward_reverse;

    // UDP destination port for flow notification packets; a port of 0
    // indicates that no UDP notifications are sent or received
    uint16_t		_udp_port;
    IPAddress		_local_addr;

    // Flows that have already been seen by a previous DR on the path,
    // as determined by UDP flow notifications.
    HashTable<IPFlowID, String> _seen_flows;
};


CLICK_ENDDECLS
#endif
