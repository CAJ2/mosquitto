#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "mqtt_protocol.h"
#include "property_mosq.h"
#include "packet_mosq.h"

static void byte_prop_write_helper(
		int remaining_length,
		int rc_expected,
		int identifier,
		uint8_t value_expected)
{
	struct mqtt5__property property;
	struct mosquitto__packet packet;
	struct mqtt5__property *properties;
	int rc;

	memset(&property, 0, sizeof(struct mqtt5__property));

	property.identifier = identifier;
	property.value.i8 = value_expected;

	memset(&packet, 0, sizeof(struct mosquitto__packet));
	packet.remaining_length = property__get_length_all(&property)+1;
	packet.packet_length = packet.remaining_length+10;
	packet.payload = calloc(packet.remaining_length+10, 1);

	property__write_all(&packet, &property);
	packet.pos = 0;

	rc = property__read_all(&packet, &properties);

	CU_ASSERT_EQUAL(rc, rc_expected);
	CU_ASSERT_EQUAL(packet.pos, remaining_length);
	if(properties){
		CU_ASSERT_EQUAL(properties->identifier, identifier);
		CU_ASSERT_EQUAL(properties->value.i8, value_expected);
		CU_ASSERT_PTR_EQUAL(properties->next, NULL);
		CU_ASSERT_EQUAL(property__get_length_all(properties), 2);
		property__free_all(&properties);
	}
	CU_ASSERT_PTR_EQUAL(properties, NULL);
}


static void int32_prop_write_helper(
		int remaining_length,
		int rc_expected,
		int identifier,
		uint32_t value_expected)
{
	struct mqtt5__property property;
	struct mosquitto__packet packet;
	struct mqtt5__property *properties;
	int rc;

	memset(&property, 0, sizeof(struct mqtt5__property));

	property.identifier = identifier;
	property.value.i32 = value_expected;

	memset(&packet, 0, sizeof(struct mosquitto__packet));
	packet.remaining_length = property__get_length_all(&property)+1;
	packet.packet_length = packet.remaining_length+10;
	packet.payload = calloc(packet.remaining_length+10, 1);

	property__write_all(&packet, &property);
	packet.pos = 0;

	rc = property__read_all(&packet, &properties);

	CU_ASSERT_EQUAL(rc, rc_expected);
	CU_ASSERT_EQUAL(packet.pos, remaining_length);
	if(properties){
		CU_ASSERT_EQUAL(properties->identifier, identifier);
		CU_ASSERT_EQUAL(properties->value.i32, value_expected);
		CU_ASSERT_PTR_EQUAL(properties->next, NULL);
		CU_ASSERT_EQUAL(property__get_length_all(properties), 5);
		property__free_all(&properties);
	}
	CU_ASSERT_PTR_EQUAL(properties, NULL);
}


static void int16_prop_write_helper(
		int remaining_length,
		int rc_expected,
		int identifier,
		uint16_t value_expected)
{
	struct mqtt5__property property;
	struct mosquitto__packet packet;
	struct mqtt5__property *properties;
	int rc;

	memset(&property, 0, sizeof(struct mqtt5__property));

	property.identifier = identifier;
	property.value.i16 = value_expected;

	memset(&packet, 0, sizeof(struct mosquitto__packet));
	packet.remaining_length = property__get_length_all(&property)+1;
	packet.packet_length = packet.remaining_length+10;
	packet.payload = calloc(packet.remaining_length+10, 1);

	property__write_all(&packet, &property);
	packet.pos = 0;

	rc = property__read_all(&packet, &properties);

	CU_ASSERT_EQUAL(rc, rc_expected);
	CU_ASSERT_EQUAL(packet.pos, remaining_length);
	if(properties){
		CU_ASSERT_EQUAL(properties->identifier, identifier);
		CU_ASSERT_EQUAL(properties->value.i16, value_expected);
		CU_ASSERT_PTR_EQUAL(properties->next, NULL);
		CU_ASSERT_EQUAL(property__get_length_all(properties), 3);
		property__free_all(&properties);
	}
	CU_ASSERT_PTR_EQUAL(properties, NULL);
}

