(:*******************************************************:)
(: Test: K-DurationEQ-56                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The 'le' operator is not available between xs:yearMonthDuration and xs:duration(reversed operand order). :)
(:*******************************************************:)
xs:yearMonthDuration("P1999Y10M") le
			   xs:duration("P3DT08H34M12.143S")