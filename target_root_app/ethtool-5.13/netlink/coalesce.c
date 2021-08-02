/*
 * coalesce.c - netlink implementation of coalescing commands
 *
 * Implementation of "ethtool -c <dev>" and "ethtool -C <dev> ..."
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "parser.h"

/* COALESCE_GET */

int coalesce_reply_cb(const struct nlmsghdr *nlhdr, void *data)
{
	const struct nlattr *tb[ETHTOOL_A_COALESCE_MAX + 1] = {};
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
	nlctx->devname = get_dev_name(tb[ETHTOOL_A_COALESCE_HEADER]);
	if (!dev_ok(nlctx))
		return err_ret;

	if (silent)
		putchar('\n');
	printf("Coalesce parameters for %s:\n", nlctx->devname);
	show_bool("rx", "Adaptive RX: %s  ",
		  tb[ETHTOOL_A_COALESCE_USE_ADAPTIVE_RX]);
	show_bool("tx", "TX: %s\n", tb[ETHTOOL_A_COALESCE_USE_ADAPTIVE_TX]);
	show_u32(tb[ETHTOOL_A_COALESCE_STATS_BLOCK_USECS],
		 "stats-block-usecs: ");
	show_u32(tb[ETHTOOL_A_COALESCE_RATE_SAMPLE_INTERVAL],
		 "sample-interval: ");
	show_u32(tb[ETHTOOL_A_COALESCE_PKT_RATE_LOW], "pkt-rate-low: ");
	show_u32(tb[ETHTOOL_A_COALESCE_PKT_RATE_HIGH], "pkt-rate-high: ");
	putchar('\n');
	show_u32(tb[ETHTOOL_A_COALESCE_RX_USECS], "rx-usecs: ");
	show_u32(tb[ETHTOOL_A_COALESCE_RX_MAX_FRAMES], "rx-frames: ");
	show_u32(tb[ETHTOOL_A_COALESCE_RX_USECS_IRQ], "rx-usecs-irq: ");
	show_u32(tb[ETHTOOL_A_COALESCE_RX_MAX_FRAMES_IRQ], "rx-frames-irq: ");
	putchar('\n');
	show_u32(tb[ETHTOOL_A_COALESCE_TX_USECS], "tx-usecs: ");
	show_u32(tb[ETHTOOL_A_COALESCE_TX_MAX_FRAMES], "tx-frames: ");
	show_u32(tb[ETHTOOL_A_COALESCE_TX_USECS_IRQ], "tx-usecs-irq: ");
	show_u32(tb[ETHTOOL_A_COALESCE_TX_MAX_FRAMES_IRQ], "tx-frames-irq: ");
	putchar('\n');
	show_u32(tb[ETHTOOL_A_COALESCE_RX_USECS_LOW], "rx-usecs-low: ");
	show_u32(tb[ETHTOOL_A_COALESCE_RX_MAX_FRAMES_LOW], "rx-frame-low: ");
	show_u32(tb[ETHTOOL_A_COALESCE_TX_USECS_LOW], "tx-usecs-low: ");
	show_u32(tb[ETHTOOL_A_COALESCE_TX_MAX_FRAMES_LOW], "tx-frame-low: ");
	putchar('\n');
	show_u32(tb[ETHTOOL_A_COALESCE_RX_USECS_HIGH], "rx-usecs-high: ");
	show_u32(tb[ETHTOOL_A_COALESCE_RX_MAX_FRAMES_HIGH], "rx-frame-high: ");
	show_u32(tb[ETHTOOL_A_COALESCE_TX_USECS_HIGH], "tx-usecs-high: ");
	show_u32(tb[ETHTOOL_A_COALESCE_TX_MAX_FRAMES_HIGH], "tx-frame-high: ");
	putchar('\n');

	return MNL_CB_OK;
}

int nl_gcoalesce(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_COALESCE_GET, true))
		return -EOPNOTSUPP;
	if (ctx->argc > 0) {
		fprintf(stderr, "ethtool: unexpected parameter '%s'\n",
			*ctx->argp);
		return 1;
	}

	ret = nlsock_prep_get_request(nlsk, ETHTOOL_MSG_COALESCE_GET,
				      ETHTOOL_A_COALESCE_HEADER, 0);
	if (ret < 0)
		return ret;
	return nlsock_send_get_request(nlsk, coalesce_reply_cb);
}

