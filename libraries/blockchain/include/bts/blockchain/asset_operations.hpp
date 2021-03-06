#pragma once

#include <bts/blockchain/asset.hpp>
#include <bts/blockchain/operations.hpp>
#include <bts/blockchain/types.hpp>

namespace bts { namespace blockchain {

   bool is_power_of_ten( uint64_t n );

   /**
    *  Creates / defines an asset type but does not
    *  allocate it to anyone. Use issue_asset_operation
    */
   struct create_asset_operation
   {
       static const operation_type_enum type;

       /**
        * Symbols may only contain A-Z and 0-9 and up to 5
        * characters and must be unique.
        */
       std::string      symbol;

       /**
        * Names are a more complete description and may
        * contain any kind of characters or spaces.
        */
       std::string      name;
       /**
        *  Describes the asset and its purpose.
        */
       std::string      description;
       /**
        * Other information relevant to this asset.
        */
       fc::variant      public_data;

       /**
        *  Assets can only be issued by individuals that
        *  have registered a name.
        */
       account_id_type  issuer_account_id;

       /** The maximum number of shares that may be allocated */
       share_type       maximum_share_supply = 0;

       uint64_t         precision = 0;

       void evaluate( transaction_evaluation_state& eval_state );
   };

   /**
    * This operation updates an existing issuer record provided
    * it is signed by a proper key.
    */
   struct update_asset_operation
   {
       static const operation_type_enum type;

       asset_id_type                asset_id;
       optional<std::string>        name;
       optional<std::string>        description;
       optional<fc::variant>        public_data;
       optional<share_type>         maximum_share_supply;
       optional<uint64_t>           precision;

       void evaluate( transaction_evaluation_state& eval_state );
   };

   /**
    *  Transaction must be signed by the active key
    *  on the issuer_name_record.
    *
    *  The resulting amount of shares must be below
    *  the maximum share supply.
    */
   struct issue_asset_operation
   {
       static const operation_type_enum type;

       issue_asset_operation( asset a = asset() ):amount(a){}

       asset            amount;

       void evaluate( transaction_evaluation_state& eval_state );
   };

} } // bts::blockchain

FC_REFLECT( bts::blockchain::create_asset_operation,
            (symbol)
            (name)
            (description)
            (public_data)
            (issuer_account_id)
            (maximum_share_supply)
            (precision)
            )
FC_REFLECT( bts::blockchain::update_asset_operation,
            (asset_id)
            (name)
            (description)
            (public_data)
            (maximum_share_supply)
            (precision)
            )
FC_REFLECT( bts::blockchain::issue_asset_operation,
            (amount)
            )
