// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>

struct virtual_eth_port {
	struct net_device *netdev;
	struct virtual_eth_port *partner_dev;
	int	id;
	struct work_struct work;
    spinlock_t lock;
	struct sk_buff *tx_skb;
	uint64_t tx_packets;
	uint64_t tx_bytes;
	uint64_t tx_errors;
	uint64_t rx_packets;
	uint64_t rx_bytes;
	uint64_t rx_errors;
};

static struct virtual_eth_port *vport[2];

static netdev_tx_t send_packet(struct sk_buff *skb, struct net_device *dev)
{
	struct virtual_eth_port *vport = netdev_priv(dev);
	unsigned long flags;

	netif_stop_queue(dev);

	spin_lock_irqsave(&vport->lock, flags);
	vport->tx_skb = skb;
	spin_unlock_irqrestore(&vport->lock, flags);

	schedule_work(&vport->work);

	return NETDEV_TX_OK;
}

static void get_stats64(struct net_device *netdev,
			    		struct rtnl_link_stats64 *stats)
{
	struct virtual_eth_port *vport = netdev_priv(netdev);

	stats->tx_packets = vport->tx_packets;
	stats->tx_bytes = vport->tx_bytes;
	stats->tx_errors = vport->tx_errors;
	stats->rx_packets = vport->rx_packets;
	stats->rx_bytes = vport->rx_bytes;
	stats->rx_errors = vport->rx_errors;
}

static void transmission_data(struct work_struct *work)
{
    struct virtual_eth_port *self_vport = container_of(work, struct virtual_eth_port, work);
	struct virtual_eth_port *target_vport = self_vport->partner_dev;
	struct sk_buff *tx_skb;
	struct sk_buff *rx_skb;
	unsigned long flags;
	unsigned int len;

	spin_lock_irqsave(&self_vport->lock, flags);
	tx_skb = self_vport->tx_skb;
	spin_unlock_irqrestore(&self_vport->lock, flags);

	if (tx_skb) {
		len = tx_skb->len;
		rx_skb = netdev_alloc_skb(target_vport->netdev,len);
		skb_put_data(rx_skb, tx_skb->data,len);
		rx_skb->protocol = eth_type_trans(rx_skb, target_vport->netdev);
		netif_rx(rx_skb);

		spin_lock_irqsave(&self_vport->lock, flags);
		self_vport->tx_skb = NULL;
		self_vport->tx_packets++;
		self_vport->tx_bytes+=len;
		spin_unlock_irqrestore(&self_vport->lock, flags);

		spin_lock_irqsave(&target_vport->lock, flags);
		target_vport->rx_packets++;
		target_vport->rx_bytes+=len;
		spin_unlock_irqrestore(&target_vport->lock, flags);

		dev_kfree_skb(tx_skb);
		netif_wake_queue(self_vport->netdev);
	}
}

static const struct net_device_ops netdev_ops = {
	.ndo_start_xmit		= send_packet,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_get_stats64	= get_stats64,
};

static struct virtual_eth_port *alloc_and_init_device(unsigned int id)
{
	struct virtual_eth_port *vport;
	struct net_device *netdev;

	netdev = alloc_netdev_mqs(sizeof(struct virtual_eth_port), "veth%d", NET_NAME_ENUM,
				ether_setup, 1, 1);
	if (!netdev) {
		return NULL;
	}

	vport = netdev_priv(netdev);
	vport->netdev = netdev;
	vport->id = id;
	vport->tx_packets = 0;
	vport->tx_bytes = 0;
	vport->tx_errors = 0;
	vport->rx_packets = 0;
	vport->rx_bytes = 0;
	vport->rx_errors = 0;

    spin_lock_init(&vport->lock);
   	INIT_WORK(&vport->work, transmission_data);

	eth_hw_addr_random(netdev);
	
	netdev->if_port = IF_PORT_10BASET;
	netdev->netdev_ops = &netdev_ops;
	netdev->watchdog_timeo = HZ;
	netdev->min_mtu = 1500;
	netdev->max_mtu = 1500;
	netdev->mtu	= 1500;

	if (register_netdev(netdev)) {
		free_netdev(netdev);
		return NULL;
	}

	netif_start_queue(netdev);
	return vport;
}

static void destroy_and_deinit_port(struct virtual_eth_port *vport)
{
	unregister_netdev(vport->netdev);
	free_netdev(vport->netdev);
}

static void link_dev_port(struct virtual_eth_port *vport0,struct virtual_eth_port *vport1)
{
	vport0->partner_dev = vport1;
	vport1->partner_dev = vport0;
}

static int __init veth_init(void)
{
	pr_info("virt ethernet port driver initialized\n");
	vport[0] = alloc_and_init_device(0);
	if(vport[0] == NULL) {
		return -1;
	}
	vport[1] = alloc_and_init_device(1);
	if(vport[0] == NULL) {
    	destroy_and_deinit_port(vport[0]);
		return -1;
	}
	link_dev_port(vport[0],vport[1]);
	return 0;
}

static void __exit veth_exit(void)
{
    destroy_and_deinit_port(vport[0]);
    destroy_and_deinit_port(vport[1]);
}

module_init(veth_init);
module_exit(veth_exit);

MODULE_AUTHOR("qiaoqm@aliyun.com");
MODULE_DESCRIPTION("virt ethernet port driver");
MODULE_LICENSE("GPL v2");
