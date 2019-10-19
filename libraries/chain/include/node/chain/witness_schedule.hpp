#pragma once

namespace node { namespace chain {

class database;

void update_witness_schedule( database& db );
void reset_witness_virtual_schedule_time( database& db );
void reset_miner_virtual_schedule_time( database& db );
vector< account_name_type > shuffle_producers( database& db, vector< account_name_type >& producer_set );

} }
