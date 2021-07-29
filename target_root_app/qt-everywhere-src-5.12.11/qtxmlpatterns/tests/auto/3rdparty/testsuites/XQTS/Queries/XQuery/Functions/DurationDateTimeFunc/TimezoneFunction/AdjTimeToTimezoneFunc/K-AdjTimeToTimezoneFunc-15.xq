(:*******************************************************:)
(: Test: K-AdjTimeToTimezoneFunc-15                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Example from F&O.                            :)
(:*******************************************************:)

			adjust-time-to-timezone(xs:time("10:00:00-07:00"),
						())
			eq xs:time("10:00:00")
		