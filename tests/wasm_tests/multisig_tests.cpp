#include <boost/test/unit_test.hpp>
#include <aacio/testing/tester.hpp>
#include <aacio/chain/contracts/abi_serializer.hpp>
#include <aacio/chain/wast_to_wasm.hpp>
#include <aacio/chain_plugin/chain_plugin.hpp>

#include <aacio.msig/aacio.msig.wast.hpp>
#include <aacio.msig/aacio.msig.abi.hpp>

#include <exchange/exchange.wast.hpp>
#include <exchange/exchange.abi.hpp>

#include <Runtime/Runtime.h>

#include <fc/variant_object.hpp>

using namespace aacio::testing;
using namespace aacio;
using namespace aacio::chain;
using namespace aacio::chain::contracts;
using namespace aacio::chain_apis;
using namespace aacio::testing;
using namespace fc;

using mvo = fc::mutable_variant_object;

class aacio_msig_tester : public tester {
public:

   aacio_msig_tester() {
      create_accounts( { N(aacio.msig), N(alice), N(bob), N(carol) } );
      produce_block();

      auto trace = base_tester::push_action(config::system_account_name, N(setpriv),
                                            config::system_account_name,  mutable_variant_object()
                                            ("account", "aacio.msig")
                                            ("is_priv", 1)
      );

      set_code( N(aacio.msig), aacio_msig_wast );
      set_abi( N(aacio.msig), aacio_msig_abi );

      produce_blocks();
      const auto& accnt = control->get_database().get<account_object,by_name>( N(aacio.msig) );
      abi_def abi;
      BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
      abi_ser.set_abi(abi);
   }

   action_result push_action( const account_name& signer, const action_name &name, const variant_object &data, bool auth = true ) {
         string action_type_name = abi_ser.get_action_type(name);

         action act;
         act.account = N(aacio.msig);
         act.name = name;
         act.data = abi_ser.variant_to_binary( action_type_name, data );
         //std::cout << "test:\n" << fc::to_hex(act.data.data(), act.data.size()) << " size = " << act.data.size() << std::endl;

         return base_tester::push_action( std::move(act), auth ? uint64_t(signer) : 0 );
   }

   transaction reqauth( account_name from, const vector<permission_level>& auths );

   abi_serializer abi_ser;
};

transaction aacio_msig_tester::reqauth( account_name from, const vector<permission_level>& auths ) {
   fc::variants v;
   for ( auto& level : auths ) {
      v.push_back(fc::mutable_variant_object()
                  ("actor", level.actor)
                  ("permission", level.permission)
      );
   }
   variant pretty_trx = fc::mutable_variant_object()
      ("expiration", "2020-01-01T00:30")
      ("region", 0)
      ("ref_block_num", 2)
      ("ref_block_prefix", 3)
      ("max_net_usage_words", 0)
      ("max_kcpu_usage", 0)
      ("delay_sec", 0)
      ("actions", fc::variants({
            fc::mutable_variant_object()
               ("account", name(config::system_account_name))
               ("name", "reqauth")
               ("authorization", v)
               ("data", fc::mutable_variant_object() ("from", from) )
               })
      );
   transaction trx;
   contracts::abi_serializer::from_variant(pretty_trx, trx, get_resolver());
   return trx;
}

BOOST_AUTO_TEST_SUITE(aacio_msig_tests)

