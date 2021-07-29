(:*******************************************************:)
(: Test: K-DateTimeEQ-27                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The operator 'eq' is not available between xs:dateTime and xs:date . :)
(:*******************************************************:)
xs:time("22:12:04") eq
		xs:dateTime("1999-12-04T12:12:23")