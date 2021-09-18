// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <y.karadz@gmail.com>
 */

 /**
  *  @file    libkshark-model.c
  *  @brief   Visualization model for FTRACE (trace-cmd) data.
  */

// C
#include <stdlib.h>
#include <assert.h>

// KernelShark
#include "libkshark-model.h"

/** The index of the Upper Overflow bin. */
#define UOB(histo) (histo->n_bins)

/** The index of the Lower Overflow bin. */
#define LOB(histo) (histo->n_bins + 1)

/** For all bins. */
# define ALLB(histo) LOB(histo)

/**
 * @brief Initialize the Visualization model.
 *
 * @param histo: Input location for the model descriptor.
 */
void ksmodel_init(struct kshark_trace_histo *histo)
{
	/*
	 * Initialize an empty histo. The histo will have no bins and will
	 * contain no data.
	 */
	memset(histo, 0, sizeof(*histo));
}

/**
 * @brief Clear (reset) the Visualization model.
 *
 * @param histo: Input location for the model descriptor.
 */
void ksmodel_clear(struct kshark_trace_histo *histo)
{
	/* Reset the histo. It will have no bins and will contain no data. */
	free(histo->map);
	free(histo->bin_count);
	ksmodel_init(histo);
}

static void ksmodel_reset_bins(struct kshark_trace_histo *histo,
			       size_t first, size_t last)
{
	/*
	 * Reset the content of the bins.
	 * Be careful here! Resetting the entire array of signed integers with
	 * memset() will work only for values of "0" and "-1". Hence
	 * KS_EMPTY_BIN is expected to be "-1".
	 */
	memset(&histo->map[first], KS_EMPTY_BIN,
	       (last - first + 1) * sizeof(histo->map[0]));

	memset(&histo->bin_count[first], 0,
	       (last - first + 1) * sizeof(histo->bin_count[0]));
}

static bool ksmodel_histo_alloc(struct kshark_trace_histo *histo, size_t n)
{
	free(histo->bin_count);
	free(histo->map);

	/* Create bins. Two overflow bins are added. */
	histo->map = calloc(n + 2, sizeof(*histo->map));
	histo->bin_count = calloc(n + 2, sizeof(*histo->bin_count));

	if (!histo->map || !histo->bin_count) {
		ksmodel_clear(histo);
		fprintf(stderr, "Failed to allocate memory for a histo.\n");
		return false;
	}

	histo->n_bins = n;

	return true;
}

static void ksmodel_set_in_range_bining(struct kshark_trace_histo *histo,
					size_t n, uint64_t min, uint64_t max,
					bool force_in_range)
{
	uint64_t corrected_range, delta_range, range = max - min;
	struct kshark_entry *last;

	/* The size of the bin must be >= 1, hence the range must be >= n. */
	if (n == 0 || range < n) {
		range = n;
		max = min + n;
	}

	/*
	 * If the number of bins changes, allocate memory for the descriptor of
	 * the model.
	 */
	if (n != histo->n_bins) {
		if (!ksmodel_histo_alloc(histo, n)) {
			ksmodel_clear(histo);
			return;
		}
	}

	/* Reset the content of all bins (including overflow bins) to zero. */
	ksmodel_reset_bins(histo, 0, ALLB(histo));

	if (range % n == 0) {
		/*
		 * The range is multiple of the number of bin and needs no
		 * adjustment. This is very unlikely to happen but still ...
		 */
		histo->min = min;
		histo->max = max;
		histo->bin_size = range / n;
	} else {
		/*
		 * The range needs adjustment. The new range will be slightly
		 * bigger, compared to the requested one.
		 */
		histo->bin_size = range / n + 1;
		corrected_range = histo->bin_size * n;
		delta_range = corrected_range - range;
		histo->min = min - delta_range / 2;
		histo->max = histo->min + corrected_range;

		if (!force_in_range)
			return;

		/*
		 * Make sure that the new range doesn't go outside of the time
		 * interval of the dataset.
		 */
		last = histo->data[histo->data_size - 1];
		if (histo->min < histo->data[0]->ts) {
			histo->min = histo->data[0]->ts;
			histo->max = histo->min + corrected_range;
		} else if (histo->max > last->ts) {
			histo->max = last->ts;
			histo->min = histo->max - corrected_range;
		}
	}
}

/**
 * @brief Prepare the bining of the Visualization model.
 *
 * @param histo: Input location for the model descriptor.
 * @param n: Number of bins.
 * @param min: Lower edge of the time-window to be visualized.
 * @param max: Upper edge of the time-window to be visualized.
 */
void ksmodel_set_bining(struct kshark_trace_histo *histo,
			size_t n, uint64_t min, uint64_t max)
{
	ksmodel_set_in_range_bining(histo, n, min, max, false);
}

