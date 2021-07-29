(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-66                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that xs:token is not a child of xs:NCName. :)
(:*******************************************************:)
xs:token("ncname") instance of xs:NCName