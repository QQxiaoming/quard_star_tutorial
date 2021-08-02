/*
 * rings.c - netlink implementation of ring commands
 *
 * Implementation of "ethtool -g <dev>" and "ethtool -G <dev> ..."
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "parser.h"

/* RINGS_GET */

int rings_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_RINGS_MAX + 1] = {};
	DECLARE_ATTR_TB_INFO(tb);
	struct nl_context *nlctx = data;
	bool silent;
	int err_ret;
	int ret;

	silent = nlctx->is_dump || nlctx->is_monitor;
	err_ret = silent ? MNL_CB_OK : MNL_CB_ERROR;
	ret = mnl_attr_parse(nlhdr, GENL_HDRLEN, attr_cb, &tb_info);
	if (ret < 0)
		return err_ret;
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_RINGS_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	if (silent)
		putchar('\n');
	printf("Ring parameters for %s:\n", nlctx->devname);
	printf("Pre-set maximums:\n");
	show_u32(tb[ETHTOOL_A_RINGS_RX_MAX], "RX:\t\t");
	show_u32(tb[ETHTOOL_A_RINGS_RX_MINI_MAX], "RX Mini:\t");
	show_u32(tb[ETHTOOL_A_RINGS_RX_JUMBO_MAX], "RX Jumbo:\t");
	show_u32(tb[ETHTOOL_A_RINGS_TX_MAX], "TX:\t\t");
	printf("Current hardware settings:\n");
	show_u32(tb[ETHTOOL_A_RINGS_RX], "RX:\t\t");
	show_u32(tb[ETHTOOL_A_RINGS_RX_MINI], "RX Mini:\t");
	show_u32(tb[ETHTOOL_A_RINGS_RX_JUMBO], "RX Jumbo:\t");
	show_u32(tb[ETHTOOL_A_RINGS_TX], "TX:\t\t");

	return MNL_CB_OK;
}

int nl_gring(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_RINGS_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_RINGS_GET,
				      ETHTOOL_A_RINGS_HEADER, 0);
	if (ret < 0)
		return ret;
	return nlsock_send_get_request(nlsk, rings_reply_cb);
}

/* RINGS_SET */

static const struct param_parser sring_params[] = {
	{
		.arg		= "rx",
		.type		= ETHTOOL_A_RINGS_RX,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-mini",
		.type		= ETHTOOL_A_RINGS_RX_MINI,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-jumbo",
		.type		= ETHTOOL_A_RINGS_RX_JUMBO,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx",
		.type		= ETHTOOL_A_RINGS_TX,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{}
};

int nl_sring(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_msg_buff *msgbuff;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_RINGS_SET, false))
		return -EOPNOTSUPP;

	nlctx->cmd = "-G";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_RINGS_SET,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret < 0)
		return 2;
	if (ethnla_fill_header(msgbuff, ETHTOOL_A_RINGS_HEADER,
			       ctx->devname, 0))
		return -EMSGSIZE;

	ret = nl_parser(nlctx, sring_params, NULL, PARSER_GROUP_NONE, NULL);
	if (ret < 0)
		return 1;

	ret = nlsock_sendmsg(nlsk, NULL);
	if (ret < 0)
		return 81;
	ret = nlsock_process_reply(nlsk, nomsg_reply_cb, nlctx);
	if (ret == 0)
		return 0;
	else
		return nlctx->exit_code ?: 81;
}