static void string_prop_write_helper(
		int remaining_length,
		int rc_expected,
		int identifier,
		const char *value_expected)
{
	struct mqtt5__property property;
	struct mosquitto__packet packet;
	struct mqtt5__property *properties;
	int rc;

	memset(&property, 0, sizeof(struct mqtt5__property));

	property.identifier = identifier;
	property.value.s.v = strdup(value_expected);
	property.value.s.len = strlen(value_expected);

	memset(&packet, 0, sizeof(struct mosquitto__packet));
	packet.remaining_length = property__get_length_all(&property)+1;
	packet.packet_length = packet.remaining_length+10;
	packet.payload = calloc(packet.remaining_length+10, 1);

	property__write_all(&packet, &property);
	packet.pos = 0;

	rc = property__read_all(&packet, &properties);

	CU_ASSERT_EQUAL(rc, rc_expected);
	CU_ASSERT_EQUAL(packet.pos, remaining_length);
	if(properties){
		CU_ASSERT_EQUAL(properties->identifier, identifier);
		CU_ASSERT_EQUAL(properties->value.s.len, strlen(value_expected));
		CU_ASSERT_STRING_EQUAL(properties->value.s.v, value_expected);
		CU_ASSERT_PTR_EQUAL(properties->next, NULL);
		CU_ASSERT_EQUAL(property__get_length_all(properties), 1+2+strlen(value_expected));
		property__free_all(&properties);
	}
	CU_ASSERT_PTR_EQUAL(properties, NULL);
	free(property.value.s.v);
}


static void binary_prop_write_helper(
		int remaining_length,
		int rc_expected,
		int identifier,
		const uint8_t *value_expected,
		int len_expected)
{
	struct mqtt5__property property;
	struct mosquitto__packet packet;
	struct mqtt5__property *properties;
	int rc;

	memset(&property, 0, sizeof(struct mqtt5__property));

	property.identifier = identifier;
	property.value.bin.v = malloc(len_expected);
	memcpy(property.value.bin.v, value_expected, len_expected);
	property.value.bin.len = len_expected;

	memset(&packet, 0, sizeof(struct mosquitto__packet));
	packet.remaining_length = property__get_length_all(&property)+1;
	packet.packet_length = packet.remaining_length+10;
	packet.payload = calloc(packet.remaining_length+10, 1);

	property__write_all(&packet, &property);
	packet.pos = 0;

	rc = property__read_all(&packet, &properties);

	CU_ASSERT_EQUAL(rc, rc_expected);
	CU_ASSERT_EQUAL(packet.pos, remaining_length);
	if(properties){
		CU_ASSERT_EQUAL(properties->identifier, identifier);
		CU_ASSERT_EQUAL(properties->value.bin.len, len_expected);
		CU_ASSERT_EQUAL(memcmp(properties->value.bin.v, value_expected, len_expected), 0);
		CU_ASSERT_PTR_EQUAL(properties->next, NULL);
		CU_ASSERT_EQUAL(property__get_length_all(properties), 1+2+len_expected);
		property__free_all(&properties);
	}
	CU_ASSERT_PTR_EQUAL(properties, NULL);
	free(property.value.bin.v);
}

static void string_pair_prop_write_helper(
		int remaining_length,
		int rc_expected,
		int identifier,
		const char *name_expected,
		const char *value_expected,
		bool expect_multiple)
{
	struct mqtt5__property property;
	struct mosquitto__packet packet;
	struct mqtt5__property *properties;
	int rc;

	memset(&property, 0, sizeof(struct mqtt5__property));

	property.identifier = identifier;
	property.value.s.v = strdup(value_expected);
	property.value.s.len = strlen(value_expected);
	property.name.v = strdup(name_expected);
	property.name.len = strlen(name_expected);

	memset(&packet, 0, sizeof(struct mosquitto__packet));
	packet.remaining_length = property__get_length_all(&property)+1;
	packet.packet_length = packet.remaining_length+10;
	packet.payload = calloc(packet.remaining_length+10, 1);

	property__write_all(&packet, &property);
	packet.pos = 0;

	rc = property__read_all(&packet, &properties);

	CU_ASSERT_EQUAL(rc, rc_expected);
	CU_ASSERT_EQUAL(packet.pos, remaining_length);
	if(properties){
		CU_ASSERT_EQUAL(properties->identifier, identifier);
		CU_ASSERT_EQUAL(properties->name.len, strlen(name_expected));
		CU_ASSERT_EQUAL(properties->value.s.len, strlen(value_expected));
		CU_ASSERT_STRING_EQUAL(properties->name.v, name_expected);
		CU_ASSERT_STRING_EQUAL(properties->value.s.v, value_expected);
		if(expect_multiple){
			CU_ASSERT_PTR_NOT_NULL(properties->next);
		}else{
			CU_ASSERT_PTR_NULL(properties->next);
			CU_ASSERT_EQUAL(property__get_length_all(properties), 1+2+strlen(name_expected)+2+strlen(value_expected));
		}
		property__free_all(&properties);
	}
	CU_ASSERT_PTR_NULL(properties);
	free(property.value.s.v);
	free(property.name.v);
}

