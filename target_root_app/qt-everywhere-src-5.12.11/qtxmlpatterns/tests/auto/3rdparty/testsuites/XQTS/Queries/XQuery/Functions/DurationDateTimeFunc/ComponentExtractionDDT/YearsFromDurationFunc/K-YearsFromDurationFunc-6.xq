(:*******************************************************:)
(: Test: K-YearsFromDurationFunc-6                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Simple test invoking years-from-duration() on a negative duration. :)
(:*******************************************************:)
years-from-duration(xs:yearMonthDuration("-P0003Y2M")) eq -3