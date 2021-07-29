(:*******************************************************:)
(: Test: K-DayTimeDurationDivide-13                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The division operator is not available between xs:duration and xs:yearMonthDuration. :)
(:*******************************************************:)
xs:duration("P3D") div
						       xs:yearMonthDuration("P3Y3M")