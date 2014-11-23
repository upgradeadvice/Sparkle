#pragma once

#include <bts/blockchain/transaction.hpp>

namespace bts { namespace blockchain {

   struct block_header
   {
       digest_type   digest()const;
       block_id_type id()const;
       int64_t       difficulty()const;

       block_header():block_num(0){}

       block_id_type        previous;
       uint32_t             block_num;
       fc::time_point_sec   timestamp;
       digest_type          transaction_digest;
       uint64_t             nonce = 0;
       address              miner;
   };

   typedef block_header signed_block_header;

   struct digest_block : public signed_block_header
   {
       digest_block( const signed_block_header& c )
       :signed_block_header(c){}

       digest_block(){}

       bool                             validate_digest()const;
       bool                             validate_unique()const;
       digest_type                      calculate_transaction_digest()const;
       std::vector<transaction_id_type> user_transaction_ids;
   };

   struct full_block : public signed_block_header
   {
       size_t               block_size()const;
       signed_transactions  user_transactions;

       operator digest_block()const;
   };

} } // bts::blockchain

FC_REFLECT( bts::blockchain::block_header,
            (previous)(block_num)(timestamp)(transaction_digest)(nonce)(miner) )
FC_REFLECT_DERIVED( bts::blockchain::digest_block, (bts::blockchain::block_header), (user_transaction_ids) )
FC_REFLECT_DERIVED( bts::blockchain::full_block, (bts::blockchain::block_header), (user_transactions) )
