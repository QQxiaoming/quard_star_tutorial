(:*******************************************************:)
(: Test: K-NumericGT-22                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: ge combined with count().                    :)
(:*******************************************************:)
count((1, 2, 3, timezone-from-time(current-time()), 4)) ge 1