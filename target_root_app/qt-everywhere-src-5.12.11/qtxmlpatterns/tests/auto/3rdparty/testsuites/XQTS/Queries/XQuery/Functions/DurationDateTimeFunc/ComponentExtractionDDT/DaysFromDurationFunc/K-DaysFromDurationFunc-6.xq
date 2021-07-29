(:*******************************************************:)
(: Test: K-DaysFromDurationFunc-6                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Simple test invoking days-from-duration() on a negative duration. :)
(:*******************************************************:)
days-from-duration(xs:dayTimeDuration("-P45678DT8H2M1.03S"))
			eq -45678