static size_t ksmodel_set_lower_edge(struct kshark_trace_histo *histo)
{
	/*
	 * Find the index of the first entry inside the range
	 * (timestamp >= min). Note that the value of "min" is considered
	 * inside the range.
	 */
	ssize_t row = kshark_find_entry_by_time(histo->min,
						histo->data,
						0,
						histo->data_size - 1);

	assert(row != BSEARCH_ALL_SMALLER);

	if (row == BSEARCH_ALL_GREATER || row == 0) {
		/* Lower Overflow bin is empty. */
		histo->map[LOB(histo)] = KS_EMPTY_BIN;
		histo->bin_count[LOB(histo)] = 0;
		row = 0;
	} else {
		/*
		 * The first entry inside the range is not the first entry of
		 * the dataset. This means that the Lower Overflow bin contains
		 * data.
		 */

		/* Lower Overflow bin starts at "0". */
		histo->map[LOB(histo)] = 0;

		/*
		 * The number of entries inside the Lower Overflow bin is equal
		 * to the index of the first entry inside the range.
		 */
		histo->bin_count[LOB(histo)] = row;
	}

	/*
	 * Now check if the first entry inside the range falls into the first
	 * bin.
	 */
	if (histo->data[row]->ts < histo->min + histo->bin_size) {
		/*
		 * It is inside the first bin. Set the beginning
		 * of the first bin.
		 */
		histo->map[0] = row;
	} else {
		/* The first bin is empty. */
		histo->map[0] = KS_EMPTY_BIN;
	}

	return row;
}

static size_t ksmodel_set_upper_edge(struct kshark_trace_histo *histo)
{
	/*
	 * Find the index of the first entry outside the range
	 * (timestamp > max). Note that the value of "max" is considered inside
	 * the range. Remember that kshark_find_entry_by_time returns the first
	 * entry which is equal or greater than the reference time.
	 */
	ssize_t row = kshark_find_entry_by_time(histo->max + 1,
						histo->data,
						0,
						histo->data_size - 1);

	assert(row != BSEARCH_ALL_GREATER);

	if (row == BSEARCH_ALL_SMALLER) {
		/* Upper Overflow bin is empty. */
		histo->map[UOB(histo)] = KS_EMPTY_BIN;
		histo->bin_count[UOB(histo)] = 0;
	} else {
		/*
		 * The Upper Overflow bin contains data. Set its beginning and
		 * the number of entries.
		 */
		histo->map[UOB(histo)] = row;
		histo->bin_count[UOB(histo)] = histo->data_size - row;
	}

	return row;
}

static void ksmodel_set_next_bin_edge(struct kshark_trace_histo *histo,
				      size_t bin, size_t last_row)
{
	uint64_t time_min, time_max;
	size_t next_bin = bin + 1;
	ssize_t row;

	/* Calculate the beginning and the end of the next bin. */
	time_min = histo->min + next_bin * histo->bin_size;
	time_max = time_min + histo->bin_size;
	/*
	 * The timestamp of the very last entry of the dataset can be exactly
	 * equal to the value of the upper edge of the range. This is very
	 * likely to happen when we use ksmodel_set_in_range_bining(). In this
	 * case we have to increase the size of the very last bin in order to
	 * make sure that the last entry of the dataset will fall into it.
	 */
	if (next_bin == histo->n_bins - 1)
		++time_max;

	/*
	 * Find the index of the first entry inside
	 * the next bin (timestamp > time_min).
	 */
	row = kshark_find_entry_by_time(time_min, histo->data, last_row,
					histo->data_size - 1);

	if (row < 0 || histo->data[row]->ts >= time_max) {
		/* The bin is empty. */
		histo->map[next_bin] = KS_EMPTY_BIN;
		return;
	}

	/* Set the index of the first entry. */
	histo->map[next_bin] = row;
}

/*
 * Fill in the bin_count array, which maps the number of entries within each
 * bin.
 */
