(:*******************************************************:)
(: Test: K-SeqDistinctValuesFunc-15                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: fn:distinct-values() applied on an argument of cardinality exactly-one. :)
(:*******************************************************:)
count(distinct-values(current-time())) eq 1