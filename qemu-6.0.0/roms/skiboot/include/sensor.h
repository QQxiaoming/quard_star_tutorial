/* Copyright 2013-2015 IBM Corp.
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

#ifndef __SENSOR_H
#define __SENSOR_H

/*
 * A sensor handler is a four bytes value which identifies a sensor by
 * its resource class (temperature, fans ...), a resource identifier
 * and an attribute number (data, status, ...) :
 *
 *                    Res.
 *     | Attr.  |Fam Class|   Resource Id  |
 *     |--------|---|-----|----------------|
 *
 * The last 3bits of the resource class are used to hold the family
 * number. That leaves 32 differents resource classes. This is enough
 * for the FSP as it uses 15.
 */

/*
 * Helper routines to build or use the sensor handler.
 */
#define sensor_make_handler(family, class, rid, attr)			\
	(((attr) << 24) | ((family) & 0x7) << 21 | ((class) & 0x1f) << 16 | \
	 ((rid) & 0xffff))

#define sensor_get_family(handler)	(((handler) >> 21) & 0x7)
#define sensor_get_frc(handler)		(((handler) >> 16) & 0x1f)
#define sensor_get_rid(handler)		((handler) & 0xffff)
#define sensor_get_attr(handler)	((handler) >> 24)

/*
 * Sensor families
 *
 * This identifier is used to dispatch calls to OPAL_SENSOR_READ to
 * the appropriate component. FSP is the initial family and you can
 * have up to eight, as we are hijacking the last 3bits of the
 * resource class.
 */
enum {
	SENSOR_FSP = 0,
	SENSOR_OCC = 6,
	SENSOR_DTS = 7,
};

/*
 * root node of all sensors : /ibm,opal/sensors
 */
extern struct dt_node *sensor_node;

extern void sensor_init(void);
extern void check_sensor_read(int token);

#endif /* __SENSOR_H */
