(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-95                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that a cast to xs:unsignedShort is of the right type, when casting from xs:float. :)
(:*******************************************************:)
xs:unsignedShort(xs:float("4")) instance of xs:unsignedShort