static void ksmodel_set_bin_counts(struct kshark_trace_histo *histo)
{
	int i = 0, prev_not_empty;
	ssize_t count_tmp = 0;

	histo->tot_count = 0;
	memset(&histo->bin_count[0], 0,
	       (histo->n_bins) * sizeof(histo->bin_count[0]));
	/*
	 * Find the first bin which contains data. Start by checking the Lower
	 * Overflow bin.
	 */
	if (histo->map[LOB(histo)] != KS_EMPTY_BIN) {
		prev_not_empty = LOB(histo);
	} else {
		/* Loop till the first non-empty bin. */
		while (histo->map[i] < 0 && i < histo->n_bins) {
			++i;
		}

		prev_not_empty = i++;
	}

	/*
	 * Starting from the first not empty bin, loop over all bins and fill
	 * in the bin_count array to hold the number of entries in each bin.
	 */
	for (; i < histo->n_bins; ++i) {
		if (histo->map[i] != KS_EMPTY_BIN) {
			/*
			 * The current bin is not empty, take its data row and
			 * subtract it from the data row of the previous not
			 * empty bin, which will give us the number of data
			 * rows in the "prev_not_empty" bin.
			 */
			count_tmp = histo->map[i] - histo->map[prev_not_empty];

			/*
			 * We will do a sanity check. The number of data rows
			 * in the previous not empty bin must be greater than
			 * zero.
			 */
			assert(count_tmp > 0);
			histo->bin_count[prev_not_empty] = count_tmp;

			if (prev_not_empty != LOB(histo))
				histo->tot_count += count_tmp;

			prev_not_empty = i;
		}
	}

	/* Check if the Upper Overflow bin contains data. */
	if (histo->map[UOB(histo)] == KS_EMPTY_BIN) {
		/*
		 * The Upper Overflow bin is empty. Use the size of the dataset
		 * to calculate the content of the previouse not empty bin.
		 */
		count_tmp = histo->data_size - histo->map[prev_not_empty];
	} else {
		/*
		 * Use the index of the first entry inside the Upper Overflow
		 * bin to calculate the content of the previouse not empty
		 * bin.
		 */
		count_tmp = histo->map[UOB(histo)] - histo->map[prev_not_empty];
	}

	/*
	 * We will do a sanity check. The number of data rows in the last not
	 * empty bin must be greater than zero.
	 */
	assert(count_tmp >= 0);
	histo->tot_count += histo->bin_count[prev_not_empty] = count_tmp;
}

/**
 * @brief Provide the Visualization model with data. Calculate the current
 *	  state of the model.
 *
 * @param histo: Input location for the model descriptor.
 * @param data: Input location for the trace data.
 * @param n: Number of bins.
 */
void ksmodel_fill(struct kshark_trace_histo *histo,
		  struct kshark_entry **data, size_t n)
{
	size_t last_row = 0;
	int bin;

	histo->data_size = n;
	histo->data = data;

	if (histo->n_bins == 0 ||
	    histo->bin_size == 0 ||
	    histo->data_size == 0) {
		/*
		 * Something is wrong with this histo.
		 * Most likely the binning is not set.
		 */
		ksmodel_clear(histo);
		fprintf(stderr,
			"Unable to fill the model with data.\n");
		fprintf(stderr,
			"Try to set the bining of the model first.\n");

		return;
	}

	/* Set the Lower Overflow bin */
	ksmodel_set_lower_edge(histo);

	/*
	 * Loop over the dataset and set the beginning of all individual bins.
	 */
	for (bin = 0; bin < histo->n_bins; ++bin) {
		ksmodel_set_next_bin_edge(histo, bin, last_row);
		if (histo->map[bin + 1] > 0)
			last_row = histo->map[bin + 1];
	}

	/* Set the Upper Overflow bin. */
	ksmodel_set_upper_edge(histo);

	/* Calculate the number of entries in each bin. */
	ksmodel_set_bin_counts(histo);
}

/**
 * @brief Get the total number of entries in a given bin.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 *
 * @returns The number of entries in this bin.
 */
size_t ksmodel_bin_count(struct kshark_trace_histo *histo, int bin)
{
	if (bin >= 0 && bin < histo->n_bins)
		return histo->bin_count[bin];

	if (bin == UPPER_OVERFLOW_BIN)
		return histo->bin_count[UOB(histo)];

	if (bin == LOWER_OVERFLOW_BIN)
		return histo->bin_count[LOB(histo)];

	return 0;
}

/**
 * @brief Shift the time-window of the model forward. Recalculate the current
 *	  state of the model.
 *
 * @param histo: Input location for the model descriptor.
 * @param n: Number of bins to shift.
 */
void ksmodel_shift_forward(struct kshark_trace_histo *histo, size_t n)
{
	size_t last_row = 0;
	int bin;

	if (!histo->data_size)
		return;

	if (histo->map[UOB(histo)] == KS_EMPTY_BIN) {
		/*
		 * The Upper Overflow bin is empty. This means that we are at
		 * the upper edge of the dataset already. Do nothing in this
		 * case.
		 */
		return;
	}

	histo->min += n * histo->bin_size;
	histo->max += n * histo->bin_size;

	if (n >= histo->n_bins) {
		/*
		 * No overlap between the new and the old ranges. Recalculate
		 * all bins from scratch. First calculate the new range.
		 */
		ksmodel_set_bining(histo, histo->n_bins, histo->min,
							 histo->max);

		ksmodel_fill(histo, histo->data, histo->data_size);
		return;
	}

	/* Set the new Lower Overflow bin. */
	ksmodel_set_lower_edge(histo);

	/*
	 * Copy the the mapping indexes of all overlaping bins starting from
	 * bin "0" of the new histo. Note that the number of overlaping bins
	 * is histo->n_bins - n.
	 * We will do a sanity check. ksmodel_set_lower_edge() sets map[0]
	 * index of the new histo. This index should then be equal to map[n]
	 * index of the old histo.
	 */
	assert (histo->map[0] == histo->map[n]);
	memmove(&histo->map[0], &histo->map[n],
		sizeof(histo->map[0]) * (histo->n_bins - n));

	/*
	 * Calculate only the content of the new (non-overlapping) bins.
	 * Start from the last copied bin and set the edge of each consecutive
	 * bin.
	 */
	bin = histo->n_bins - n - 1;
	for (; bin < histo->n_bins - 1; ++bin) {
		/*
		 * Note that this function will set the bin having index
		 * "bin + 1".
		 */
		ksmodel_set_next_bin_edge(histo, bin, last_row);
		if (histo->map[bin + 1] > 0)
			last_row = histo->map[bin + 1];
	}

	/*
	 * Set the new Upper Overflow bin and calculate the number of entries
	 * in each bin.
	 */
	ksmodel_set_upper_edge(histo);
	ksmodel_set_bin_counts(histo);
}

