(:*******************************************************:)
(: Test: K-SeqSubsequenceFunc-31                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `count(subsequence((1, 2, 2, current-time()), 2, 2)) eq 2`. :)
(:*******************************************************:)
count(subsequence((1, 2, 2, current-time()), 2, 2)) eq 2