(:*******************************************************:)
(: Test: K-SeqIndexOfFunc-24                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `count(index-of((1, 2, 3, 2, 1), 3)) eq 1`. :)
(:*******************************************************:)
count(index-of((1, 2, 3, 2, 1), 3)) eq 1