/**
 * @brief Shift the time-window of the model backward. Recalculate the current
 *	  state of the model.
 *
 * @param histo: Input location for the model descriptor.
 * @param n: Number of bins to shift.
 */
void ksmodel_shift_backward(struct kshark_trace_histo *histo, size_t n)
{
	size_t last_row = 0;
	int bin;

	if (!histo->data_size)
		return;

	if (histo->map[LOB(histo)] == KS_EMPTY_BIN) {
		/*
		 * The Lower Overflow bin is empty. This means that we are at
		 * the Lower edge of the dataset already. Do nothing in this
		 * case.
		 */
		return;
	}

	histo->min -= n * histo->bin_size;
	histo->max -= n * histo->bin_size;

	if (n >= histo->n_bins) {
		/*
		 * No overlap between the new and the old range. Recalculate
		 * all bins from scratch. First calculate the new range.
		 */
		ksmodel_set_bining(histo, histo->n_bins, histo->min,
							 histo->max);

		ksmodel_fill(histo, histo->data, histo->data_size);
		return;
	}

	/*
	 * Copy the mapping indexes of all overlaping bins starting from
	 * bin "0" of the old histo. Note that the number of overlaping bins
	 * is histo->n_bins - n.
	 */
	memmove(&histo->map[n], &histo->map[0],
		sizeof(histo->map[0]) * (histo->n_bins - n));

	/* Set the new Lower Overflow bin. */
	ksmodel_set_lower_edge(histo);

	/* Calculate only the content of the new (non-overlapping) bins. */
	for (bin = 0; bin < n - 1; ++bin) {
		/*
		 * Note that this function will set the bin having index
		 * "bin + 1".
		 */
		ksmodel_set_next_bin_edge(histo, bin, last_row);
		if (histo->map[bin + 1] > 0)
			last_row = histo->map[bin + 1];
	}

	/*
	 * Set the new Upper Overflow bin and calculate the number of entries
	 * in each bin.
	 */
	ksmodel_set_upper_edge(histo);
	ksmodel_set_bin_counts(histo);
}

/**
 * @brief Move the time-window of the model to a given location. Recalculate
 *	  the current state of the model.
 *
 * @param histo: Input location for the model descriptor.
 * @param ts: position in time to be visualized.
 */
void ksmodel_jump_to(struct kshark_trace_histo *histo, uint64_t ts)
{
	uint64_t min, max, range_min;

	if (ts > histo->min && ts < histo->max) {
		/*
		 * The new position is already inside the range.
		 * Do nothing in this case.
		 */
		return;
	}

	/*
	 * Calculate the new range without changing the size and the number
	 * of bins.
	 */
	min = ts - histo->n_bins * histo->bin_size / 2;

	/* Make sure that the range does not go outside of the dataset. */
	if (min < histo->data[0]->ts) {
		min = histo->data[0]->ts;
	} else {
		range_min = histo->data[histo->data_size - 1]->ts -
			    histo->n_bins * histo->bin_size;

		if (min > range_min)
			min = range_min;
	}

	max = min + histo->n_bins * histo->bin_size;

	/* Use the new range to recalculate all bins from scratch. */
	ksmodel_set_bining(histo, histo->n_bins, min, max);
	ksmodel_fill(histo, histo->data, histo->data_size);
}

