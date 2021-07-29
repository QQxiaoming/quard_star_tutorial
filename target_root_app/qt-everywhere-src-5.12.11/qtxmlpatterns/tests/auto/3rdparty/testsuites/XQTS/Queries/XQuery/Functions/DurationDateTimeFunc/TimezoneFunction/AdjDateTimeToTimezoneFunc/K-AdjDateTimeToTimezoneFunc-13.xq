(:*******************************************************:)
(: Test: K-AdjDateTimeToTimezoneFunc-13                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Example from F&O.                            :)
(:*******************************************************:)

			adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T10:00:00-07:00"),
						    xs:dayTimeDuration("-PT10H"))
			eq xs:dateTime("2002-03-07T07:00:00-10:00")
		