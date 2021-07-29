// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Initiates the request for histograms.
 */
function requestHistograms() {
  let query = '';
  if (document.location.pathname)
    query = document.location.pathname.substring(1);
  cr.sendWithPromise('requestHistograms', query).then(addHistograms);
}

/**
 * Callback from backend with the list of histograms. Builds the UI.
 * @param {!Array<string>} histograms A list of trusted HTML strings
 *     representing histograms.
 */
function addHistograms(histograms) {
  let htmlOutput = '';
  for (let histogram of histograms)
    htmlOutput += histogram;

  // The following HTML tags are coming from
  // |HistogramsMessageHandler::HandleRequestHistograms|.
  const sanitizedHTML = parseHtmlSubset(`<span>${htmlOutput}</span>`, [
                          'PRE', 'H4', 'BR', 'HR'
                        ]).firstChild.innerHTML;
  $('histograms').innerHTML = sanitizedHTML;
}

/**
 * Load the initial list of histograms.
 */
document.addEventListener('DOMContentLoaded', function() {
  $('refresh').onclick = requestHistograms;

  requestHistograms();
});
