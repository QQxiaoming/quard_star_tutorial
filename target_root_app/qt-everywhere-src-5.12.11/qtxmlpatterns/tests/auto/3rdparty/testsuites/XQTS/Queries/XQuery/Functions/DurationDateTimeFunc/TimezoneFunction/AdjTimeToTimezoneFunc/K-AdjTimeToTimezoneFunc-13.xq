(:*******************************************************:)
(: Test: K-AdjTimeToTimezoneFunc-13                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Example from F&O.                            :)
(:*******************************************************:)

			adjust-time-to-timezone(xs:time("10:00:00-07:00"),
						xs:dayTimeDuration("-PT10H"))
			eq xs:time("07:00:00-10:00")
		