(:*******************************************************:)
(: Test: K-YearMonthDurationSubtract-6                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The addition operator is not available between xs:duration and xs:yearMonthDuration. :)
(:*******************************************************:)
xs:duration("P3D") +
						       xs:yearMonthDuration("P3Y3M")