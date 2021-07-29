(:*******************************************************:)
(: Test: K-DateTimeEQ-10                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The operator 'ne' is not available between xs:dateTime and xs:date . :)
(:*******************************************************:)
xs:dateTime("1999-12-04T12:12:23") ne
				       xs:date("1999-12-04")