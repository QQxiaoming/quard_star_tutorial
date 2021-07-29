(:*******************************************************:)
(: Test: K-HoursFromDurationFunc-7                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Simple test invoking days-from-hours() on an negative xs:duration. :)
(:*******************************************************:)
hours-from-duration(xs:duration("-P3Y4M8DT1H23M2.34S")) eq -1