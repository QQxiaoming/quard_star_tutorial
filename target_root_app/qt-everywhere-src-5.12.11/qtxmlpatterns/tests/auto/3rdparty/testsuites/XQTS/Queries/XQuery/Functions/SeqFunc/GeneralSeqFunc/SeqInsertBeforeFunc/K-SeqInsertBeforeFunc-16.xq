(:*******************************************************:)
(: Test: K-SeqInsertBeforeFunc-16                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `count(insert-before((error(), 1), 1, (1, "two", 3))) > 1`. :)
(:*******************************************************:)
count(insert-before((error(), 1), 1, (1, "two", 3))) > 1