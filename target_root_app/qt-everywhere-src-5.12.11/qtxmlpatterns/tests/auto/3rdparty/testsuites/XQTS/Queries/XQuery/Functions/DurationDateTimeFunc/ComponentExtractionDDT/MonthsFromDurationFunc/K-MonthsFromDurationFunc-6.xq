(:*******************************************************:)
(: Test: K-MonthsFromDurationFunc-6                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Simple test invoking months-from-duration() on a negative duration. :)
(:*******************************************************:)
months-from-duration(xs:yearMonthDuration("-P0003Y2M")) eq -2