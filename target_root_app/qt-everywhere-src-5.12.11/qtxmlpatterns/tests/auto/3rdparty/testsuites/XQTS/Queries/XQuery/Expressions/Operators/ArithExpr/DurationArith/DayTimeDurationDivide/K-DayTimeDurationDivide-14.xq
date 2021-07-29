(:*******************************************************:)
(: Test: K-DayTimeDurationDivide-14                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The division operator is not available between xs:yearMonthDuration and xs:duration. :)
(:*******************************************************:)
xs:yearMonthDuration("P3Y3M") div
						       xs:duration("P3D")