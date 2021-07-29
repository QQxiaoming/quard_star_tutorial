(:*******************************************************:)
(: Test: K-AdjTimeToTimezoneFunc-10                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Example from F&O.                            :)
(:*******************************************************:)

			adjust-time-to-timezone(xs:time("10:00:00"),
						xs:dayTimeDuration("-PT5H0M"))
			eq xs:time("10:00:00-05:00")
		