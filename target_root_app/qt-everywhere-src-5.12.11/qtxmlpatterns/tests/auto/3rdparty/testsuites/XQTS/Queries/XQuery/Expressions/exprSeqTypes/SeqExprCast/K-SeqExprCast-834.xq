(:*******************************************************:)
(: Test: K-SeqExprCast-834                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:dateTime to xs:boolean isn't allowed. :)
(:*******************************************************:)
xs:dateTime("2002-10-10T12:00:00-05:00") cast as xs:boolean