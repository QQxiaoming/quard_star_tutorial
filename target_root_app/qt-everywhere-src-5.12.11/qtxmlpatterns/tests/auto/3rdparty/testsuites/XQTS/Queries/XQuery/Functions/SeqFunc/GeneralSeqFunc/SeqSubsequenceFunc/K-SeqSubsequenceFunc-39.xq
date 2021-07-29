(:*******************************************************:)
(: Test: K-SeqSubsequenceFunc-39                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `count(subsequence((current-time(), 2 , 3, 4), 2, 2)) eq 2`. :)
(:*******************************************************:)
count(subsequence((current-time(), 2 , 3,  4), 2, 2)) eq 2