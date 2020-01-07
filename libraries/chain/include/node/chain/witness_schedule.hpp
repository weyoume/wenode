#pragma once

namespace node { namespace chain {

class database;

void update_witness_schedule( database& db );
void reset_witness_virtual_schedule_time( database& db );
void reset_miner_virtual_schedule_time( database& db );

} }
