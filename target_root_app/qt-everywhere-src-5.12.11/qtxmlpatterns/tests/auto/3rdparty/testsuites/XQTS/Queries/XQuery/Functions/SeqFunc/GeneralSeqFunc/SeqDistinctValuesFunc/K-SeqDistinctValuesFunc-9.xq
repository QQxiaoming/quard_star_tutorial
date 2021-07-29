(:*******************************************************:)
(: Test: K-SeqDistinctValuesFunc-9                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `count(distinct-values((1, 2.0, 3, 2))) eq 3`. :)
(:*******************************************************:)
count(distinct-values((1, 2.0, 3, 2))) eq 3