static void ksmodel_zoom(struct kshark_trace_histo *histo,
			 double r, int mark, bool zoom_in)
{
	size_t range, min, max, delta_min;
	double delta_tot;

	if (!histo->data_size)
		return;

	/*
	 * If the marker is not set, assume that the focal point of the zoom
	 * is the center of the range.
	 */
	if (mark < 0)
		mark = histo->n_bins / 2;

	range = histo->max - histo->min;

	/*
	 * Avoid overzooming. If needed, adjust the Scale factor to a the value
	 * which provides bin_size >= 5.
	 */
	if (zoom_in && (size_t) (range * (1. - r)) < histo->n_bins * 5)
		r = 1. - (histo->n_bins * 5.) / range;

	/*
	 * Now calculate the new range of the histo. Use the bin of the marker
	 * as a focal point for the zoomout. With this the maker will stay
	 * inside the same bin in the new histo.
	 *
	 * First we set delta_tot to increase by the percentage requested (r).
	 * Then we make delta_min equal to a percentage of delta_tot based on
	 * where the position of the mark is. After this we add / subtract the
	 * original min by the delta_min and subtract / add the max by
	 * delta_tot - delta_min.
	 */
	delta_tot = range * r;

	if (mark == (int)histo->n_bins - 1)
		delta_min = delta_tot;
	else
		delta_min = delta_tot * mark / histo->n_bins;

	min = zoom_in ? histo->min + delta_min :
			histo->min - delta_min;

	max = zoom_in ? histo->max - (size_t) delta_tot + delta_min :
			histo->max + (size_t) delta_tot - delta_min;


	/* Make sure the new range doesn't go outside of the dataset. */
	if (min < histo->data[0]->ts)
		min = histo->data[0]->ts;

	if (max > histo->data[histo->data_size - 1]->ts)
		max = histo->data[histo->data_size - 1]->ts;

	/*
	 * Use the new range to recalculate all bins from scratch. Enforce
	 * "In Range" adjustment of the range of the model, in order to avoid
	 * slowly drifting outside of the data-set in the case when the very
	 * first or the very last entry is used as a focal point.
	 */
	ksmodel_set_in_range_bining(histo, histo->n_bins, min, max, true);
	ksmodel_fill(histo, histo->data, histo->data_size);
}

/**
 * @brief Extend the time-window of the model. Recalculate the current state
 *	  of the model.
 *
 * @param histo: Input location for the model descriptor.
 * @param r: Scale factor of the zoom-out.
 * @param mark: Focus point of the zoom-out.
 */
void ksmodel_zoom_out(struct kshark_trace_histo *histo,
		      double r, int mark)
{
	ksmodel_zoom(histo, r, mark, false);
}

/**
 * @brief Shrink the time-window of the model. Recalculate the current state
 *	  of the model.
 *
 * @param histo: Input location for the model descriptor.
 * @param r: Scale factor of the zoom-in.
 * @param mark: Focus point of the zoom-in.
 */
void ksmodel_zoom_in(struct kshark_trace_histo *histo,
		     double r, int mark)
{
	ksmodel_zoom(histo, r, mark, true);
}

/**
 * @brief Get the index of the first entry in a given bin.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 *
 * @returns Index of the first entry in this bin. If the bin is empty the
 *	    function returns negative error identifier (KS_EMPTY_BIN).
 */
ssize_t ksmodel_first_index_at_bin(struct kshark_trace_histo *histo, int bin)
{
	if (bin >= 0 && bin < (int) histo->n_bins)
		return histo->map[bin];

	if (bin == UPPER_OVERFLOW_BIN)
		return histo->map[UOB(histo)];

	if (bin == LOWER_OVERFLOW_BIN)
		return histo->map[LOB(histo)];

	return KS_EMPTY_BIN;
}

/**
 * @brief Get the index of the last entry in a given bin.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 *
 * @returns Index of the last entry in this bin. If the bin is empty the
 *	    function returns negative error identifier (KS_EMPTY_BIN).
 */
ssize_t ksmodel_last_index_at_bin(struct kshark_trace_histo *histo, int bin)
{
	ssize_t index = ksmodel_first_index_at_bin(histo, bin);
	size_t count = ksmodel_bin_count(histo, bin);

	if (index >= 0 && count)
		index += count - 1;

	return index;
}

static bool ksmodel_is_visible(struct kshark_entry *e)
{
	if ((e->visible & KS_GRAPH_VIEW_FILTER_MASK) &&
	    (e->visible & KS_EVENT_VIEW_FILTER_MASK))
		return true;

	return false;
}

static struct kshark_entry_request *
ksmodel_entry_front_request_alloc(struct kshark_trace_histo *histo,
				  int bin, bool vis_only,
				  matching_condition_func func, int val)
{
	size_t first, n;

	/* Get the number of entries in this bin. */
	n = ksmodel_bin_count(histo, bin);
	if (!n)
		return NULL;

	first = ksmodel_first_index_at_bin(histo, bin);

	return kshark_entry_request_alloc(first, n,
					  func, val,
					  vis_only, KS_GRAPH_VIEW_FILTER_MASK);
}

static struct kshark_entry_request *
ksmodel_entry_back_request_alloc(struct kshark_trace_histo *histo,
				 int bin, bool vis_only,
				 matching_condition_func func, int val)
{
	size_t first, n;

	/* Get the number of entries in this bin. */
	n = ksmodel_bin_count(histo, bin);
	if (!n)
		return NULL;

	first = ksmodel_last_index_at_bin(histo, bin);

	return kshark_entry_request_alloc(first, n,
					  func, val,
					  vis_only, KS_GRAPH_VIEW_FILTER_MASK);
}

