(:*******************************************************:)
(: Test: K-SeqInsertBeforeFunc-15                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `count(insert-before((), 30, (1, 2, 3))) eq 3`. :)
(:*******************************************************:)
count(insert-before((), 30, (1, 2, 3))) eq 3