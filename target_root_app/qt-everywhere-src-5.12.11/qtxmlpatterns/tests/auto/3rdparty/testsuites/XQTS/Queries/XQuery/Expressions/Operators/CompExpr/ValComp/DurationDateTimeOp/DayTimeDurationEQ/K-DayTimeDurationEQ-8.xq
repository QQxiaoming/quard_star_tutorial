(:*******************************************************:)
(: Test: K-DayTimeDurationEQ-8                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test eq operator for xs:dayTimeDuration values with large milli second component. :)
(:*******************************************************:)
xs:dayTimeDuration("P6DT16H34M59.613999S") eq
		   xs:dayTimeDuration("P6DT16H34M59.613999S")