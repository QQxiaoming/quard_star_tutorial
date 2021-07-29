(:*******************************************************:)
(: Test: K-SecondsFromDurationFunc-5                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Simple test invoking seconds-from-duration() on a positive duration. :)
(:*******************************************************:)
seconds-from-duration(xs:dayTimeDuration("P3DT8H2M1.03S")) eq 1.03