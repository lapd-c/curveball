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

#ifndef CURVEBALL_BITTORRENTDETECTOR_HH
#define CURVEBALL_BITTORRENTDETECTOR_HH
#include "sentineldetector.hh"
CLICK_DECLS

// Element that detects and redirects Curveball packets.
//
// It is assumed that incoming packets are BitTorrent protocol messages.
// Packets to be redirected to the Curveball system are pushed out the
// element's outbound interface 0. All other packets are pushed out
// interface 1.

class BitTorrentDetector : public SentinelDetector { public:

    BitTorrentDetector();
    ~BitTorrentDetector();

    const char *class_name() const	{ return "BitTorrentDetector"; }

    void * cast(const char *name);

  private:

    // handles incoming non-SYN TCP packets
    virtual void process_non_syn_packet(Packet *p);

    // inspects data flow for sentinel
    void process_bittorrent_data_packet(Packet *p, FlowEntry *entry);
    void process_sentinel_segment(Packet *p, FlowEntry *entry);

};


CLICK_ENDDECLS
#endif
