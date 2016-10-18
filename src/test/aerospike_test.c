/*
 * Copyright 2008-2016 Aerospike, Inc.
 *
 * Portions may be licensed to Aerospike, Inc. under one or more contributor
 * license agreements.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
#include <errno.h>
#include <getopt.h>

#include <aerospike/aerospike.h>
#include <aerospike/as_event.h>

#include "test.h"
#include "aerospike_test.h"

/******************************************************************************
 * MACROS
 *****************************************************************************/

#define TIMEOUT 1000
#define SCRIPT_LEN_MAX 1048576

/******************************************************************************
 * VARIABLES
 *****************************************************************************/

aerospike * as = NULL;
int g_argc = 0;
char ** g_argv = NULL;
char g_host[MAX_HOST_SIZE];
int g_port = 3000;
static char g_user[AS_USER_SIZE];
static char g_password[AS_PASSWORD_HASH_SIZE];
as_config_tls g_tls = {0};

#if defined(AS_USE_LIBEV) || defined(AS_USE_LIBUV)
static bool g_use_async = true;
#else
static bool g_use_async = false;
#endif

/******************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

static bool
as_client_log_callback(as_log_level level, const char * func, const char * file, uint32_t line, const char * fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	atf_logv(stderr, as_log_level_tostring(level), ATF_LOG_PREFIX, NULL, 0, fmt, ap);
	va_end(ap);
	return true;
}

static void
usage()
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  -h, --host <host1>[:<tlsname1>][:<port1>],...  Default: 127.0.0.1\n");
	fprintf(stderr, "  Server seed hostnames or IP addresses.\n");
	fprintf(stderr, "  The tlsname is only used when connecting with a secure TLS enabled server.\n");
	fprintf(stderr, "  If the port is not specified, the default port is used. Examples:\n\n");
	fprintf(stderr, "  host1\n");
	fprintf(stderr, "  host1:3000,host2:3000\n");
	fprintf(stderr, "  192.168.1.10:cert1:3000,192.168.1.20:cert2:3000\n\n");

	fprintf(stderr, "  -p, --port <port>\n");
	fprintf(stderr, "  The default server port. Default: 3000.\n\n");

	fprintf(stderr, "  -U, --user <user>\n");
	fprintf(stderr, "  The user to connect as. Default: no user.\n\n");

	fprintf(stderr, "  -P[<password>], --password\n");
	fprintf(stderr, "  The user's password. If empty, a prompt is shown. Default: no password.\n\n");

	fprintf(stderr, "  -S, --suite <suite>\n");
	fprintf(stderr, "  The suite to be run. Default: all suites.\n\n");

	fprintf(stderr, "  -T, --testcase <testcase>\n");
	fprintf(stderr, "  The test case to run. Default: all test cases.\n\n");

	fprintf(stderr, "  --tlsEnable         # Default: TLS disabled\n");
	fprintf(stderr, "  Enable TLS.\n\n");

	fprintf(stderr, "  --tlsEncryptOnly\n");
	fprintf(stderr, "  Disable TLS certificate verification.\n\n");

	fprintf(stderr, "  --tlsCaFile <path>\n");
	fprintf(stderr, "  Set the TLS certificate authority file.\n\n");

	fprintf(stderr, "  --tlsCaPath <path>\n");
	fprintf(stderr, "  Set the TLS certificate authority directory.\n\n");

	fprintf(stderr, "  --tlsProtocols <protocols>\n");
	fprintf(stderr, "  Set the TLS protocol selection criteria.\n\n");

	fprintf(stderr, "  --tlsCipherSuite <suite>\n");
	fprintf(stderr, "  Set the TLS cipher selection criteria.\n\n");

	fprintf(stderr, "  --tlsCrlCheck\n");
	fprintf(stderr, "  Enable CRL checking for leaf certs.\n\n");

	fprintf(stderr, "  --tlsCrlCheckAll\n");
	fprintf(stderr, "  Enable CRL checking for all certs.\n\n");

	fprintf(stderr, "  --tlsCertBlackList <path>\n");
	fprintf(stderr, "  Path to a certificate blacklist file.\n\n");

	fprintf(stderr, "  --tlsLogSessionInfo\n");
	fprintf(stderr, "  Log TLS connected session info.\n\n");

	fprintf(stderr, "  --tlsKeyFile <path>\n");
	fprintf(stderr, "  Set the TLS client key file for mutual authentication.\n\n");

	fprintf(stderr, "  --tlsChainFile <path>\n");
	fprintf(stderr, "  Set the TLS client chain file for mutual authentication.\n\n");

	fprintf(stderr, "  -u --usage         # Default: usage not printed.\n");
	fprintf(stderr, "  Display program usage.\n\n");
}

static const char* short_options = "h:p:U:P::S:T:u";

static struct option long_options[] = {
	{"hosts",                required_argument, 0, 'h'},
	{"port",                 required_argument, 0, 'p'},
	{"user",                 required_argument, 0, 'U'},
	{"password",             optional_argument, 0, 'P'},
	{"suite",                required_argument, 0, 'S'},
	{"test",                 required_argument, 0, 'T'},
	{"tlsEnable",            no_argument,       0, 'A'},
	{"tlsEncryptOnly",       no_argument,       0, 'B'},
	{"tlsCaFile",            required_argument, 0, 'E'},
	{"tlsCaPath",            required_argument, 0, 'F'},
	{"tlsProtocols",         required_argument, 0, 'G'},
	{"tlsCipherSuite",       required_argument, 0, 'H'},
	{"tlsCrlCheck",          no_argument,       0, 'I'},
	{"tlsCrlCheckAll",       no_argument,       0, 'J'},
	{"tlsCertBlackList",     required_argument, 0, 'O'},
	{"tlsLogSessionInfo",    no_argument,       0, 'Q'},
	{"tlsKeyFile",           required_argument, 0, 'Z'},
	{"tlsChainFile",         required_argument, 0, 'y'},
	{"usage",     	         no_argument,       0, 'u'},
	{0, 0, 0, 0}
};

static bool parse_opts(int argc, char* argv[])
{
	int option_index = 0;
	int c;

	strcpy(g_host, "127.0.0.1");

	while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
		switch (c) {
		case 'h':
			if (strlen(optarg) >= sizeof(g_host)) {
				error("ERROR: host exceeds max length");
				return false;
			}
			strcpy(g_host, optarg);
			error("host:           %s", g_host);
			break;

		case 'p':
			g_port = atoi(optarg);
			break;

		case 'U':
			if (strlen(optarg) >= sizeof(g_user)) {
				error("ERROR: user exceeds max length");
				return false;
			}
			strcpy(g_user, optarg);
			error("user:           %s", g_user);
			break;

		case 'P':
			as_password_prompt_hash(optarg, g_password);
			break;
				
		case 'S':
			// Exclude all but the specified suite from the plan.
			atf_suite_filter(optarg);
			break;

		case 'T':
			// Exclude all but the specified test.
			atf_test_filter(optarg);
			break;
				
		case 'A':
			g_tls.enable = true;
			break;

		case 'B':
			g_tls.encrypt_only = true;
			break;

		case 'E':
			g_tls.cafile = strdup(optarg);
			break;

		case 'F':
			g_tls.capath = strdup(optarg);
			break;

		case 'G':
			g_tls.protocols = strdup(optarg);
			break;

		case 'H':
			g_tls.cipher_suite = strdup(optarg);
			break;

		case 'I':
			g_tls.crl_check = true;
			break;

		case 'J':
			g_tls.crl_check_all = true;
			break;

		case 'O':
			g_tls.cert_blacklist = strdup(optarg);
			break;

		case 'Q':
			g_tls.log_session_info = true;
			break;

		case 'Z':
			g_tls.keyfile = strdup(optarg);
			break;

		case 'y':
			g_tls.chainfile = strdup(optarg);
			break;

		case 'u':
			usage();
			return false;
				
		default:
	        error("unrecognized options");
	        usage();
			return false;
		}
	}

	return true;
}

static bool before(atf_plan * plan) {

    if ( as ) {
        error("aerospike was already initialized");
        return false;
    }

	// Initialize logging.
	as_log_set_level(AS_LOG_LEVEL_INFO);
	as_log_set_callback(as_client_log_callback);
	
	if (g_use_async) {
		if (as_event_create_loops(1) == 0) {
			error("failed to create event loops");
			return false;
		}
	}
	
	// Initialize global lua configuration.
	as_config_lua lua;
	as_config_lua_init(&lua);
	strcpy(lua.system_path, "modules/lua-core/src");
	strcpy(lua.user_path, "src/test/lua");
	aerospike_init_lua(&lua);

	// Initialize cluster configuration.
	as_config config;
	as_config_init(&config);

	if (! as_config_add_hosts(&config, g_host, g_port)) {
		error("Invalid host(s) %s", g_host);
		return false;
	}

	as_config_set_user(&config, g_user, g_password);

	// Transfer ownership of all heap allocated TLS fields via shallow copy.
	memcpy(&config.tls, &g_tls, sizeof(as_config_tls));

	as_error err;
	as_error_reset(&err);

	as = aerospike_new(&config);

	if ( aerospike_connect(as, &err) == AEROSPIKE_OK ) {
		debug("connected to %s %d", g_host, g_port);
    	return true;
	}
	else {
		error("%s @ %s[%s:%d]", err.message, err.func, err.file, err.line);
		return false;
	}
}

static bool after(atf_plan * plan) {

    if ( ! as ) {
        error("aerospike was not initialized");
        return false;
    }
	
	as_error err;
	as_error_reset(&err);
	
	as_status status = aerospike_close(as, &err);
	aerospike_destroy(as);

	if (g_use_async) {
		as_event_close_loops();
	}

	if (status == AEROSPIKE_OK) {
		debug("disconnected from %s %d", g_host, g_port);
		return true;
	}
	else {
		error("%s @ %s[%s:%d]", err.message, err.func, err.file, err.line);
		return false;
	}
}

/******************************************************************************
 * TEST PLAN
 *****************************************************************************/

