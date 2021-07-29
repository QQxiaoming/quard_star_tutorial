(:*******************************************************:)
(: Test: K-SeqRemoveFunc-9                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `count(remove(("one", 2, 3), 1)) eq 2`. :)
(:*******************************************************:)
count(remove(("one", 2, 3), 1)) eq 2