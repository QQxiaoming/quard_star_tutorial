/* Copyright 2013-2018 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <sensor.h>
#include <skiboot.h>
#include <device.h>
#include <opal.h>
#include <dts.h>
#include <lock.h>
#include <occ.h>

struct dt_node *sensor_node;

static struct lock async_read_list_lock = LOCK_UNLOCKED;
static LIST_HEAD(async_read_list);

struct sensor_async_read {
	struct list_node link;
	u64 *sensor_data64;
	u32 *sensor_data32;
	int token;
};

static int add_to_async_read_list(int token, u32 *data32, u64 *data64)
{
	struct sensor_async_read *req;

	req = zalloc(sizeof(*req));
	if (!req)
		return OPAL_NO_MEM;

	req->token = token;
	req->sensor_data64 = data64;
	req->sensor_data32 = data32;

	lock(&async_read_list_lock);
	list_add_tail(&async_read_list, &req->link);
	unlock(&async_read_list_lock);

	return OPAL_ASYNC_COMPLETION;
}

void check_sensor_read(int token)
{
	struct sensor_async_read *req = NULL;

	lock(&async_read_list_lock);
	if (list_empty(&async_read_list))
		goto out;

	list_for_each(&async_read_list, req, link) {
		if (req->token == token)
			break;
	}
	if (!req)
		goto out;

	*req->sensor_data32 = *req->sensor_data64;
	free(req->sensor_data64);
	list_del(&req->link);
	free(req);
out:
	unlock(&async_read_list_lock);
}

static s64 opal_sensor_read_u64(u32 sensor_hndl, int token, u64 *sensor_data)
{
	switch (sensor_get_family(sensor_hndl)) {
	case SENSOR_DTS:
		return dts_sensor_read(sensor_hndl, token, sensor_data);
	case SENSOR_OCC:
		return occ_sensor_read(sensor_hndl, sensor_data);
	default:
		break;
	}

	if (platform.sensor_read)
		return platform.sensor_read(sensor_hndl, token, sensor_data);

	return OPAL_UNSUPPORTED;
}

static int64_t opal_sensor_read(uint32_t sensor_hndl, int token,
				uint32_t *sensor_data)
{
	u64 *val;
	s64 ret;

	val = zalloc(sizeof(*val));
	if (!val)
		return OPAL_NO_MEM;

	ret = opal_sensor_read_u64(sensor_hndl, token, val);
	if (!ret) {
		*sensor_data = *val;
		free(val);
	} else if (ret == OPAL_ASYNC_COMPLETION) {
		ret = add_to_async_read_list(token, sensor_data, val);
	}

	return ret;
}

static int opal_sensor_group_clear(u32 group_hndl, int token)
{
	switch (sensor_get_family(group_hndl)) {
	case SENSOR_OCC:
		return occ_sensor_group_clear(group_hndl, token);
	default:
		break;
	}

	return OPAL_UNSUPPORTED;
}

static int opal_sensor_group_enable(u32 group_hndl, int token, bool enable)
{
	switch (sensor_get_family(group_hndl)) {
	case SENSOR_OCC:
		return occ_sensor_group_enable(group_hndl, token, enable);
	default:
		break;
	}

	return OPAL_UNSUPPORTED;
}
void sensor_init(void)
{
	sensor_node = dt_new(opal_node, "sensors");

	dt_add_property_string(sensor_node, "compatible", "ibm,opal-sensor");
	dt_add_property_cells(sensor_node, "#address-cells", 1);
	dt_add_property_cells(sensor_node, "#size-cells", 0);

	/* Register OPAL interface */
	opal_register(OPAL_SENSOR_READ, opal_sensor_read, 3);
	opal_register(OPAL_SENSOR_GROUP_CLEAR, opal_sensor_group_clear, 2);
	opal_register(OPAL_SENSOR_READ_U64, opal_sensor_read_u64, 3);
	opal_register(OPAL_SENSOR_GROUP_ENABLE, opal_sensor_group_enable, 3);
}
