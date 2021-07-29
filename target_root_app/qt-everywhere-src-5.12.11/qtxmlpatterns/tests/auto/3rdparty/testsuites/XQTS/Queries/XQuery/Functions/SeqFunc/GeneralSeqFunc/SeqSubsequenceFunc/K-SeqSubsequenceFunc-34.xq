(:*******************************************************:)
(: Test: K-SeqSubsequenceFunc-34                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: Using subsequence inside a predicate.        :)
(:*******************************************************:)
(1)[deep-equal(1, subsequence((1, 2, current-time()), 1, 1))] eq 1