(:*******************************************************:)
(: Test: K-MinutesFromDurationFunc-6                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Simple test invoking minutes-from-duration() on a negative duration. :)
(:*******************************************************:)
minutes-from-duration(xs:dayTimeDuration("-P3DT8H2M1.03S")) eq -2