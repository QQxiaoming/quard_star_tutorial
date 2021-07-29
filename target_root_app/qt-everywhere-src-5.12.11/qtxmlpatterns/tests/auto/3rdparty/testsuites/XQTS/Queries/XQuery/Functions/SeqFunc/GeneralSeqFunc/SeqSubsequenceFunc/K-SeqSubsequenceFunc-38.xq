(:*******************************************************:)
(: Test: K-SeqSubsequenceFunc-38                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `count(subsequence((current-time(), 2 , 3), 3)) eq 1`. :)
(:*******************************************************:)
count(subsequence((current-time(), 2 , 3), 3)) eq 1