PLAN(aerospike_test) {

	// This needs to be done before we add the tests.
    if (! parse_opts(g_argc, g_argv)) {
    	return;
    }
		
	plan_before(before);
	plan_after(after);

	// aerospike_key module
	plan_add(key_basics);
	plan_add(key_apply);
	plan_add(key_apply2);
	plan_add(key_operate);

	// cdt
	plan_add(list_basics);
	plan_add(map_basics);
	plan_add(map_udf);
	plan_add(map_index);

	// aerospike_info module
	plan_add(info_basics);

	// aerospike_info module
	plan_add(udf_basics);
	plan_add(udf_types);
	plan_add(udf_record);

	// aerospike_sindex module
	plan_add(index_basics);

	// aerospike_query module
	plan_add(query_foreach);
	plan_add(query_background);
    plan_add(query_geospatial);

	// aerospike_scan module
	plan_add(scan_basics);

	// aerospike_scan module
	plan_add(batch_get);

	// as_policy module
	plan_add(policy_read);
	plan_add(policy_scan);

	// as_ldt module
	plan_add(ldt_lmap);

	if (g_use_async) {
		plan_add(key_basics_async);
		plan_add(list_basics_async);
		plan_add(map_basics_async);
		plan_add(key_apply_async);
		plan_add(key_pipeline);
		plan_add(batch_async);
		plan_add(scan_async);
		plan_add(query_async);
	}
}