/**
 * @brief Get the index of the first entry from a given Cpu in a given bin.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 * @param cpu: Cpu Id.
 *
 * @returns Index of the first entry from a given Cpu in this bin.
 */
ssize_t ksmodel_first_index_at_cpu(struct kshark_trace_histo *histo,
				   int bin, int cpu)
{
	size_t i, n, first, not_found = KS_EMPTY_BIN;

	n = ksmodel_bin_count(histo, bin);
	if (!n)
		return not_found;

	first = ksmodel_first_index_at_bin(histo, bin);

	for (i = first; i < first + n; ++i) {
		if (histo->data[i]->cpu == cpu) {
			if (ksmodel_is_visible(histo->data[i]))
				return i;
			else
				not_found = KS_FILTERED_BIN;
		}
	}

	return not_found;
}

/**
 * @brief Get the index of the first entry from a given Task in a given bin.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 * @param pid: Process Id of a task.
 *
 * @returns Index of the first entry from a given Task in this bin.
 */
ssize_t ksmodel_first_index_at_pid(struct kshark_trace_histo *histo,
				   int bin, int pid)
{
	size_t i, n, first, not_found = KS_EMPTY_BIN;

	n = ksmodel_bin_count(histo, bin);
	if (!n)
		return not_found;

	first = ksmodel_first_index_at_bin(histo, bin);

	for (i = first; i < first + n; ++i) {
		if (histo->data[i]->pid == pid) {
			if (ksmodel_is_visible(histo->data[i]))
				return i;
			else
				not_found = KS_FILTERED_BIN;
		}
	}

	return not_found;
}

/**
 * @brief In a given bin, start from the front end of the bin and go towards
 *	  the back end, searching for an entry satisfying the Matching
 *	  condition defined by a Matching condition function.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 * @param vis_only: If true, a visible entry is requested.
 * @param func: Matching condition function.
 * @param val: Matching condition value, used by the Matching condition
 *	       function.
 * @param col: Optional input location for Data collection.
 * @param index: Optional output location for the index of the requested
 *		 entry inside the array.
 *
 * @returns Pointer ot a kshark_entry, if an entry has been found. Else NULL.
 */
const struct kshark_entry *
ksmodel_get_entry_front(struct kshark_trace_histo *histo,
			int bin, bool vis_only,
			matching_condition_func func, int val,
			struct kshark_entry_collection *col,
			ssize_t *index)
{
	struct kshark_entry_request *req;
	const struct kshark_entry *entry;

	if (index)
		*index = KS_EMPTY_BIN;

	/* Set the position at the beginning of the bin and go forward. */
	req = ksmodel_entry_front_request_alloc(histo, bin, vis_only,
							    func, val);
	if (!req)
		return NULL;

	if (col && col->size)
		entry = kshark_get_collection_entry_front(req, histo->data,
							  col, index);
	else
		entry = kshark_get_entry_front(req, histo->data, index);

	kshark_free_entry_request(req);

	return entry;
}

/**
 * @brief In a given bin, start from the back end of the bin and go towards
 *	  the front end, searching for an entry satisfying the Matching
 *	  condition defined by a Matching condition function.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 * @param vis_only: If true, a visible entry is requested.
 * @param func: Matching condition function.
 * @param val: Matching condition value, used by the Matching condition
 *	       function.
 * @param col: Optional input location for Data collection.
 * @param index: Optional output location for the index of the requested
 *		 entry inside the array.
 *
 * @returns Pointer ot a kshark_entry, if an entry has been found. Else NULL.
 */
const struct kshark_entry *
ksmodel_get_entry_back(struct kshark_trace_histo *histo,
		       int bin, bool vis_only,
		       matching_condition_func func, int val,
		       struct kshark_entry_collection *col,
		       ssize_t *index)
{
	struct kshark_entry_request *req;
	const struct kshark_entry *entry;

	if (index)
		*index = KS_EMPTY_BIN;

	/* Set the position at the end of the bin and go backwards. */
	req = ksmodel_entry_back_request_alloc(histo, bin, vis_only,
							   func, val);
	if (!req)
		return NULL;

	if (col && col->size)
		entry = kshark_get_collection_entry_back(req, histo->data,
							 col, index);
	else
		entry = kshark_get_entry_back(req, histo->data, index);

	kshark_free_entry_request(req);

	return entry;
}

static int ksmodel_get_entry_pid(const struct kshark_entry *entry)
{
	if (!entry) {
		/* No data has been found. */
		return KS_EMPTY_BIN;
	}

	/*
	 * Note that if some data has been found, but this data is
	 * filtered-outa, the Dummy entry is returned. The PID of the Dummy
	 * entry is KS_FILTERED_BIN.
	 */

	return entry->pid;
}

