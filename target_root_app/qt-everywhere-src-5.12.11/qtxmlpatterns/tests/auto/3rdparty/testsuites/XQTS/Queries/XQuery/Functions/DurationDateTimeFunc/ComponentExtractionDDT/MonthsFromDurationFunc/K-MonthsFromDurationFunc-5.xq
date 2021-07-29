(:*******************************************************:)
(: Test: K-MonthsFromDurationFunc-5                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `months-from-duration(xs:yearMonthDuration("P0003Y2M")) eq 2`. :)
(:*******************************************************:)
months-from-duration(xs:yearMonthDuration("P0003Y2M")) eq 2