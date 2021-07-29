(:*******************************************************:)
(: Test: K-SeqDistinctValuesFunc-13                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `count(distinct-values((1, 2, 2, current-time()))) eq 3`. :)
(:*******************************************************:)
count(distinct-values((1, 2, 2, current-time()))) eq 3