BOOST_FIXTURE_TEST_CASE( propose_approve_execute, aacio_msig_tester ) try {
   auto trx = reqauth("alice", {permission_level{N(alice), config::active_name}} );

   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(propose), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("trx",           trx)
                                                ("requested", vector<permission_level>{{ N(alice), config::active_name }})
                        ));

   //fail to execute before approval
   BOOST_REQUIRE_EQUAL( error("transaction declares authority '{\"actor\":\"alice\",\"permission\":\"active\"}', but does not have signatures for it."),
                        push_action( N(alice), N(exec), mvo()
                                     ("proposer",      "alice")
                                     ("proposal_name", "first")
                                     ("executer",      "alice")
                        ));

   //approve and execute
   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(approve), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(alice), config::active_name })
                        ));

   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(exec), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("executer",      "alice")
                        ));

   auto traces = control->push_deferred_transactions( true );
   BOOST_CHECK_EQUAL( 1, traces.size() );
   BOOST_CHECK_EQUAL( 1, traces.at(0).action_traces.size() );
   BOOST_CHECK_EQUAL( transaction_receipt::executed, traces.at(0).status );
} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( propose_approve_unapprove, aacio_msig_tester ) try {
   auto trx = reqauth("alice", {permission_level{N(alice), config::active_name}} );

   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(propose), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("trx",           trx)
                                                ("requested", vector<permission_level>{{ N(alice), config::active_name }})
                        ));

   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(approve), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(alice), config::active_name })
                        ));

   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(unapprove), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(alice), config::active_name })
                        ));

   BOOST_REQUIRE_EQUAL( error("transaction declares authority '{\"actor\":\"alice\",\"permission\":\"active\"}', but does not have signatures for it."),
                        push_action( N(alice), N(exec), mvo()
                                     ("proposer",      "alice")
                                     ("proposal_name", "first")
                                     ("executer",      "alice")
                        ));
} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( propose_approve_by_two, aacio_msig_tester ) try {
   auto trx = reqauth("alice", vector<permission_level>{ { N(alice), config::active_name }, { N(bob), config::active_name } } );
   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(propose), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("trx",           trx)
                                                ("requested", vector<permission_level>{ { N(alice), config::active_name }, { N(bob), config::active_name } })
                        ));

   //approve by alice
   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(approve), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(alice), config::active_name })
                        ));

   //fail because approval by bob is missing
   BOOST_REQUIRE_EQUAL( error("transaction declares authority '{\"actor\":\"bob\",\"permission\":\"active\"}', but does not have signatures for it."),
                        push_action( N(alice), N(exec), mvo()
                                     ("proposer",      "alice")
                                     ("proposal_name", "first")
                                     ("executer",      "alice")
                        ));

   //approve by bob and execute
   BOOST_REQUIRE_EQUAL( success(), push_action( N(bob), N(approve), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(bob), config::active_name })
                        ));
   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(exec), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("executer",      "alice")
                        ));
   auto traces = control->push_deferred_transactions( true );
   BOOST_CHECK_EQUAL( 1, traces.size() );
   BOOST_CHECK_EQUAL( 1, traces.at(0).action_traces.size() );
   BOOST_CHECK_EQUAL( transaction_receipt::executed, traces.at(0).status );
} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( propose_with_wrong_requested_auth, aacio_msig_tester ) try {
   auto trx = reqauth("alice", vector<permission_level>{ { N(alice), config::active_name },  { N(bob), config::active_name } } );
   //try with not enough requested auth
   BOOST_REQUIRE_EQUAL( error("transaction declares authority '{\"actor\":\"bob\",\"permission\":\"active\"}', but does not have signatures for it."),
                        push_action( N(alice), N(propose), mvo()
                                     ("proposer",      "alice")
                                     ("proposal_name", "third")
                                     ("trx",           trx)
                                     ("requested", vector<permission_level>{ { N(alice), config::active_name } } )
                        ));
} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( big_transaction, aacio_msig_tester ) try {
   vector<permission_level> perm = { { N(alice), config::active_name }, { N(bob), config::active_name } };
   auto wasm = wast_to_wasm( exchange_wast );

   variant pretty_trx = fc::mutable_variant_object()
      ("expiration", "2020-01-01T00:30")
      ("region", 0)
      ("ref_block_num", 2)
      ("ref_block_prefix", 3)
      ("max_net_usage_words", 0)
      ("max_kcpu_usage", 0)
      ("delay_sec", 0)
      ("actions", fc::variants({
            fc::mutable_variant_object()
               ("account", name(config::system_account_name))
               ("name", "setcode")
               ("authorization", perm)
               ("data", fc::mutable_variant_object()
                ("account", "alice")
                ("vmtype", 0)
                ("vmversion", 0)
                ("code", bytes( wasm.begin(), wasm.end() ))
               )
               })
      );

   transaction trx;
   contracts::abi_serializer::from_variant(pretty_trx, trx, get_resolver());

   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(propose), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("trx",           trx)
                                                ("requested", perm)
                        ));

   //approve by alice
   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(approve), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(alice), config::active_name })
                        ));
   //approve by bob and execute
   BOOST_REQUIRE_EQUAL( success(), push_action( N(bob), N(approve), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(bob), config::active_name })
                        ));
   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(exec), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("executer",      "alice")
                        ));
   auto traces = control->push_deferred_transactions( true );
   BOOST_CHECK_EQUAL( 1, traces.size() );
   BOOST_CHECK_EQUAL( 1, traces.at(0).action_traces.size() );
   BOOST_CHECK_EQUAL( transaction_receipt::executed, traces.at(0).status );
} FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