static void varint_prop_write_helper(
		int remaining_length,
		int rc_expected,
		int identifier,
		uint32_t value_expected)
{
	struct mqtt5__property property;
	struct mosquitto__packet packet;
	struct mqtt5__property *properties;
	int rc;

	memset(&property, 0, sizeof(struct mqtt5__property));

	property.identifier = identifier;
	property.value.varint = value_expected;

	memset(&packet, 0, sizeof(struct mosquitto__packet));
	packet.remaining_length = property__get_length_all(&property)+1;
	packet.packet_length = packet.remaining_length+10;
	packet.payload = calloc(packet.remaining_length+10, 1);

	property__write_all(&packet, &property);
	packet.pos = 0;

	rc = property__read_all(&packet, &properties);

	CU_ASSERT_EQUAL(rc, rc_expected);
	if(properties){
		CU_ASSERT_EQUAL(properties->identifier, identifier);
		CU_ASSERT_EQUAL(properties->value.varint, value_expected);
		CU_ASSERT_PTR_NULL(properties->next);
		if(value_expected < 128){
			CU_ASSERT_EQUAL(property__get_length_all(properties), 2);
		}else if(value_expected < 16384){
			CU_ASSERT_EQUAL(property__get_length_all(properties), 3);
		}else if(value_expected < 2097152){
			CU_ASSERT_EQUAL(property__get_length_all(properties), 4);
		}else if(value_expected < 268435456){
			CU_ASSERT_EQUAL(property__get_length_all(properties), 5);
		}else{
			CU_FAIL("Incorrect varint value.");
		}
		property__free_all(&properties);
	}
	CU_ASSERT_PTR_NULL(properties);
}

/* ========================================================================
 * BAD IDENTIFIER
 * ======================================================================== */

static void TEST_bad_identifier(void)
{
	struct mqtt5__property property;
	struct mosquitto__packet packet;
	uint8_t payload[10];
	int rc;

	memset(&property, 0, sizeof(struct mqtt5__property));
	memset(&packet, 0, sizeof(struct mosquitto__packet));
	property.identifier = 0xFFFF;
	packet.packet_length = 10;
	packet.remaining_length = 8;
	packet.payload = payload;
	rc = property__write_all(&packet, &property);
	CU_ASSERT_EQUAL(rc, MOSQ_ERR_INVAL);
}


/* ========================================================================
 * SINGLE PROPERTIES
 * ======================================================================== */

static void TEST_single_payload_format_indicator(void)
{
	byte_prop_write_helper(3, MOSQ_ERR_SUCCESS, PROP_PAYLOAD_FORMAT_INDICATOR, 1);
}

static void TEST_single_request_problem_information(void)
{
	byte_prop_write_helper(3, MOSQ_ERR_SUCCESS, PROP_REQUEST_PROBLEM_INFO, 1);
}

static void TEST_single_request_response_information(void)
{
	byte_prop_write_helper(3, MOSQ_ERR_SUCCESS, PROP_REQUEST_RESPONSE_INFO, 1);
}

static void TEST_single_maximum_qos(void)
{
	byte_prop_write_helper(3, MOSQ_ERR_SUCCESS, PROP_MAXIMUM_QOS, 1);
}

static void TEST_single_retain_available(void)
{
	byte_prop_write_helper(3, MOSQ_ERR_SUCCESS, PROP_RETAIN_AVAILABLE, 1);
}

static void TEST_single_wildcard_subscription_available(void)
{
	byte_prop_write_helper(3, MOSQ_ERR_SUCCESS, PROP_WILDCARD_SUB_AVAILABLE, 0);
}

