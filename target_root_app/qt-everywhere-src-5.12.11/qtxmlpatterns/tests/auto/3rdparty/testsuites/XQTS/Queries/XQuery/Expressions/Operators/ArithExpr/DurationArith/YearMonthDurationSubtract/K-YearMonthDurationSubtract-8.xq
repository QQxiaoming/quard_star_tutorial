(:*******************************************************:)
(: Test: K-YearMonthDurationSubtract-8                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The addition operator is not available between xs:dayTimeDuration and xs:duration. :)
(:*******************************************************:)
xs:dayTimeDuration("P3D") +
						       xs:duration("P3Y3M")