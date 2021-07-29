(:*******************************************************:)
(: Test: K-SeqUnorderedFunc-5                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `count(unordered((1, 2, current-time()))) eq 3`. :)
(:*******************************************************:)
count(unordered((1, 2, current-time()))) eq 3