/* COALESCE_SET */

static const struct param_parser scoalesce_params[] = {
	{
		.arg		= "adaptive-rx",
		.type		= ETHTOOL_A_COALESCE_USE_ADAPTIVE_RX,
		.handler	= nl_parse_u8bool,
		.min_argc	= 1,
	},
	{
		.arg		= "adaptive-tx",
		.type		= ETHTOOL_A_COALESCE_USE_ADAPTIVE_TX,
		.handler	= nl_parse_u8bool,
		.min_argc	= 1,
	},
	{
		.arg		= "sample-interval",
		.type		= ETHTOOL_A_COALESCE_RATE_SAMPLE_INTERVAL,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "stats-block-usecs",
		.type		= ETHTOOL_A_COALESCE_STATS_BLOCK_USECS,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "pkt-rate-low",
		.type		= ETHTOOL_A_COALESCE_PKT_RATE_LOW,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "pkt-rate-high",
		.type		= ETHTOOL_A_COALESCE_PKT_RATE_HIGH,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-usecs",
		.type		= ETHTOOL_A_COALESCE_RX_USECS,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-frames",
		.type		= ETHTOOL_A_COALESCE_RX_MAX_FRAMES,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-usecs-irq",
		.type		= ETHTOOL_A_COALESCE_RX_USECS_IRQ,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-frames-irq",
		.type		= ETHTOOL_A_COALESCE_RX_MAX_FRAMES_IRQ,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-usecs",
		.type		= ETHTOOL_A_COALESCE_TX_USECS,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-frames",
		.type		= ETHTOOL_A_COALESCE_TX_MAX_FRAMES,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-usecs-irq",
		.type		= ETHTOOL_A_COALESCE_TX_USECS_IRQ,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-frames-irq",
		.type		= ETHTOOL_A_COALESCE_TX_MAX_FRAMES_IRQ,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-usecs-low",
		.type		= ETHTOOL_A_COALESCE_RX_USECS_LOW,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-frames-low",
		.type		= ETHTOOL_A_COALESCE_RX_MAX_FRAMES_LOW,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-usecs-low",
		.type		= ETHTOOL_A_COALESCE_TX_USECS_LOW,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-frames-low",
		.type		= ETHTOOL_A_COALESCE_TX_MAX_FRAMES_LOW,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-usecs-high",
		.type		= ETHTOOL_A_COALESCE_RX_USECS_HIGH,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "rx-frames-high",
		.type		= ETHTOOL_A_COALESCE_RX_MAX_FRAMES_HIGH,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-usecs-high",
		.type		= ETHTOOL_A_COALESCE_TX_USECS_HIGH,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{
		.arg		= "tx-frames-high",
		.type		= ETHTOOL_A_COALESCE_TX_MAX_FRAMES_HIGH,
		.handler	= nl_parse_direct_u32,
		.min_argc	= 1,
	},
	{}
};

int nl_scoalesce(struct cmd_context *ctx)
{
	struct nl_context *nlctx = ctx->nlctx;
	struct nl_msg_buff *msgbuff;
	struct nl_socket *nlsk;
	int ret;

	if (netlink_cmd_check(ctx, ETHTOOL_MSG_COALESCE_SET, false))
		return -EOPNOTSUPP;

	nlctx->cmd = "-C";
	nlctx->argp = ctx->argp;
	nlctx->argc = ctx->argc;
	nlctx->devname = ctx->devname;
	nlsk = nlctx->ethnl_socket;
	msgbuff = &nlsk->msgbuff;

	ret = msg_init(nlctx, msgbuff, ETHTOOL_MSG_COALESCE_SET,
		       NLM_F_REQUEST | NLM_F_ACK);
	if (ret < 0)
		return 2;
	if (ethnla_fill_header(msgbuff, ETHTOOL_A_COALESCE_HEADER,
			       ctx->devname, 0))
		return -EMSGSIZE;

	ret = nl_parser(nlctx, scoalesce_params, NULL, PARSER_GROUP_NONE, NULL);
	if (ret < 0)
		return 1;

	ret = nlsock_sendmsg(nlsk, NULL);
	if (ret < 0)
		return 1;
	ret = nlsock_process_reply(nlsk, nomsg_reply_cb, nlctx);
	if (ret == 0)
		return 0;
	else
		return nlctx->exit_code ?: 1;
}
