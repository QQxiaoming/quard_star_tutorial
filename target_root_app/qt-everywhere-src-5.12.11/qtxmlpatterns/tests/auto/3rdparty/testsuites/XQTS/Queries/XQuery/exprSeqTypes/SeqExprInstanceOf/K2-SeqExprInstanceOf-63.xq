(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-63                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that xs:long is not a child of nonPositiveInteger. :)
(:*******************************************************:)
xs:long("1") instance of xs:nonPositiveInteger