#include <bts/blockchain/asset_operations.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/exceptions.hpp>

namespace bts { namespace blockchain {

   bool is_power_of_ten( uint64_t n )
   {
      switch( n )
      {
         case 1ll:
         case 10ll:
         case 100ll:
         case 1000ll:
         case 10000ll:
         case 100000ll:
         case 1000000ll:
         case 10000000ll:
         case 100000000ll:
         case 1000000000ll:
         case 10000000000ll:
         case 100000000000ll:
         case 1000000000000ll:
         case 10000000000000ll:
         case 100000000000000ll:
         case 1000000000000000ll:
            return true;
         default:
            return false;
      }
   }

   void create_asset_operation::evaluate( transaction_evaluation_state& eval_state )
   { try {
      if( NOT eval_state._current_state->is_valid_symbol_name( this->symbol ) )
          FC_CAPTURE_AND_THROW( invalid_asset_symbol, (symbol) );

      oasset_record current_asset_record = eval_state._current_state->get_asset_record( this->symbol );
      if( current_asset_record.valid() )
          FC_CAPTURE_AND_THROW( asset_symbol_in_use, (symbol) );

      if( this->name.empty() )
          FC_CAPTURE_AND_THROW( invalid_asset_name, (this->name) );

      const asset_id_type asset_id = eval_state._current_state->last_asset_id() + 1;
      current_asset_record = eval_state._current_state->get_asset_record( asset_id );
      if( current_asset_record.valid() )
          FC_CAPTURE_AND_THROW( asset_id_in_use, (asset_id) );

      if( issuer_account_id != asset_record::market_issued_asset )
      {
         const oaccount_record issuer_account_record = eval_state._current_state->get_account_record( this->issuer_account_id );
         if( NOT issuer_account_record.valid() )
             FC_CAPTURE_AND_THROW( unknown_account_id, (issuer_account_id) );
      }

      if( this->maximum_share_supply <= 0 || this->maximum_share_supply > BTS_BLOCKCHAIN_MAX_SHARES )
          FC_CAPTURE_AND_THROW( invalid_asset_amount, (this->maximum_share_supply) );

      if( NOT is_power_of_ten( this->precision ) )
          FC_CAPTURE_AND_THROW( invalid_precision, (this->precision) );

      const asset reg_fee( eval_state._current_state->get_asset_registration_fee( this->symbol.size() ), 0 );
      eval_state.required_fees += reg_fee;

      asset_record new_record;
      new_record.id                     = eval_state._current_state->new_asset_id();
      new_record.symbol                 = this->symbol;
      new_record.name                   = this->name;
      new_record.description            = this->description;
      new_record.public_data            = this->public_data;
      new_record.issuer_account_id      = this->issuer_account_id;
      new_record.precision              = this->precision;
      new_record.registration_date      = eval_state._current_state->now();
      new_record.last_update            = new_record.registration_date;
      new_record.current_share_supply   = 0;
      new_record.maximum_share_supply   = this->maximum_share_supply;
      new_record.collected_fees         = 0;

      eval_state._current_state->store_asset_record( new_record );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   void update_asset_operation::evaluate( transaction_evaluation_state& eval_state )
   { try {
      oasset_record current_asset_record = eval_state._current_state->get_asset_record( this->asset_id );
      if( NOT current_asset_record.valid() )
          FC_CAPTURE_AND_THROW( unknown_asset_id, (asset_id) );

      if( current_asset_record->is_market_issued() )
          FC_CAPTURE_AND_THROW( not_user_issued, (*current_asset_record) );

      // Reject no-ops
      FC_ASSERT( this->name.valid() || this->description.valid() || this->public_data.valid()
                 || this->maximum_share_supply.valid() || this->precision.valid() );

      // Cannot update name, description, max share supply, or precision if any shares have been issued
      if( current_asset_record->current_share_supply > 0 )
      {
          FC_ASSERT( !this->name.valid() );
          FC_ASSERT( !this->description.valid() );
          FC_ASSERT( !this->maximum_share_supply.valid() );
          FC_ASSERT( !this->precision.valid() );
      }

      const account_id_type& current_issuer_account_id = current_asset_record->issuer_account_id;
      const oaccount_record current_issuer_account_record = eval_state._current_state->get_account_record( current_issuer_account_id );
      if( NOT current_issuer_account_record.valid() )
          FC_CAPTURE_AND_THROW( unknown_account_id, (current_issuer_account_id) );

      if( NOT eval_state.account_or_any_parent_has_signed( *current_issuer_account_record ) )
          FC_CAPTURE_AND_THROW( missing_signature, (*current_issuer_account_record) );

      if( this->name.valid() )
      {
          if( this->name->empty() )
              FC_CAPTURE_AND_THROW( invalid_asset_name, (*this->name) );

          current_asset_record->name = *this->name;
      }

      if( this->description.valid() )
          current_asset_record->description = *this->description;

      if( this->public_data.valid() )
          current_asset_record->public_data = *this->public_data;

      if( this->maximum_share_supply.valid() )
      {
          if( *this->maximum_share_supply <= 0 || *this->maximum_share_supply > BTS_BLOCKCHAIN_MAX_SHARES )
              FC_CAPTURE_AND_THROW( invalid_asset_amount, (*this->maximum_share_supply) );

          current_asset_record->maximum_share_supply = *this->maximum_share_supply;
      }

      if( this->precision.valid() )
      {
          if( NOT is_power_of_ten( *this->precision ) )
              FC_CAPTURE_AND_THROW( invalid_precision, (*this->precision) );

          current_asset_record->precision = *this->precision;
      }

      current_asset_record->last_update = eval_state._current_state->now();

      eval_state._current_state->store_asset_record( *current_asset_record );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   void issue_asset_operation::evaluate( transaction_evaluation_state& eval_state )
   { try {
      oasset_record current_asset_record = eval_state._current_state->get_asset_record( this->amount.asset_id );
      if( NOT current_asset_record.valid() )
          FC_CAPTURE_AND_THROW( unknown_asset_id, (amount.asset_id) );

      if( current_asset_record->is_market_issued() )
          FC_CAPTURE_AND_THROW( not_user_issued, (*current_asset_record) );

      const account_id_type& issuer_account_id = current_asset_record->issuer_account_id;
      const oaccount_record issuer_account_record = eval_state._current_state->get_account_record( issuer_account_id );
      if( NOT issuer_account_record.valid() )
          FC_CAPTURE_AND_THROW( unknown_account_id, (issuer_account_id) );

      if( NOT eval_state.account_or_any_parent_has_signed( *issuer_account_record ) )
          FC_CAPTURE_AND_THROW( missing_signature, (*issuer_account_record) );

      if( this->amount.amount <= 0 )
          FC_CAPTURE_AND_THROW( negative_issue, (amount) );

      if( NOT current_asset_record->can_issue( this->amount ) )
          FC_CAPTURE_AND_THROW( over_issue, (amount)(*current_asset_record) );

      current_asset_record->current_share_supply += this->amount.amount;
      eval_state.add_balance( this->amount );

      current_asset_record->last_update = eval_state._current_state->now();

      eval_state._current_state->store_asset_record( *current_asset_record );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

} } // bts::blockchain