/**
 * @brief In a given bin, start from the front end of the bin and go towards
 *	  the back end, searching for an entry from a given CPU. Return
 *	  the Process Id of the task of the entry found.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 * @param cpu: CPU Id.
 * @param vis_only: If true, a visible entry is requested.
 * @param col: Optional input location for Data collection.
 * @param index: Optional output location for the index of the requested
 *		 entry inside the array.
 *
 * @returns Process Id of the task if an entry has been found. Else a negative
 *	    Identifier (KS_EMPTY_BIN or KS_FILTERED_BIN).
 */
int ksmodel_get_pid_front(struct kshark_trace_histo *histo,
			  int bin, int cpu, bool vis_only,
			  struct kshark_entry_collection *col,
			  ssize_t *index)
{
	const struct kshark_entry *entry;

	if (cpu < 0)
		return KS_EMPTY_BIN;

	entry = ksmodel_get_entry_front(histo, bin, vis_only,
					       kshark_match_cpu, cpu,
					       col, index);

	return ksmodel_get_entry_pid(entry);
}

/**
 * @brief In a given bin, start from the back end of the bin and go towards
 *	  the front end, searching for an entry from a given CPU. Return
 *	  the Process Id of the task of the entry found.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 * @param cpu: CPU Id.
 * @param vis_only: If true, a visible entry is requested.
 * @param col: Optional input location for Data collection.
 * @param index: Optional output location for the index of the requested
 *		 entry inside the array.
 *
 * @returns Process Id of the task if an entry has been found. Else a negative
 *	    Identifier (KS_EMPTY_BIN or KS_FILTERED_BIN).
 */
int ksmodel_get_pid_back(struct kshark_trace_histo *histo,
			 int bin, int cpu, bool vis_only,
			 struct kshark_entry_collection *col,
			 ssize_t *index)
{
	const struct kshark_entry *entry;

	if (cpu < 0)
		return KS_EMPTY_BIN;

	entry = ksmodel_get_entry_back(histo, bin, vis_only,
					      kshark_match_cpu, cpu,
					      col, index);

	return ksmodel_get_entry_pid(entry);
}

static int ksmodel_get_entry_cpu(const struct kshark_entry *entry)
{
	if (!entry) {
		/* No data has been found. */
		return KS_EMPTY_BIN;
	}

	/*
	 * Note that if some data has been found, but this data is
	 * filtered-outa, the Dummy entry is returned. The CPU Id of the Dummy
	 * entry is KS_FILTERED_BIN.
	 */

	return entry->cpu;
}

/**
 * @brief In a given bin, start from the front end of the bin and go towards
 *	  the back end, searching for an entry from a given PID. Return
 *	  the CPU Id of the entry found.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 * @param pid: Process Id.
 * @param vis_only: If true, a visible entry is requested.
 * @param col: Optional input location for Data collection.
 * @param index: Optional output location for the index of the requested
 *		 entry inside the array.
 *
 * @returns The CPU Id of the task if an entry has been found. Else a negative
 *	    Identifier (KS_EMPTY_BIN or KS_FILTERED_BIN).
 */
int ksmodel_get_cpu_front(struct kshark_trace_histo *histo,
			  int bin, int pid, bool vis_only,
			  struct kshark_entry_collection *col,
			  ssize_t *index)
{
	const struct kshark_entry *entry;

	if (pid < 0)
		return KS_EMPTY_BIN;

	entry = ksmodel_get_entry_front(histo, bin, vis_only,
					       kshark_match_pid, pid,
					       col,
					       index);
	return ksmodel_get_entry_cpu(entry);
}

/**
 * @brief In a given bin, start from the back end of the bin and go towards
 *	  the front end, searching for an entry from a given PID. Return
 *	  the CPU Id of the entry found.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 * @param pid: Process Id.
 * @param vis_only: If true, a visible entry is requested.
 * @param col: Optional input location for Data collection.
 * @param index: Optional output location for the index of the requested
 *		 entry inside the array.
 *
 * @returns The CPU Id of the task if an entry has been found. Else a negative
 *	    Identifier (KS_EMPTY_BIN or KS_FILTERED_BIN).
 */
int ksmodel_get_cpu_back(struct kshark_trace_histo *histo,
			 int bin, int pid, bool vis_only,
			 struct kshark_entry_collection *col,
			 ssize_t *index)
{
	const struct kshark_entry *entry;

	if (pid < 0)
		return KS_EMPTY_BIN;

	entry = ksmodel_get_entry_back(histo, bin, vis_only,
					      kshark_match_pid, pid,
					      col,
					      index);

	return ksmodel_get_entry_cpu(entry);
}

