// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>

struct virtual_eth_port {
	struct net_device *netdev;
	struct napi_struct	napi;
	struct virtual_eth_port *partner_dev;
	int	id;
	struct work_struct work;
    spinlock_t lock;
	bool closing;
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
	struct virtual_eth_port *priv = netdev_priv(dev);
	unsigned long flags;
	bool closing;

	netif_stop_queue(dev);

	spin_lock_irqsave(&priv->lock, flags);
	closing = priv->closing;
	priv->tx_skb = skb;
	spin_unlock_irqrestore(&priv->lock, flags);

	if (closing)
		dev_kfree_skb(skb);
	else
		schedule_work(&priv->work);

	return NETDEV_TX_OK;
}

static void get_stats64(struct net_device *netdev,
			    		struct rtnl_link_stats64 *stats)
{
	struct virtual_eth_port *priv = netdev_priv(netdev);
	stats->tx_packets = priv->tx_packets;
	stats->tx_bytes = priv->tx_bytes;
	stats->tx_errors = priv->tx_errors;
	stats->rx_packets = priv->rx_packets;
	stats->rx_bytes = priv->rx_bytes;
	stats->rx_errors = priv->rx_errors;
}

//static int napi_poll(struct napi_struct *napi, int budget)
//{
//	pr_info("qqm:E %s\n",__func__);
//	pr_info("qqm:X %s\n",__func__);
//	return 0;
//}

static void transmission_data(struct work_struct *work)
{
    struct virtual_eth_port *self_port = container_of(work, struct virtual_eth_port, work);
	struct virtual_eth_port *target_port = self_port->partner_dev;
	struct sk_buff *tx_skb;
	struct sk_buff *rx_skb;
	unsigned long flags;
	unsigned int len;

	spin_lock_irqsave(&self_port->lock, flags);
	tx_skb = self_port->tx_skb;
	spin_unlock_irqrestore(&self_port->lock, flags);
	if (tx_skb) {
		len = tx_skb->len;
		//print_hex_dump(KERN_ERR, "", DUMP_PREFIX_NONE, 16, 1, (void *)tx_skb->data, len, 0);
		rx_skb = netdev_alloc_skb(target_port->netdev,len);
		skb_put_data(rx_skb, tx_skb->data,len);
		rx_skb->protocol = eth_type_trans(rx_skb, target_port->netdev);
		netif_rx(rx_skb);
		spin_lock_irqsave(&self_port->lock, flags);
		self_port->tx_skb = NULL;
		self_port->tx_packets++;
		self_port->tx_bytes+=len;
		spin_unlock_irqrestore(&self_port->lock, flags);
		spin_lock_irqsave(&target_port->lock, flags);
		target_port->rx_packets++;
		target_port->rx_bytes+=len;
		spin_unlock_irqrestore(&target_port->lock, flags);
		dev_kfree_skb(tx_skb);
		netif_wake_queue(self_port->netdev);
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
	struct virtual_eth_port *dev;
	struct net_device *netdev;

	netdev = alloc_etherdev(sizeof(struct virtual_eth_port));
	if (!netdev) {
		return NULL;
	}
	dev = netdev_priv(netdev);
	dev->netdev = netdev;
	dev->id = id;
	dev->tx_packets = 0;
	dev->tx_bytes = 0;
	dev->tx_errors = 0;
	dev->rx_packets = 0;
	dev->rx_bytes = 0;
	dev->rx_errors = 0;

    spin_lock_init(&dev->lock);
   	INIT_WORK(&dev->work, transmission_data);

	eth_hw_addr_random(netdev);

	netdev->if_port = IF_PORT_10BASET;
	netdev->netdev_ops = &netdev_ops;
	netdev->watchdog_timeo = HZ;
	netdev->min_mtu = 1500;
	netdev->max_mtu = 1500;
	netdev->mtu	= 1500;

	//netif_napi_add(netdev, &dev->napi, napi_poll, 64);
	if (register_netdev(netdev)) {
		free_netdev(netdev);
		return NULL;
	}

	netif_start_queue(netdev);
	return dev;
}

static void destroy_and_deinit_port(struct virtual_eth_port *dev)
{
	unregister_netdev(dev->netdev);
	free_netdev(dev->netdev);
}

static void link_dev_port(struct virtual_eth_port *dev0,struct virtual_eth_port *dev1)
{
	dev0->partner_dev = dev1;
	dev1->partner_dev = dev0;
}

static int __init veth_init(void)
{
	pr_info("virt eth port driver initialized\n");
	vport[0] = alloc_and_init_device(0);
	vport[1] = alloc_and_init_device(1);
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
