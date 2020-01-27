# Block Producers Guide

## Producers earn voting power from being selected by network stakeholders, and mining power by broadcasting proofs of work.

### Begin by starting a full node on a reliable server.

### Use the producer_update operation to create your account's producer.

Include in the operation:

    account_name_type  owner   
    
    // Your account name.

    string   details   
    
    // A description of your producer setup and information about you for voting consideration.

    string   url    
    
    // A URL that explains more details about your producer proposal.

    string   json   
    
    // A JSON stringified object containing additional attributes.

    double   latitude  
    
    // Approximate Latitude co-ordinates of your node.

    double   longitude   
    
    // Approximate Longitude co-ordinates of your node.

    string   block_signing_key  
    
    // The public key used to sign blocks for your producer.

    chain_properties    props   
    
    //  Blockchain properties for the network, used to adjust variables for changing the way the network operates.

    bool     active = true   
    
    // Set active to true to activate, false to deactivate; 

