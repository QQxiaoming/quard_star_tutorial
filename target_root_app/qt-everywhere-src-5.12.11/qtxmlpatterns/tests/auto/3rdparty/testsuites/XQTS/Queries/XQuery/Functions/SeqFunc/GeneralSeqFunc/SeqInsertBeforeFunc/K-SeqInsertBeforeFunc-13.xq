(:*******************************************************:)
(: Test: K-SeqInsertBeforeFunc-13                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `count(insert-before((1, 2, 3, 4), 1, ())) eq 4`. :)
(:*******************************************************:)
count(insert-before((1, 2, 3, 4), 1, ())) eq 4