// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2018 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

 /**
  *  @file    libkshark-collection.c
  *  @brief   Data Collections.
  */

// C
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

// KernelShark
#include "libkshark.h"

/* Quiet warnings over documenting simple structures */
//! @cond Doxygen_Suppress

enum collection_point_type {
	COLLECTION_IGNORE = 0,
	COLLECTION_RESUME,
	COLLECTION_BREAK,
};

#define LAST_BIN	-3

struct entry_list {
	struct entry_list	*next;
	size_t			index;
	uint8_t			type;
};


enum map_flags {
	COLLECTION_BEFORE = -1,
	COLLECTION_INSIDE = 0,
	COLLECTION_AFTER = 1,
};

//! @endcond

/*
 * If the type of the last added entry is COLLECTION_IGNORE, overwrite this
 * entry (ignore the old entry values). Else add a new entry to the list.
 */
static bool collection_add_entry(struct entry_list **list,
				 size_t i, uint8_t type)
{
	struct entry_list *entry = *list;

	if (entry->type != COLLECTION_IGNORE) {
		entry->next = malloc(sizeof(*entry));
		if (!entry->next)
			return false;

		entry = entry->next;
		*list = entry;
	}

	entry->index = i;
	entry->type = type;

	return true;
}

static struct kshark_entry_collection *
kshark_data_collection_alloc(struct kshark_context *kshark_ctx,
			     struct kshark_entry **data,
			     ssize_t first,
			     size_t n_rows,
			     matching_condition_func cond,
			     int val,
			     size_t margin)
{
	struct kshark_entry_collection *col_ptr = NULL;
	struct kshark_entry *last_vis_entry = NULL;
	struct entry_list *col_list, *temp;
	size_t resume_count = 0, break_count = 0;
	size_t i, j, last_added = 0;
	ssize_t end;
	bool good_data = false;

	/* Create the collection. */
	col_ptr = calloc(1, sizeof(*col_ptr));
	if (!col_ptr)
		goto fail;

	end = first + n_rows - margin;
	if (first >= end)
		return col_ptr;

	col_list = malloc(sizeof(*col_list));
	if (!col_list)
		goto fail;

	temp = col_list;

	if (margin != 0) {
		/*
		 * If this collection includes margin data, add a margin data
		 * interval at the very beginning of the data-set.
		 */
		temp->index = first;
		temp->type = COLLECTION_RESUME;
		++resume_count;

		collection_add_entry(&temp, first + margin - 1,
				     COLLECTION_BREAK);
		++break_count;
	} else {
		temp->type = COLLECTION_IGNORE;
	}

	for (i = first + margin; i < end; ++i) {
		if (!cond(kshark_ctx, data[i], val)) {
			/*
			 * The entry is irrelevant for this collection.
			 * Do nothing.
			 */
			continue;
		}

		/* The Matching condition is satisfed. */
		if (!good_data) {
			/*
			 * Resume the collection here. Add some margin data
			 * in front of the data of interest.
			 */
			good_data = true;
			if (last_added == 0 || last_added < i - margin) {
				collection_add_entry(&temp, i - margin,
						 COLLECTION_RESUME);
				++resume_count;
			} else {
				/*
				 * Ignore the last collection Break point.
				 * Continue extending the previous data
				 * interval.
				 */
				temp->type = COLLECTION_IGNORE;
				--break_count;
			}
		} else if (good_data &&
			   data[i]->next &&
			   !cond(kshark_ctx, data[i]->next, val)) {
			/*
			 * Break the collection here. Add some margin data
			 * after the data of interest.
			 */
			good_data = false;
			last_vis_entry = data[i];

			/* Keep adding entries until the "next" record. */
			for (j = i + 1;
			     j != end && last_vis_entry->next != data[j];
			     j++)
				;

			/*
			 * If the number of added entries is smaller than the
			 * number of margin entries requested, keep adding
			 * until you fill the margin.
			 */
			if (i + margin >= j) {
				for (;j < i + margin; ++j) {
					if (cond(kshark_ctx, data[j], val)) {
						/*
						 * Good data has been found.
						 * Continue extending the
						 * previous data interval.
						 */
						good_data = true;
						break;
					}
				}
			}

			last_added = i = j;
			if (!good_data) {
				collection_add_entry(&temp, i, COLLECTION_BREAK);
				++break_count;
			}
		}
	}

	if (good_data) {
		collection_add_entry(&temp, end - 1, COLLECTION_BREAK);
		++break_count;
	}

	if (margin != 0) {
		/*
		 * If this collection includes margin data, add a margin data
		 * interval at the very end of the data-set.
		 */
		collection_add_entry(&temp, first + n_rows - margin,
				 COLLECTION_RESUME);
		++resume_count;

		collection_add_entry(&temp, first + n_rows - 1,
				 COLLECTION_BREAK);
		++break_count;
	}

	/*
	 * If everything is OK, we must have pairs of COLLECTION_RESUME
	 * and COLLECTION_BREAK points.
	 */
	assert(break_count == resume_count);

	col_ptr->next = NULL;

	col_ptr->resume_points = calloc(resume_count,
					sizeof(*col_ptr->resume_points));
	if (!col_ptr->resume_points)
		goto fail;

	col_ptr->break_points = calloc(break_count,
				       sizeof(*col_ptr->break_points));
	if (!col_ptr->break_points) {
		free(col_ptr->resume_points);
		goto fail;
	}

	col_ptr->cond = cond;
	col_ptr->val = val;

	col_ptr->size = resume_count;
	for (i = 0; i < col_ptr->size; ++i) {
		assert(col_list->type == COLLECTION_RESUME);
		col_ptr->resume_points[i] = col_list->index;
		temp = col_list;
		col_list = col_list->next;
		free(temp);

		assert(col_list->type == COLLECTION_BREAK);
		col_ptr->break_points[i] = col_list->index;
		temp = col_list;
		col_list = col_list->next;
		free(temp);
	}

	return col_ptr;

fail:
	fprintf(stderr, "Failed to allocate memory for Data collection.\n");

	free(col_ptr);
	for (i = 0; i < resume_count + break_count; ++i) {
		temp = col_list;
		col_list = col_list->next;
		free(temp);
	}

	return NULL;
}

