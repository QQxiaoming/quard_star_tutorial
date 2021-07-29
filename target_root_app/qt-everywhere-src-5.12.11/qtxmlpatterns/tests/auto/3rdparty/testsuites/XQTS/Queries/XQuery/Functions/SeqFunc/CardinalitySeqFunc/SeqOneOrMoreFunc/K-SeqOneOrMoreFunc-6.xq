(:*******************************************************:)
(: Test: K-SeqOneOrMoreFunc-6                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `count(one-or-more( (1, 2, 3, "four") )) eq 4`. :)
(:*******************************************************:)
count(one-or-more( (1, 2, 3, "four") )) eq 4