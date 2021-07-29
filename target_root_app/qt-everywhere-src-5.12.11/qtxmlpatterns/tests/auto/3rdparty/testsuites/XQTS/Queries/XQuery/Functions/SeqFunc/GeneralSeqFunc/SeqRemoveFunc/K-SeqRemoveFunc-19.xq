(:*******************************************************:)
(: Test: K-SeqRemoveFunc-19                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: An expression involving the eq operator that trigger certain optimization paths in some implementations. :)
(:*******************************************************:)
count(remove(current-time(), 1)) eq 0