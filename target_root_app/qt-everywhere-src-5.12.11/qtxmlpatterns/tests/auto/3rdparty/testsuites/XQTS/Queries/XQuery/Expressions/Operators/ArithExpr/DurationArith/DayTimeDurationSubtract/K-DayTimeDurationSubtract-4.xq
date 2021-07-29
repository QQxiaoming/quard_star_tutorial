(:*******************************************************:)
(: Test: K-DayTimeDurationSubtract-4                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The substraction operator is not available between xs:dayTimeDuration and xs:yearMonthDuration. :)
(:*******************************************************:)
xs:dayTimeDuration("P3D") -
						       xs:yearMonthDuration("P3Y3M")