(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-67                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that xs:ID is not a child of xs:NCName. :)
(:*******************************************************:)
xs:ID("ncname") instance of xs:IDREF