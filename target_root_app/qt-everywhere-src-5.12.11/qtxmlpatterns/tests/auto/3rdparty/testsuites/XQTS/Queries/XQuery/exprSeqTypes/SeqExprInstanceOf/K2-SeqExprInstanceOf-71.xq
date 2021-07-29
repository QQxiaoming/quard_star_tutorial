(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-71                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that xs:normalizedString is not a child of xs:token. :)
(:*******************************************************:)
xs:normalizedString("ncname") instance of xs:token