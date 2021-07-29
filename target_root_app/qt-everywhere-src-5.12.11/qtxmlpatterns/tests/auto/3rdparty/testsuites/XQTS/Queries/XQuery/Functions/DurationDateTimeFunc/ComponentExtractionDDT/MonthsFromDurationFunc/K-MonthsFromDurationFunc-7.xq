(:*******************************************************:)
(: Test: K-MonthsFromDurationFunc-7                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Simple test invoking months-from-duration() on an negative xs:duration. :)
(:*******************************************************:)
months-from-duration(xs:duration("-P3Y4M4DT1H23M2.34S")) eq -4