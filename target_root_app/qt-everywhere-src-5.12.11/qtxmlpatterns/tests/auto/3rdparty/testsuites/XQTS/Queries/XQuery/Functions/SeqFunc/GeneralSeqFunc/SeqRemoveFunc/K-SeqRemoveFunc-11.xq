(:*******************************************************:)
(: Test: K-SeqRemoveFunc-11                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `count(remove((1, 2, "three"), 3)) eq 2`. :)
(:*******************************************************:)
count(remove((1, 2, "three"), 3)) eq 2