/*
 * This function provides mapping between the index inside the data-set and
 * the index of the collection interval. Additional output flag is used to
 * resolve the ambiguity of the mapping. If the value of the flag is
 * COLLECTION_INSIDE, the "source_index" is inside the returned interval. If
 * the value of the flag is COLLECTION_BEFORE, the "source_index" is inside
 * the gap before the returned interval. If the value of the flag is
 * COLLECTION_AFTER, the "source_index" is inside the gap after the returned
 * interval.
 */
static ssize_t
map_collection_index_from_source(const struct kshark_entry_collection *col,
				 size_t source_index, int *flag)
{
	size_t l, h, mid;

	if (!col->size)
		return KS_EMPTY_BIN;

	l = 0;
	h = col->size - 1;

	if (source_index < col->resume_points[l]) {
		*flag = COLLECTION_BEFORE;
		return l;
	}

	if (source_index >= col->resume_points[h]) {
		if (source_index < col->break_points[h])
			*flag = COLLECTION_INSIDE;
		else
			*flag = COLLECTION_AFTER;

		return h;
	}

	BSEARCH(h, l, source_index > col->resume_points[mid]);

	if (source_index <= col->break_points[l])
		*flag = COLLECTION_INSIDE;
	else
		*flag = COLLECTION_AFTER;

	return l;
}

static ssize_t
map_collection_request_init(const struct kshark_entry_collection *col,
			    struct kshark_entry_request *req,
			    bool front, size_t *end)
{
	int col_index_flag;
	ssize_t col_index;
	size_t req_end;

	if (req->next || col->size == 0) {
		fprintf(stderr, "Unexpected input in ");
		fprintf(stderr, "map_collection_request_init()\n");
		goto do_nothing;
	}

	req_end = front ? req->first + req->n - 1 :
			  req->first - req->n + 1;

	/*
	 * Find the first Resume Point of the collection which is equal or
	 * greater than the first index of this request.
	 */
	col_index = map_collection_index_from_source(col,
						     req->first,
						     &col_index_flag);

	/*
	 * The value of "col_index" is ambiguous. Use the "col_index_flag" to
	 * deal with all possible cases.
	 */
	if (col_index == KS_EMPTY_BIN) {
		/* Empty collection. */
		goto do_nothing;
	}

	if (col_index_flag == COLLECTION_AFTER) {
		/*
		 * This request starts after the end of interval "col_index".
		 */
		if (front && (col_index == col->size - 1 ||
			      req_end < col->resume_points[col_index + 1])) {
			/*
			 * No overlap between the collection and this front
			 * request. Do nothing.
			 */
			goto do_nothing;
		} else if (!front && req_end > col->break_points[col_index]) {
			/*
			 * No overlap between the collection and this back
			 * request. Do nothing.
			 */
			goto do_nothing;
		}

		/* Remember that the initial request starts in the gap between
		 * the end of "col_index" interval and the beginning of
		 * "col_index + 1" interval. If we process a Front request, we
		 * have to go forwards, so the proper place for starting our
		 * search will be the Resume point of the "col_index + 1"
		 * interval. However, if we process a Back request, we will be
		 * going backwards, so the proper place to start will be the
		 * Break point of "col_index".
		 */
		req->first = front ? col->resume_points[++col_index] :
				     col->break_points[col_index];
	}

	if (col_index_flag == COLLECTION_BEFORE) {
		/*
		 * This request starts before the beginning of interval
		 * "col_index".
		 */
		if (!front && (col_index == 0 ||
			       req_end > col->break_points[col_index - 1])) {
			/*
			 * No overlap between the collection and this back
			 * request. Do nothing.
			 */
			goto do_nothing;
		} else if (front && req_end < col->resume_points[col_index]) {
			/*
			 * No overlap between the collection and this front
			 * request. Do nothing.
			 */
			goto do_nothing;
		}

		/* Remember that the initial request starts in the gap between
		 * the end of "col_index - 1" interval and the beginning of
		 * "col_index" interval. If we process a Front request, we have
		 * to go forwards, so the proper place for starting our search
		 * will be the Resume point of the "col_index" interval.
		 * However, if we process a Back request, we will be going
		 * backwards, so the proper place to start will be the Break
		 * point of "col_index - 1".
		 */
		req->first = front ? col->resume_points[col_index] :
				     col->break_points[--col_index];
	}

	*end = req_end;

	return col_index;

do_nothing:
	*end = KS_EMPTY_BIN;

	return KS_EMPTY_BIN;
}

