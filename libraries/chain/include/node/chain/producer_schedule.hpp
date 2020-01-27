#pragma once

namespace node { namespace chain {

class database;

void update_producer_schedule( database& db );
void reset_voting_virtual_schedule_time( database& db );
void reset_mining_virtual_schedule_time( database& db );

} }