static void TEST_single_subscription_identifier_available(void)
{
	byte_prop_write_helper(3, MOSQ_ERR_SUCCESS, PROP_SUBSCRIPTION_ID_AVAILABLE, 0);
}

static void TEST_single_shared_subscription_available(void)
{
	byte_prop_write_helper(3, MOSQ_ERR_SUCCESS, PROP_SHARED_SUB_AVAILABLE, 1);
}

static void TEST_single_message_expiry_interval(void)
{
	int32_prop_write_helper(6, MOSQ_ERR_SUCCESS, PROP_MESSAGE_EXPIRY_INTERVAL, 0x12233445);
}

static void TEST_single_session_expiry_interval(void)
{
	int32_prop_write_helper(6, MOSQ_ERR_SUCCESS, PROP_SESSION_EXPIRY_INTERVAL, 0x45342312);
}

static void TEST_single_will_delay_interval(void)
{
	int32_prop_write_helper(6, MOSQ_ERR_SUCCESS, PROP_WILL_DELAY_INTERVAL, 0x45342312);
}

static void TEST_single_maximum_packet_size(void)
{
	int32_prop_write_helper(6, MOSQ_ERR_SUCCESS, PROP_MAXIMUM_PACKET_SIZE, 0x45342312);
}

static void TEST_single_server_keep_alive(void)
{
	int16_prop_write_helper(4, MOSQ_ERR_SUCCESS, PROP_SERVER_KEEP_ALIVE, 0x4534);
}

static void TEST_single_receive_maximum(void)
{
	int16_prop_write_helper(4, MOSQ_ERR_SUCCESS, PROP_RECEIVE_MAXIMUM, 0x6842);
}

static void TEST_single_topic_alias_maximum(void)
{
	int16_prop_write_helper(4, MOSQ_ERR_SUCCESS, PROP_TOPIC_ALIAS_MAXIMUM, 0x6842);
}

static void TEST_single_topic_alias(void)
{
	int16_prop_write_helper(4, MOSQ_ERR_SUCCESS, PROP_TOPIC_ALIAS, 0x6842);
}

static void TEST_single_content_type(void)
{
	string_prop_write_helper(9, MOSQ_ERR_SUCCESS, PROP_CONTENT_TYPE, "hello");
}

static void TEST_single_response_topic(void)
{
	string_prop_write_helper(9, MOSQ_ERR_SUCCESS, PROP_RESPONSE_TOPIC, "hello");
}

static void TEST_single_assigned_client_identifier(void)
{
	string_prop_write_helper(9, MOSQ_ERR_SUCCESS, PROP_ASSIGNED_CLIENT_IDENTIFIER, "hello");
}

static void TEST_single_authentication_method(void)
{
	string_prop_write_helper(9, MOSQ_ERR_SUCCESS, PROP_AUTHENTICATION_METHOD, "hello");
}

static void TEST_single_response_information(void)
{
	string_prop_write_helper(9, MOSQ_ERR_SUCCESS, PROP_RESPONSE_INFO, "hello");
}

static void TEST_single_server_reference(void)
{
	string_prop_write_helper(9, MOSQ_ERR_SUCCESS, PROP_SERVER_REFERENCE, "hello");
}

static void TEST_single_reason_string(void)
{
	string_prop_write_helper(9, MOSQ_ERR_SUCCESS, PROP_REASON_STRING, "hello");
}

static void TEST_single_correlation_data(void)
{
	uint8_t payload[5] = {1, 'e', 0, 'l', 9};

	binary_prop_write_helper(9, MOSQ_ERR_SUCCESS, PROP_CORRELATION_DATA, payload, 5);
}

static void TEST_single_authentication_data(void)
{
	uint8_t payload[5] = {1, 'e', 0, 'l', 9};

	binary_prop_write_helper(9, MOSQ_ERR_SUCCESS, PROP_AUTHENTICATION_DATA, payload, 5);
}

static void TEST_single_user_property(void)
{
	string_pair_prop_write_helper(10, MOSQ_ERR_SUCCESS, PROP_USER_PROPERTY, "za", "bc", false);
}

