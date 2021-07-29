(:*******************************************************:)
(: Test: K-YearMonthDurationSubtract-7                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The addition operator is not available between xs:yearMonthDuration and xs:duration. :)
(:*******************************************************:)
xs:yearMonthDuration("P3Y3M") +
						       xs:duration("P3D")