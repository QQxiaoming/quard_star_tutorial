(:*******************************************************:)
(: Test: K-SeqSubsequenceFunc-32                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `count(subsequence(remove(current-time(), 1), 1, 1)) eq 0`. :)
(:*******************************************************:)
count(subsequence(remove(current-time(), 1), 1, 1)) eq 0