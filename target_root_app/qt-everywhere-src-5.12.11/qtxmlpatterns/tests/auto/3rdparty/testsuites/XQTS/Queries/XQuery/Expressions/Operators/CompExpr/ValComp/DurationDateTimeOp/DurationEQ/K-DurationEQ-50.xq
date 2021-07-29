(:*******************************************************:)
(: Test: K-DurationEQ-50                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The 'gt' operator is not available between xs:yearMonthDuration and xs:duration. :)
(:*******************************************************:)
xs:yearMonthDuration("P1999Y10M") gt
			   xs:duration("P3DT08H34M12.143S")