/**
 * @brief Check if a visible trace event from a given Cpu exists in this bin.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 * @param cpu: Cpu Id.
 * @param col: Optional input location for Data collection.
 * @param index: Optional output location for the index of the requested
 *		 entry inside the array.
 *
 * @returns True, if a visible entry exists in this bin. Else false.
 */
bool ksmodel_cpu_visible_event_exist(struct kshark_trace_histo *histo,
				     int bin, int cpu,
				     struct kshark_entry_collection *col,
				     ssize_t *index)
{
	struct kshark_entry_request *req;
	const struct kshark_entry *entry;

	if (index)
		*index = KS_EMPTY_BIN;

	/* Set the position at the beginning of the bin and go forward. */
	req = ksmodel_entry_front_request_alloc(histo,
						bin, true,
						kshark_match_cpu, cpu);
	if (!req)
		return false;

	/*
	 * The default visibility mask of the Model Data request is
	 * KS_GRAPH_VIEW_FILTER_MASK. Change the mask to
	 * KS_EVENT_VIEW_FILTER_MASK because we want to find a visible event.
	 */
	req->vis_mask = KS_EVENT_VIEW_FILTER_MASK;

	if (col && col->size)
		entry = kshark_get_collection_entry_front(req, histo->data,
							  col, index);
	else
		entry = kshark_get_entry_front(req, histo->data, index);

	kshark_free_entry_request(req);

	if (!entry || !entry->visible) {
		/* No visible entry has been found. */
		return false;
	}

	return true;
}

/**
 * @brief Check if a visible trace event from a given Task exists in this bin.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 * @param pid: Process Id of the task.
 * @param col: Optional input location for Data collection.
 * @param index: Optional output location for the index of the requested
 *		 entry inside the array.
 *
 * @returns True, if a visible entry exists in this bin. Else false.
 */
bool ksmodel_task_visible_event_exist(struct kshark_trace_histo *histo,
				      int bin, int pid,
				      struct kshark_entry_collection *col,
				      ssize_t *index)
{
	struct kshark_entry_request *req;
	const struct kshark_entry *entry;

	if (index)
		*index = KS_EMPTY_BIN;

	/* Set the position at the beginning of the bin and go forward. */
	req = ksmodel_entry_front_request_alloc(histo,
						bin, true,
						kshark_match_pid, pid);
	if (!req)
		return false;

	/*
	 * The default visibility mask of the Model Data request is
	 * KS_GRAPH_VIEW_FILTER_MASK. Change the mask to
	 * KS_EVENT_VIEW_FILTER_MASK because we want to find a visible event.
	 */
	req->vis_mask = KS_EVENT_VIEW_FILTER_MASK;

	if (col && col->size)
		entry = kshark_get_collection_entry_front(req, histo->data,
							  col, index);
	else
		entry = kshark_get_entry_front(req, histo->data, index);

	kshark_free_entry_request(req);

	if (!entry || !entry->visible) {
		/* No visible entry has been found. */
		return false;
	}

	return true;
}

static bool match_cpu_missed_events(struct kshark_context *kshark_ctx,
				    struct kshark_entry *e, int cpu)
{
	return e->event_id == -EOVERFLOW && e->cpu == cpu;
}

static bool match_pid_missed_events(struct kshark_context *kshark_ctx,
				    struct kshark_entry *e, int pid)
{
	return e->event_id == -EOVERFLOW && e->pid == pid;
}

/**
 * @brief In a given CPU and bin, start from the front end of the bin and go towards
 *	  the back end, searching for a Missed Events entry.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 * @param cpu: CPU Id.
 * @param col: Optional input location for Data collection.
 * @param index: Optional output location for the index of the requested
 *		 entry inside the array.
 *
 * @returns Pointer ot a kshark_entry, if an entry has been found. Else NULL.
 */
const struct kshark_entry *
ksmodel_get_cpu_missed_events(struct kshark_trace_histo *histo,
			      int bin, int cpu,
			      struct kshark_entry_collection *col,
			      ssize_t *index)
{
	return ksmodel_get_entry_front(histo, bin, true,
				       match_cpu_missed_events, cpu,
				       col, index);
}

/**
 * @brief In a given task and bin, start from the front end of the bin and go towards
 *	  the back end, searching for a Missed Events entry.
 *
 * @param histo: Input location for the model descriptor.
 * @param bin: Bin id.
 * @param pid: Process Id of the task.
 * @param col: Optional input location for Data collection.
 * @param index: Optional output location for the index of the requested
 *		 entry inside the array.
 *
 * @returns Pointer ot a kshark_entry, if an entry has been found. Else NULL.
 */
const struct kshark_entry *
ksmodel_get_task_missed_events(struct kshark_trace_histo *histo,
			       int bin, int pid,
			       struct kshark_entry_collection *col,
			       ssize_t *index)
{
	return ksmodel_get_entry_front(histo, bin, true,
				       match_pid_missed_events, pid,
				       col, index);
}
