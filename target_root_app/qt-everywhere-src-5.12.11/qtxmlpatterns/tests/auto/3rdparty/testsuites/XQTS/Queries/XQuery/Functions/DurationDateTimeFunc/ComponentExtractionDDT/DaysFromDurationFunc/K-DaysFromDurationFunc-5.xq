(:*******************************************************:)
(: Test: K-DaysFromDurationFunc-5                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `days-from-duration(xs:dayTimeDuration("P45678DT8H2M1.03S")) eq 45678`. :)
(:*******************************************************:)
days-from-duration(xs:dayTimeDuration("P45678DT8H2M1.03S"))
			eq 45678