/*
 * This function uses the intervals of the Data collection to transform the
 * inputted single data request into a list of data requests. The new list of
 * request will ignore the data outside of the intervals of the collection.
 */
static int
map_collection_back_request(const struct kshark_entry_collection *col,
			    struct kshark_entry_request *req)
{
	size_t req_first, req_end;
	ssize_t col_index;
	int req_count;

	col_index = map_collection_request_init(col, req, false, &req_end);
	if (col_index == KS_EMPTY_BIN)
		return 0;

	/*
	 * Now loop over the intervals of the collection going backwards till
	 * the end of the inputted request and create a separate request for
	 * each of those interest.
	 */
	req_count = 1;
	while (col_index >= 0 && req_end <= col->break_points[col_index]) {
		if (req_end >= col->resume_points[col_index]) {
			/*
			 * The last entry of the original request is inside
			 * the "col_index" collection interval. Close the
			 * collection request here and return.
			 */
			req->n = req->first - req_end + 1;
			break;
		}

		/*
		 * The last entry of the original request is outside of the
		 * "col_index" interval. Close the collection request at the
		 * end of this interval and move to the next one. Try to make
		 * another request there.
		 */
		req->n = req->first -
			 col->resume_points[col_index] + 1;

		--col_index;

		if (req_end > col->break_points[col_index]) {
			/*
			 * The last entry of the original request comes before
			 * the end of the next collection interval. Stop here.
			 */
			break;
		}

		if (col_index > 0) {
			/* Make a new request. */
			req_first = col->break_points[col_index];

			req->next =
				kshark_entry_request_alloc(req_first,
							   0,
							   req->cond,
							   req->val,
							   req->vis_only,
							   req->vis_mask);

			if (!req->next)
				goto fail;

			req = req->next;
			++req_count;
		}
	}

	return req_count;

fail:
	fprintf(stderr,
		"Failed to allocate memory for Collection data request.\n");

	return -ENOMEM;
}

/*
 * This function uses the intervals of the Data collection to transform the
 * inputted single data request into a list of data requests. The new list of
 * requests will ignore the data outside of the intervals of the collection.
 */
static int
map_collection_front_request(const struct kshark_entry_collection *col,
			     struct kshark_entry_request *req)
{
	size_t req_first, req_end;
	ssize_t col_index;
	int req_count;

	col_index = map_collection_request_init(col, req, true, &req_end);
	if (col_index == KS_EMPTY_BIN)
		return 0;

	/*
	 * Now loop over the intervals of the collection going forwards till
	 * the end of the inputted request and create a separate request for
	 * each of those interest.
	 */
	req_count = 1;
	while (col_index < col->size &&
	       req_end >= col->resume_points[col_index]) {
		if (req_end <= col->break_points[col_index]) {
			/*
			 * The last entry of the original request is inside
			 * the "col_index" collection interval.
			 * Close the collection request here and return.
			 */
			req->n = req_end - req->first + 1;
			break;
		}

		/*
		 * The last entry of the original request is outside this
		 * collection interval (col_index). Close the collection
		 * request at the end of the interval and move to the next
		 * interval. Try to make another request there.
		 */
		req->n = col->break_points[col_index] -
			 req->first + 1;

		++col_index;

		if (req_end < col->resume_points[col_index]) {
			/*
			 * The last entry of the original request comes before
			 * the beginning of next collection interval.
			 * Stop here.
			 */
			break;
		}

		if (col_index < col->size) {
			/* Make a new request. */
			req_first = col->resume_points[col_index];

			req->next =
				kshark_entry_request_alloc(req_first,
							   0,
							   req->cond,
							   req->val,
							   req->vis_only,
							   req->vis_mask);

			if (!req->next)
				goto fail;

			req = req->next;
			++req_count;
		}
	}

