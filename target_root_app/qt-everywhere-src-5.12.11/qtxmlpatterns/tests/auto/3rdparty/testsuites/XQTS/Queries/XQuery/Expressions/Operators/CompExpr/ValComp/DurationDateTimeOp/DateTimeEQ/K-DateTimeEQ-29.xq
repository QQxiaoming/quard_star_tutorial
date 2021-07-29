(:*******************************************************:)
(: Test: K-DateTimeEQ-29                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The operator 'le' is not available between xs:dateTime and xs:date . :)
(:*******************************************************:)
xs:time("22:12:04") le
				       xs:dateTime("1999-12-04T12:12:23")