static void TEST_single_subscription_identifier(void)
{
	varint_prop_write_helper(3, MOSQ_ERR_SUCCESS, PROP_SUBSCRIPTION_IDENTIFIER, 0);
	varint_prop_write_helper(3, MOSQ_ERR_SUCCESS, PROP_SUBSCRIPTION_IDENTIFIER, 127);
	varint_prop_write_helper(4, MOSQ_ERR_SUCCESS, PROP_SUBSCRIPTION_IDENTIFIER, 128);
	varint_prop_write_helper(4, MOSQ_ERR_SUCCESS, PROP_SUBSCRIPTION_IDENTIFIER, 16383);
	varint_prop_write_helper(5, MOSQ_ERR_SUCCESS, PROP_SUBSCRIPTION_IDENTIFIER, 16384);
	varint_prop_write_helper(5, MOSQ_ERR_SUCCESS, PROP_SUBSCRIPTION_IDENTIFIER, 2097151);
	varint_prop_write_helper(6, MOSQ_ERR_SUCCESS, PROP_SUBSCRIPTION_IDENTIFIER, 2097152);
	varint_prop_write_helper(6, MOSQ_ERR_SUCCESS, PROP_SUBSCRIPTION_IDENTIFIER, 268435455);
}


/* ========================================================================
 * TEST SUITE SETUP
 * ======================================================================== */

int init_property_write_tests(void)
{
	CU_pSuite test_suite = NULL;

	test_suite = CU_add_suite("Property write", NULL, NULL);
	if(!test_suite){
		printf("Error adding CUnit Property write test suite.\n");
		return 1;
	}

	if(0
			|| !CU_add_test(test_suite, "Bad identifier", TEST_bad_identifier)
			|| !CU_add_test(test_suite, "Single Payload Format Indicator", TEST_single_payload_format_indicator)
			|| !CU_add_test(test_suite, "Single Request Problem Information", TEST_single_request_problem_information)
			|| !CU_add_test(test_suite, "Single Request Response Information", TEST_single_request_response_information)
			|| !CU_add_test(test_suite, "Single Maximum QoS", TEST_single_maximum_qos)
			|| !CU_add_test(test_suite, "Single Retain Available", TEST_single_retain_available)
			|| !CU_add_test(test_suite, "Single Wildcard Subscription Available", TEST_single_wildcard_subscription_available)
			|| !CU_add_test(test_suite, "Single Subscription Identifier Available", TEST_single_subscription_identifier_available)
			|| !CU_add_test(test_suite, "Single Shared Subscription Available", TEST_single_shared_subscription_available)
			|| !CU_add_test(test_suite, "Single Message Expiry Interval", TEST_single_message_expiry_interval)
			|| !CU_add_test(test_suite, "Single Session Expiry Interval", TEST_single_session_expiry_interval)
			|| !CU_add_test(test_suite, "Single Will Delay Interval", TEST_single_will_delay_interval)
			|| !CU_add_test(test_suite, "Single Maximum Packet Size", TEST_single_maximum_packet_size)
			|| !CU_add_test(test_suite, "Single Server Keep Alive", TEST_single_server_keep_alive)
			|| !CU_add_test(test_suite, "Single Receive Maximum", TEST_single_receive_maximum)
			|| !CU_add_test(test_suite, "Single Topic Alias Maximum", TEST_single_topic_alias_maximum)
			|| !CU_add_test(test_suite, "Single Topic Alias", TEST_single_topic_alias)
			|| !CU_add_test(test_suite, "Single Content Type", TEST_single_content_type)
			|| !CU_add_test(test_suite, "Single Response Topic", TEST_single_response_topic)
			|| !CU_add_test(test_suite, "Single Assigned Client Identifier", TEST_single_assigned_client_identifier)
			|| !CU_add_test(test_suite, "Single Authentication Method", TEST_single_authentication_method)
			|| !CU_add_test(test_suite, "Single Response Information", TEST_single_response_information)
			|| !CU_add_test(test_suite, "Single Server Reference", TEST_single_server_reference)
			|| !CU_add_test(test_suite, "Single Reason String", TEST_single_reason_string)
			|| !CU_add_test(test_suite, "Single Correlation Data", TEST_single_correlation_data)
			|| !CU_add_test(test_suite, "Single Authentication Data", TEST_single_authentication_data)
			|| !CU_add_test(test_suite, "Single User Property", TEST_single_user_property)
			|| !CU_add_test(test_suite, "Single Subscription Identifier", TEST_single_subscription_identifier)
			){

		printf("Error adding Property read CUnit tests.\n");
		return 1;
	}

	return 0;
}