	return req_count;

fail:
	fprintf(stderr,
		"Failed to allocate memory for Collection data request.\n");

	return -ENOMEM;
}

/**
 * @brief Search for an entry satisfying the requirements of a given Data
 *	  request. Start from the position provided by the request and go
 *	  searching in the direction of the increasing timestamps (front).
 *	  The search is performed only inside the intervals, defined by
 *	  the data collection.
 *
 * @param req: Input location for a single Data request. The imputted request
 *	       will be transformed into a list of requests. This new list of
 *	       requests will ignore the data outside of the intervals of the
 *	       collection.
 * @param data: Input location for the trace data.
 * @param col: Input location for the Data collection.
 * @param index: Optional output location for the index of the returned
 *		 entry inside the array.
 *
 * @returns Pointer to the first entry satisfying the matching condition on
 *	    success, or NULL on failure.
 *	    In the special case when some entries, satisfying the Matching
 *	    condition function have been found, but all these entries have
 *	    been discarded because of the visibility criteria (filtered
 *	    entries), the function returns a pointer to a special
 *	    "Dummy entry".
 */
const struct kshark_entry *
kshark_get_collection_entry_front(struct kshark_entry_request *req,
				  struct kshark_entry **data,
				  const struct kshark_entry_collection *col,
				  ssize_t *index)
{
	const struct kshark_entry *entry = NULL;
	int req_count;

	if (index)
		*index = KS_EMPTY_BIN;

	/*
	 * Use the intervals of the Data collection to redefine the data
	 * request in a way which will ignore the data outside of the
	 * intervals of the collection.
	 */
	req_count = map_collection_front_request(col, req);
	if (req_count <= 0)
		return NULL;

	/*
	 * Loop over the list of redefined requests and search until you find
	 * the first matching entry.
	 */
	while (req) {
		entry = kshark_get_entry_front(req, data, index);
		if (entry)
			break;

		req = req->next;
	}

	return entry;
}

/**
 * @brief Search for an entry satisfying the requirements of a given Data
 *	  request. Start from the position provided by the request and go
 *	  searching in the direction of the decreasing timestamps (back).
 *	  The search is performed only inside the intervals, defined by
 *	  the data collection.
 *
 * @param req: Input location for Data request. The imputed request
 *	       will be transformed into a list of requests. This new list of
 *	       requests will ignore the data outside of the intervals of the
 *	       collection.
 * @param data: Input location for the trace data.
 * @param col: Input location for the Data collection.
 * @param index: Optional output location for the index of the returned
 *		 entry inside the array.
 *
 * @returns Pointer to the first entry satisfying the matching condition on
 *	    success, or NULL on failure.
 *	    In the special case when some entries, satisfying the Matching
 *	    condition function have been found, but all these entries have
 *	    been discarded because of the visibility criteria (filtered
 *	    entries), the function returns a pointer to a special
 *	    "Dummy entry".
 */
const struct kshark_entry *
kshark_get_collection_entry_back(struct kshark_entry_request *req,
				 struct kshark_entry **data,
				 const struct kshark_entry_collection *col,
				 ssize_t *index)
{
	const struct kshark_entry *entry = NULL;
	int req_count;

	if (index)
		*index = KS_EMPTY_BIN;

	/*
	 * Use the intervals of the Data collection to redefine the data
	 * request in a way which will ignore the data outside of the
	 * intervals of the collection.
	 */
	req_count = map_collection_back_request(col, req);
	if (req_count <= 0)
		return NULL;

	/*
	 * Loop over the list of redefined requests and search until you find
	 * the first matching entry.
	 */
	while (req) {
		entry = kshark_get_entry_back(req, data, index);
		if (entry)
			break;

		req = req->next;
	}

	return entry;
}

/**
 * @brief Search the list of Data collections and find the collection defined
 *	  with a given Matching condition function and value.
 *
 * @param col: Input location for the Data collection list.
 * @param cond: Matching condition function.
 * @param val: Matching condition value, used by the Matching condition
 *	       function.
 *
 * @returns Pointer to a Data collections on success, or NULL on failure.
 */
