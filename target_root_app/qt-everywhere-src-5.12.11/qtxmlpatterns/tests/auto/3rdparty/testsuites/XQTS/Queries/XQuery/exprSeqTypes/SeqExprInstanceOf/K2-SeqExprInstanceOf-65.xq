(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-65                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that xs:long is not a child of positiveInteger. :)
(:*******************************************************:)
xs:long("1") instance of xs:positiveInteger