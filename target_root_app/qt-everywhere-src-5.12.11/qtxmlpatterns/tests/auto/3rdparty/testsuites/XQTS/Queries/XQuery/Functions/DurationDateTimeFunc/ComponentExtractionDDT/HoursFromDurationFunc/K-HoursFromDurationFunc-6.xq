(:*******************************************************:)
(: Test: K-HoursFromDurationFunc-6                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Simple test invoking hours-from-duration() on a negative duration. :)
(:*******************************************************:)
hours-from-duration(xs:dayTimeDuration("-P3DT8H2M1.03S")) eq -8