struct kshark_entry_collection *
kshark_find_data_collection(struct kshark_entry_collection *col,
			    matching_condition_func cond,
			    int val)
{
	while (col) {
		if (col->cond == cond && col->val == val)
			return col;

		col = col->next;
	}

	return NULL;
}

/**
 * @brief Clear all data intervals of the given Data collection.
 *
 * @param col: Input location for the Data collection.
 */
void kshark_reset_data_collection(struct kshark_entry_collection *col)
{
	free(col->resume_points);
	col->resume_points = NULL;

	free(col->break_points);
	col->break_points = NULL;

	col->size = 0;
}

static void kshark_free_data_collection(struct kshark_entry_collection *col)
{
	free(col->resume_points);
	free(col->break_points);
	free(col);
}

/**
 * @brief Allocate and process data collection, defined with a given Matching
 *	  condition function and value. Add this collection to the list of
 *	  collections used by the session.
 *
 * @param kshark_ctx: Input location for the session context pointer.
 * @param data: Input location for the trace data.
 * @param n_rows: The size of the inputted data.
 * @param cond: Matching condition function for the collection to be
 *	        registered.
 * @param val: Matching condition value of for collection to be registered.
 * @param margin: The size of the additional (margin) data which do not
 *		  satisfy the matching condition, but is added at the
 *		  beginning and at the end of each interval of the collection
 *		  as well as at the beginning and at the end of data-set. If
 *		  "0", no margin data is added.
 *
 * @returns Pointer to the registered Data collections on success, or NULL
 *	    on failure.
 */
struct kshark_entry_collection *
kshark_register_data_collection(struct kshark_context *kshark_ctx,
				struct kshark_entry **data,
				size_t n_rows,
				matching_condition_func cond,
				int val,
				size_t margin)
{
	struct kshark_entry_collection *col;

	col = kshark_add_collection_to_list(kshark_ctx,
					    &kshark_ctx->collections,
					    data, n_rows,
					    cond, val,
					    margin);

	return col;
}

/**
 * @brief Allocate and process data collection, defined with a given Matching
 *	  condition function and value. Add this collection to a given list of
 *	  collections.
 *
 * @param kshark_ctx: Input location for the session context pointer.
 * @param col_list: Input location for the list of collections.
 * @param data: Input location for the trace data.
 * @param n_rows: The size of the inputted data.
 * @param cond: Matching condition function for the collection to be
 *	        registered.
 * @param val: Matching condition value of for collection to be registered.
 * @param margin: The size of the additional (margin) data which do not
 *		  satisfy the matching condition, but is added at the
 *		  beginning and at the end of each interval of the collection
 *		  as well as at the beginning and at the end of data-set. If
 *		  "0", no margin data is added.
 *
 * @returns Pointer to the registered Data collections on success, or NULL
 *	    on failure.
 */
struct kshark_entry_collection *
kshark_add_collection_to_list(struct kshark_context *kshark_ctx,
			      struct kshark_entry_collection **col_list,
			      struct kshark_entry **data,
			      size_t n_rows,
			      matching_condition_func cond,
			      int val,
			      size_t margin)
{
	struct kshark_entry_collection *col;

	col = kshark_data_collection_alloc(kshark_ctx, data,
					   0, n_rows,
					   cond, val,
					   margin);

	if (col) {
		col->next = *col_list;
		*col_list = col;
	}

	return col;
}

/**
 * @brief Search the list of Data collections for a collection defined
 *	  with a given Matching condition function and value. If such a
 *	  collection exists, unregister (remove and free) this collection
 *	  from the list.
 *
 * @param col: Input location for the Data collection list.
 * @param cond: Matching condition function of the collection to be
 *	        unregistered.
 *
 * @param val: Matching condition value of the collection to be unregistered.
 */
void kshark_unregister_data_collection(struct kshark_entry_collection **col,
				       matching_condition_func cond,
				       int val)
{
	struct kshark_entry_collection **last = col;
	struct kshark_entry_collection *list;

	for (list = *col; list; list = list->next) {
		if (list->cond == cond && list->val == val) {
			*last = list->next;
			kshark_free_data_collection(list);
			return;
		}

		last = &list->next;
	}
}

/**
 * @brief Free all Data collections in a given list.
 *
 * @param col: Input location for the Data collection list.
 */
void kshark_free_collection_list(struct kshark_entry_collection *col)
{
	struct kshark_entry_collection *last;

	while (col) {
		last = col;
		col = col->next;
		kshark_free_data_collection(last);
	}
}
