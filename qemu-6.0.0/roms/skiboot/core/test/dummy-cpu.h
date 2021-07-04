/* Copyright 2013-2018 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* A dummy cpu.h for tests.
 * We don't want to include the real skiboot cpu.h, it's PPC-specific
 */

#ifndef __CPU_H
#define __CPU_H

#include <stdint.h>
#include <stdbool.h>

static unsigned int cpu_max_pir = 1;
struct cpu_thread {
	unsigned int			chip_id;
};
struct cpu_job *__cpu_queue_job(struct cpu_thread *cpu,
				const char *name,
				void (*func)(void *data), void *data,
				bool no_return);
static inline struct cpu_job *cpu_queue_job(struct cpu_thread *cpu,
					    const char *name,
					    void (*func)(void *data),
					    void *data)
{
	return __cpu_queue_job(cpu, name, func, data, false);
}
void cpu_wait_job(struct cpu_job *job, bool free_it);
void cpu_process_local_jobs(void);
struct cpu_job *cpu_queue_job_on_node(uint32_t chip_id,
				       const char *name,
				       void (*func)(void *data), void *data);
#endif /* __CPU_H */
